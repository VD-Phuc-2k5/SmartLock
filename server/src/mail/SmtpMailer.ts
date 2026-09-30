import nodemailer, { Transporter } from 'nodemailer';
import type { Mailer } from './IMailer.js';

export interface SmtpConfig {
    host: string;
    port: number;
    secure: boolean;
    user: string;
    pass: string;
}

export class SmtpMailer implements Mailer {
    private readonly transporter: Transporter;

    constructor(
        smtp: SmtpConfig,
        private readonly from: string,
    ) {
        this.transporter = nodemailer.createTransport({
            host: smtp.host,
            port: smtp.port,
            secure: smtp.secure,
            auth: {
                user: smtp.user,
                pass: smtp.pass,
            },
        });
    }

    async send(to: string, subject: string, text: string): Promise<void> {
        await this.transporter.sendMail({
            from: this.from,
            to,
            subject,
            text,
        });
    }
}
