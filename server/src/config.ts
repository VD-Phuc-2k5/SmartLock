import 'dotenv/config';

export const config = {
    mqtt: {
        brokerUrl: process.env.MQTT_BROKER_URL ?? 'mqtt://localhost:1883',
    },
    http: {
        port: Number(process.env.HTTP_PORT ?? 3000),
    },
    camera: {
        maxUploadBytes: Number(
            process.env.CAMERA_MAX_UPLOAD_BYTES ??
                1024 * 1024,
        ),
    },
    otp: {
        length: 6,
        ttlMs: 5 * 60 * 1000,
    },
    smtp: {
        host: process.env.SMTP_HOST ?? 'smtp.gmail.com',
        port: Number(process.env.SMTP_PORT ?? 465),
        secure: (process.env.SMTP_SECURE ?? 'true') === 'true',
        user: process.env.SMTP_USER ?? '',
        pass: process.env.SMTP_PASS ?? '',
    },
    email: {
        from: process.env.SMTP_FROM ?? '',
        to: process.env.SMTP_USER ?? '',
        subject: process.env.EMAIL_SUBJECT ?? 'Your Smart Lock OTP',
    },
    card: {
        filePath: process.env.CARD_FILE ?? './data/cards.json',
    },
};
