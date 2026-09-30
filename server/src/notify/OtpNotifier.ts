import type { OtpRecord } from '../otp/OtpStore.js';

export interface OtpNotifier {
    notify(record: OtpRecord): Promise<void>;
}

export class CompositeNotifier implements OtpNotifier {
    constructor(private readonly notifiers: OtpNotifier[]) {}

    async notify(record: OtpRecord): Promise<void> {
        await Promise.all(this.notifiers.map((notifier) => notifier.notify(record)));
    }
}
