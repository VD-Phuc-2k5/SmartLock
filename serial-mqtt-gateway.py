import json
import serial
import paho.mqtt.client as mqtt
import threading

SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 115200

MQTT_HOST = "127.0.0.1"
MQTT_PORT = 1883


# =========================
# MQTT
# =========================

mqtt_client = mqtt.Client()


def on_connect(client, userdata, flags, rc):
    print("[MQTT] Connected:", rc)

    client.subscribe("smartlock/#")

def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode("utf-8")

    print(f"[MQTT → SERIAL] {topic}: {payload}")

    message = {
        "type": "message",
        "topic": topic,
        "payload": payload
    }

    send_serial(message)


mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message


# =========================
# SERIAL
# =========================

serial_port = serial.Serial(
    SERIAL_PORT,
    BAUD_RATE,
    timeout=1
)


def send_serial(message):
    data = json.dumps(message) + "\n"

    serial_port.write(
        data.encode("utf-8")
    )


def serial_loop():
    while True:
        line = serial_port.readline()

        if not line:
            continue

        try:
            message = json.loads(
                line.decode("utf-8").strip()
            )

            print("[SERIAL → MQTT]", message)

            if message.get("type") != "publish":
                continue

            topic = message["topic"]
            payload = message.get("payload", "")

            mqtt_client.publish(
                topic,
                payload
            )

        except json.JSONDecodeError:
            print("[SERIAL] Invalid JSON")

        except Exception as e:
            print("[SERIAL] Error:", e)


# =========================
# MAIN
# =========================

print("[GATEWAY] Starting...")

threading.Thread(
    target=serial_loop,
    daemon=True
).start()

mqtt_client.connect(
    MQTT_HOST,
    MQTT_PORT
)

print("[GATEWAY] Running")

mqtt_client.loop_forever()