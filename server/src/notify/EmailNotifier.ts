import type { Mailer } from '../mail/IMailer.js';
import type { OtpRecord } from '../otp/OtpStore.js';
import type { OtpNotifier } from './OtpNotifier.js';

export class EmailNotifier implements OtpNotifier {
    constructor(
        private readonly mailer: Mailer,
        private readonly subject: string,
    ) {}

    async notify(record: OtpRecord): Promise<void> {
        const text = `Your Smart Lock OTP is ${record.code}. It expires in 5 minutes.`;
        await this.mailer.send(record.email, this.subject, text);
    }
}
