import type { OtpGenerator } from './OtpGenerator.js';
import type { OtpRecord, OtpStore } from './OtpStore.js';

export class OtpService {
    constructor(
        private readonly generator: OtpGenerator,
        private readonly store: OtpStore,
        private readonly ttlMs: number,
    ) {}

    issue(deviceId: string, email: string): OtpRecord {
        const record: OtpRecord = {
            deviceId,
            email,
            code: this.generator.generate(),
            expiresAt: Date.now() + this.ttlMs,
        };

        this.store.save(record);
        return record;
    }

    verify(deviceId: string, code: string): boolean {
        const record = this.store.find(deviceId);
        if (!record || record.code !== code) {
            return false;
        }

        this.store.remove(deviceId);
        return true;
    }
}
