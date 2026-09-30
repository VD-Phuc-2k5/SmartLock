import type { MqttClient } from '../mqtt/MqttClient.js';
import type { OtpRecord } from '../otp/OtpStore.js';
import type { OtpNotifier } from './OtpNotifier.js';

export class MqttNotifier implements OtpNotifier {
    constructor(private readonly mqtt: MqttClient) {}

    async notify(record: OtpRecord): Promise<void> {
        this.mqtt.publish(`smartlock/device/${record.deviceId}/otp`, record.code);
    }
}
