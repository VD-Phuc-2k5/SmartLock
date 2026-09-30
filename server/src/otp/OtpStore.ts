export interface OtpRecord {
    deviceId: string;
    email: string;
    code: string;
    expiresAt: number;
}

export interface OtpStore {
    save(record: OtpRecord): void;
    find(deviceId: string): OtpRecord | undefined;
    remove(deviceId: string): void;
    removeExpired(): void;
}

export class InMemoryOtpStore implements OtpStore {
    private readonly records = new Map<string, OtpRecord>();

    save(record: OtpRecord): void {
        this.records.set(record.deviceId, record);
    }

    find(deviceId: string): OtpRecord | undefined {
        const record = this.records.get(deviceId);
        if (!record) {
            return undefined;
        }

        if (record.expiresAt <= Date.now()) {
            this.records.delete(deviceId);
            return undefined;
        }

        return record;
    }

    remove(deviceId: string): void {
        this.records.delete(deviceId);
    }

    removeExpired(): void {
        const now = Date.now();
        for (const [deviceId, record] of this.records) {
            if (record.expiresAt <= now) {
                this.records.delete(deviceId);
            }
        }
    }
}
