import serial
import paho.mqtt.client as mqtt


SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200

MQTT_HOST = "127.0.0.1"
MQTT_PORT = 1883

SERIAL_PUB_PREFIX = "@PUB\t"
SERIAL_MSG_PREFIX = "@MSG\t"


serial_port = serial.Serial(
    SERIAL_PORT,
    BAUD_RATE,
    timeout=0.1,
)


mqtt_client = mqtt.Client(
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2
)


def on_connect(
    client,
    userdata,
    flags,
    reason_code,
    properties,
):
    print(
        f"[MQTT] Connected: {reason_code}"
    )

    client.subscribe(
        "smartlock/#"
    )

    print(
        "[MQTT] Subscribed: smartlock/#"
    )


def on_message(
    client,
    userdata,
    msg,
):
    topic = msg.topic
    payload = msg.payload.decode(
        "utf-8",
        errors="replace",
    )

    print(
        f"[MQTT → SERIAL] "
        f"{topic}: {payload}"
    )

    line = (
        SERIAL_MSG_PREFIX
        + topic
        + "\t"
        + payload
        + "\n"
    )

    serial_port.write(
        line.encode("utf-8")
    )

    serial_port.flush()


def process_serial_line(
    line: str,
):
    if not line.startswith(
        SERIAL_PUB_PREFIX
    ):
        # Ignore ESP32 debug logs.
        return

    content = line[
        len(SERIAL_PUB_PREFIX):
    ]

    separator = content.find("\t")

    if separator < 0:
        print(
            "[SERIAL] Invalid PUB message"
        )
        return

    topic = content[:separator]

    payload = content[
        separator + 1:
    ]

    print(
        f"[SERIAL → MQTT] "
        f"{topic}: {payload}"
    )

    result = mqtt_client.publish(
        topic,
        payload,
    )

    if result.rc == mqtt.MQTT_ERR_SUCCESS:
        print("[MQTT] Publish OK")
    else:
        print(
            f"[MQTT] Publish FAILED: "
            f"{result.rc}"
        )


mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message


print("[GATEWAY] Starting...")

mqtt_client.connect(
    MQTT_HOST,
    MQTT_PORT,
)

print(
    f"[SERIAL] "
    f"{SERIAL_PORT} @ {BAUD_RATE}"
)

print("[GATEWAY] Running")


while True:
    mqtt_client.loop(
        timeout=0.01
    )

    line = serial_port.readline()

    if not line:
        continue

    try:
        decoded = line.decode(
            "utf-8",
            errors="replace",
        ).rstrip("\r\n")

        process_serial_line(decoded)

    except Exception as error:
        print(
            f"[SERIAL] Error: {error}"
        )