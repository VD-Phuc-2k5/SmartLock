import sys
import time
import threading
from typing import Optional

import serial
import paho.mqtt.client as mqtt
import urllib.request


# ============================================================
# CONFIG
# ============================================================

SERIAL_PORT = "/dev/ttyACM0"
SERIAL_BAUDRATE = 115200

MQTT_HOST = "127.0.0.1"
MQTT_PORT = 1883
MQTT_TOPIC = "smartlock/#"

CAMERA_URL = (
    "http://127.0.0.1:3001"
    "/api/camera/frame"
)

SERIAL_TIMEOUT = 1.0

CAMERA_HEADER = b"@CAM\t"


# ============================================================
# GLOBALS
# ============================================================

serial_port: Optional[serial.Serial] = None

serial_write_lock = threading.Lock()

running = True


# ============================================================
# MQTT
# ============================================================

def on_mqtt_connect(
    client,
    userdata,
    flags,
    reason_code,
    properties=None,
):
    print(
        f"[MQTT] Connected: {reason_code}",
        flush=True,
    )

    client.subscribe(
        MQTT_TOPIC
    )

    print(
        f"[MQTT] Subscribed: {MQTT_TOPIC}",
        flush=True,
    )


def on_mqtt_disconnect(
    client,
    userdata,
    disconnect_flags,
    reason_code,
    properties=None,
):
    print(
        f"[MQTT] Disconnected: {reason_code}",
        flush=True,
    )


def on_mqtt_message(
    client,
    userdata,
    message,
):
    topic = message.topic
    payload = message.payload

    try:
        text = payload.decode(
            "utf-8"
        )
    except UnicodeDecodeError:
        print(
            f"[MQTT] Ignoring binary message: {topic}",
            flush=True,
        )
        return

    send_serial_message(
        topic,
        text,
    )


# ============================================================
# SERIAL WRITE
# ============================================================

def send_serial_message(
    topic: str,
    payload: str,
):
    global serial_port

    if serial_port is None:
        return

    line = (
        "@MSG\t"
        + topic
        + "\t"
        + payload
        + "\n"
    ).encode("utf-8")

    try:
        with serial_write_lock:
            serial_port.write(line)
            serial_port.flush()

    except serial.SerialException as exc:
        print(
            f"[SERIAL] Write error: {exc}",
            flush=True,
        )


# ============================================================
# SERIAL READ HELPERS
# ============================================================

def read_exactly(
    ser: serial.Serial,
    size: int,
) -> Optional[bytes]:
    """
    Read exactly `size` bytes from Serial.

    Returns:
        bytes  -> complete frame
        None   -> serial disconnected/error
    """

    if size <= 0:
        return None

    data = bytearray()

    while len(data) < size and running:
        try:
            chunk = ser.read(
                size - len(data)
            )

        except serial.SerialException as exc:
            print(
                f"[SERIAL] Read error: {exc}",
                flush=True,
            )
            return None

        if not chunk:
            continue

        data.extend(chunk)

    if len(data) != size:
        return None

    return bytes(data)


def read_line(
    ser: serial.Serial,
) -> Optional[bytes]:
    try:
        line = ser.readline()

    except serial.SerialException as exc:
        print(
            f"[SERIAL] Read error: {exc}",
            flush=True,
        )
        return None

    if not line:
        return None

    return line.rstrip(
        b"\r\n"
    )


# ============================================================
# CAMERA
# ============================================================

def upload_camera_frame(
    frame: bytes,
) -> bool:
    if not frame:
        print(
            "[CAMERA] Empty frame",
            flush=True,
        )
        return False

    print(
        f"[CAMERA] Uploading "
        f"{len(frame)} bytes to NestJS...",
        flush=True,
    )

    request = urllib.request.Request(
        CAMERA_URL,
        data=frame,
        method="POST",
        headers={
            "Content-Type": "image/jpeg",
            "Content-Length": str(len(frame)),
        },
    )

    try:
        with urllib.request.urlopen(
            request,
            timeout=10,
        ) as response:

            status = response.status

            body = response.read().decode(
                "utf-8",
                errors="replace",
            )

            print(
                f"[CAMERA] HTTP status: {status}",
                flush=True,
            )

            if body:
                print(
                    f"[CAMERA] Server response: {body}",
                    flush=True,
                )

            if 200 <= status < 300:
                print(
                    "[CAMERA] Upload OK",
                    flush=True,
                )
                return True

            print(
                "[CAMERA] Upload FAILED",
                flush=True,
            )

            return False

    except Exception as exc:
        print(
            f"[CAMERA] HTTP error: {exc}",
            flush=True,
        )
        return False


def handle_camera(
    ser: serial.Serial,
    header: bytes,
):
    """
    Protocol:

        @CAM\\t<size>\\n
        <binary JPEG bytes>
    """

    try:
        size_text = header[
            len(CAMERA_HEADER):
        ].decode(
            "ascii"
        ).strip()

        size = int(
            size_text
        )

    except (
        UnicodeDecodeError,
        ValueError,
    ):
        print(
            f"[CAMERA] Invalid header: "
            f"{header!r}",
            flush=True,
        )
        return

    if size <= 0:
        print(
            f"[CAMERA] Invalid frame size: {size}",
            flush=True,
        )
        return

    # Safety limit.
    # Current OV2640 frame is only a few KB.
    if size > 1024 * 1024:
        print(
            f"[CAMERA] Frame too large: "
            f"{size} bytes",
            flush=True,
        )
        return

    print(
        f"[CAMERA] Receiving {size} bytes...",
        flush=True,
    )

    frame = read_exactly(
        ser,
        size,
    )

    if frame is None:
        print(
            "[CAMERA] Failed to receive complete frame",
            flush=True,
        )
        return

    print(
        f"[CAMERA] Received {len(frame)} bytes",
        flush=True,
    )

    # Basic JPEG validation.
    if not (
        frame.startswith(b"\xff\xd8")
        and frame.endswith(b"\xff\xd9")
    ):
        print(
            "[CAMERA] WARNING: invalid JPEG markers",
            flush=True,
        )

    upload_camera_frame(
        frame
    )


# ============================================================
# SERIAL MESSAGE
# ============================================================

def handle_serial_line(
    ser: serial.Serial,
    line: bytes,
    mqtt_client,
):
    """
    Supported serial protocols:

        @PUB\\t<topic>\\t<payload>
        @CAM\\t<size>
    """

    if line.startswith(
        CAMERA_HEADER
    ):
        handle_camera(
            ser,
            line,
        )
        return

    if line.startswith(
        b"@PUB\t"
    ):
        parts = line.split(
            b"\t",
            2,
        )

        if len(parts) != 3:
            print(
                f"[SERIAL] Invalid @PUB: "
                f"{line!r}",
                flush=True,
            )
            return

        try:
            topic = parts[1].decode(
                "utf-8"
            )

            payload = parts[2].decode(
                "utf-8"
            )

        except UnicodeDecodeError:
            print(
                "[SERIAL] Invalid UTF-8 @PUB",
                flush=True,
            )
            return

        print(
            f"[SERIAL] PUB "
            f"{topic} -> {payload}",
            flush=True,
        )

        result = mqtt_client.publish(
            topic,
            payload,
        )

        if result.rc != mqtt.MQTT_ERR_SUCCESS:
            print(
                f"[MQTT] Publish failed: "
                f"{result.rc}",
                flush=True,
            )

        return

    # Normal ESP32 debug output.
    try:
        text = line.decode(
            "utf-8",
            errors="replace",
        )

        if text:
            print(
                text,
                flush=True,
            )

    except Exception:
        pass


# ============================================================
# SERIAL LOOP
# ============================================================

def serial_loop(
    mqtt_client,
):
    global serial_port
    global running

    print(
        "[SERIAL] Waiting for ESP32...",
        flush=True,
    )

    while running:
        try:
            if serial_port is None:
                serial_port = serial.Serial(
                    port=SERIAL_PORT,
                    baudrate=SERIAL_BAUDRATE,
                    timeout=SERIAL_TIMEOUT,
                    write_timeout=5,
                )

                print(
                    f"[SERIAL] Connected: "
                    f"{SERIAL_PORT} @ "
                    f"{SERIAL_BAUDRATE}",
                    flush=True,
                )

                # Allow ESP32 USB serial to settle.
                time.sleep(1)

            line = read_line(
                serial_port
            )

            if line is None:
                continue

            handle_serial_line(
                serial_port,
                line,
                mqtt_client,
            )

        except serial.SerialException as exc:
            print(
                f"[SERIAL] Disconnected: {exc}",
                flush=True,
            )

            try:
                if serial_port is not None:
                    serial_port.close()
            except Exception:
                pass

            serial_port = None

            if running:
                time.sleep(2)

        except Exception as exc:
            print(
                f"[SERIAL] Error: {exc}",
                flush=True,
            )

            time.sleep(1)


# ============================================================
# MAIN
# ============================================================

def main():
    global running

    print(
        "[GATEWAY] Starting...",
        flush=True,
    )

    # --------------------------------------------------------
    # MQTT
    # --------------------------------------------------------

    mqtt_client = mqtt.Client(
        mqtt.CallbackAPIVersion.VERSION2
    )

    mqtt_client.on_connect = (
        on_mqtt_connect
    )

    mqtt_client.on_disconnect = (
        on_mqtt_disconnect
    )

    mqtt_client.on_message = (
        on_mqtt_message
    )

    print(
        f"[MQTT] Connecting to "
        f"{MQTT_HOST}:{MQTT_PORT}",
        flush=True,
    )

    try:
        mqtt_client.connect(
            MQTT_HOST,
            MQTT_PORT,
            keepalive=60,
        )

    except Exception as exc:
        print(
            f"[MQTT] Connection failed: {exc}",
            flush=True,
        )
        return 1

    mqtt_client.loop_start()

    # --------------------------------------------------------
    # SERIAL
    # --------------------------------------------------------

    try:
        serial_loop(
            mqtt_client
        )

    except KeyboardInterrupt:
        print(
            "\n[GATEWAY] Stopping...",
            flush=True,
        )

    finally:
        running = False

        mqtt_client.loop_stop()

        try:
            mqtt_client.disconnect()
        except Exception:
            pass

        if serial_port is not None:
            try:
                serial_port.close()
            except Exception:
                pass

    print(
        "[GATEWAY] Stopped",
        flush=True,
    )

    return 0


if __name__ == "__main__":
    sys.exit(
        main()
    )