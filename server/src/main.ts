import http from 'node:http';
import { config } from './config.js';
import { RandomOtpGenerator } from './otp/OtpGenerator.js';
import { InMemoryOtpStore } from './otp/OtpStore.js';
import { OtpService } from './otp/OtpService.js';
import { MqttClient } from './mqtt/MqttClient.js';

const generator = new RandomOtpGenerator(config.otp.length);
const store = new InMemoryOtpStore();
const otpService = new OtpService(generator, store, config.otp.ttlMs);
const mqttClient = new MqttClient(config.mqtt.brokerUrl);

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
        try {
            ({ deviceId } = parseJson(body));
        } catch {
            sendJson(res, 400, { error: 'invalid JSON body' });
            return;
        }

        if (typeof deviceId !== 'string' || !deviceId) {
            sendJson(res, 400, { error: 'deviceId is required' });
            return;
        }

        const record = otpService.issue(deviceId);
        mqttClient.publish(`smartlock/device/${deviceId}/otp`, record.code);
        console.log(`[OTP] device=${deviceId} code=${record.code} expiresAt=${new Date(record.expiresAt).toISOString()}`);

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
