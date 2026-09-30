import mqtt, { MqttClient as MqttClientType } from 'mqtt';

export class MqttClient {
    private client: MqttClientType;

    constructor(private readonly brokerUrl: string) {
        this.client = mqtt.connect(brokerUrl);
    }

    connect(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.client.once('connect', () => {
                console.log('MQTT connected');
                resolve();
            });

            this.client.once('error', reject);
        });
    }

    publish(topic: string, message: string): void {
        this.client.publish(topic, message);
    }

    subscribe(topic: string): void {
        this.client.subscribe(topic);
    }

    onMessage(handler: (topic: string, message: string) => void): void {
        this.client.on('message', (topic, payload) => {
            handler(topic, payload.toString());
        });
    }
}
