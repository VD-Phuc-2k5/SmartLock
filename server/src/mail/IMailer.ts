export interface Mailer {
    send(to: string, subject: string, text: string): Promise<void>;
}
