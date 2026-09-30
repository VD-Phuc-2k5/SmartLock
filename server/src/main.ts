import http from 'node:http';
import { config } from './config.js';
import { RandomOtpGenerator } from './otp/OtpGenerator.js';
import { InMemoryOtpStore } from './otp/OtpStore.js';
import { OtpService } from './otp/OtpService.js';
import { MqttClient } from './mqtt/MqttClient.js';
import { SmtpMailer } from './mail/SmtpMailer.js';
import { MqttNotifier } from './notify/MqttNotifier.js';
import { EmailNotifier } from './notify/EmailNotifier.js';
import { CompositeNotifier } from './notify/OtpNotifier.js';

const generator = new RandomOtpGenerator(config.otp.length);
const store = new InMemoryOtpStore();
const otpService = new OtpService(generator, store, config.otp.ttlMs);
const mqttClient = new MqttClient(config.mqtt.brokerUrl);

const mailer = new SmtpMailer(config.smtp, config.email.from);
const notifier = new CompositeNotifier([
    new MqttNotifier(mqttClient),
    new EmailNotifier(mailer, config.email.subject),
]);

await mqttClient.connect();

function sendJson(res: http.ServerResponse, status: number, body: unknown): void {
    res.writeHead(status, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify(body));
}

function readBody(req: http.IncomingMessage): Promise<string> {
    return new Promise((resolve, reject) => {
        let data = '';
        req.on('data', (chunk) => (data += chunk));
        req.on('end', () => resolve(data));
        req.on('error', reject);
    });
}

function parseJson(body: string): Record<string, unknown> {
    if (!body) {
        return {};
    }
    return JSON.parse(body);
}

const server = http.createServer(async (req, res) => {
    if (req.method === 'POST' && req.url === '/api/otp') {
        const body = await readBody(req);

        let deviceId: unknown;
        let email: unknown;
        try {
            ({ deviceId, email } = parseJson(body));
        } catch {
            sendJson(res, 400, { error: 'invalid JSON body' });
            return;
        }

        if (typeof deviceId !== 'string' || !deviceId) {
            sendJson(res, 400, { error: 'deviceId is required' });
            return;
        }

        if (typeof email !== 'string' || !email.includes('@')) {
            sendJson(res, 400, { error: 'email is required' });
            return;
        }

        const record = otpService.issue(deviceId, email);
        await notifier.notify(record);
        console.log(`[OTP] device=${deviceId} email=${email} code=${record.code} expiresAt=${new Date(record.expiresAt).toISOString()}`);

        sendJson(res, 200, { deviceId, expiresAt: record.expiresAt });
        return;
    }

    if (req.method === 'POST' && req.url === '/api/otp/verify') {
        const body = await readBody(req);

        let deviceId: unknown;
        let code: unknown;
        try {
            ({ deviceId, code } = parseJson(body));
        } catch {
            sendJson(res, 400, { error: 'invalid JSON body' });
            return;
        }

        if (typeof deviceId !== 'string' || typeof code !== 'string') {
            sendJson(res, 400, { error: 'deviceId and code are required' });
            return;
        }

        const valid = otpService.verify(deviceId, code);
        sendJson(res, 200, { valid });
        return;
    }

    sendJson(res, 404, { error: 'not found' });
});

server.listen(config.http.port, () => {
    console.log(`HTTP server listening on port ${config.http.port}`);
});
