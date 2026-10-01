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
import { JsonFileCardStore } from './card/CardStore.js';
import type { Card } from './card/CardStore.js';
import { CardService } from './card/CardService.js';
import { CardEventBus } from './card/CardEventBus.js';
import { Topics, extractDeviceId } from './topics.js';

const generator = new RandomOtpGenerator(config.otp.length);
const store = new InMemoryOtpStore();
const otpService = new OtpService(generator, store, config.otp.ttlMs);
const mqttClient = new MqttClient(config.mqtt.brokerUrl);

const mailer = new SmtpMailer(config.smtp, config.email.from);
const notifier = new CompositeNotifier([
    new MqttNotifier(mqttClient),
    new EmailNotifier(mailer, config.email.subject),
]);

const cardStore = new JsonFileCardStore(config.card.filePath);
const cardService = new CardService(cardStore);
const cardEvents = new CardEventBus();

await mqttClient.connect();

mqttClient.subscribe(Topics.otpRequest);
mqttClient.subscribe(Topics.cardEnroll);
mqttClient.subscribe(Topics.cardVerify);

mqttClient.onMessage((topic, message) => {
    void handleMqttMessage(topic, message);
});

async function handleMqttMessage(topic: string, message: string): Promise<void> {
    try {
        const deviceId = extractDeviceId(topic);
        if (!deviceId) {
            return;
        }

        if (topic.endsWith('/otp/request')) {
            const record = otpService.issue(deviceId, config.email.to);
            await notifier.notify(record);
            console.log(`[OTP] requested by ${deviceId}, sent to ${config.email.to}`);
            return;
        }

        if (topic.endsWith('/card/enroll')) {
            const result = await cardService.enroll(message);
            mqttClient.publish(Topics.cardEnrollResult(deviceId), result);
            console.log(`[CARD] enroll ${deviceId} uid=${message} -> ${result}`);
            if (result === 'ok') {
                cardEvents.publish(await cardService.list());
            }
            return;
        }

        if (topic.endsWith('/card/verify')) {
            const valid = await cardService.verify(message);
            mqttClient.publish(Topics.cardVerifyResult(deviceId), valid ? 'valid' : 'invalid');
            console.log(`[CARD] verify ${deviceId} uid=${message} -> ${valid ? 'valid' : 'invalid'}`);
            return;
        }
    } catch (err) {
        console.error('[MQTT] handler error:', err);
    }
}

const corsHeaders = {
    'Access-Control-Allow-Origin': '*',
    'Access-Control-Allow-Methods': 'GET, POST, PUT, OPTIONS',
    'Access-Control-Allow-Headers': 'Content-Type',
};

function sendJson(res: http.ServerResponse, status: number, body: unknown): void {
    res.writeHead(status, { 'Content-Type': 'application/json', ...corsHeaders });
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

function parseCards(value: unknown): Card[] | null {
    if (!Array.isArray(value)) {
        return null;
    }

    const cards: Card[] = [];
    for (const item of value) {
        if (!item || typeof item !== 'object') {
            return null;
        }
        const { uid, email } = item as { uid?: unknown; email?: unknown };
        if (typeof uid !== 'string' || !uid) {
            return null;
        }
        if (email !== undefined && typeof email !== 'string') {
            return null;
        }
        cards.push({ uid, email: email ?? '' });
    }
    return cards;
}

const server = http.createServer(async (req, res) => {
    if (req.method === 'OPTIONS') {
        res.writeHead(204, corsHeaders);
        res.end();
        return;
    }

    if (req.method === 'GET' && req.url === '/api/cards') {
        const cards = await cardService.list();
        sendJson(res, 200, { cards });
        return;
    }

    if (req.method === 'GET' && req.url === '/api/cards/events') {
        res.writeHead(200, {
            'Content-Type': 'text/event-stream',
            'Cache-Control': 'no-cache',
            'Connection': 'keep-alive',
            ...corsHeaders,
        });
        res.write(': connected\n\n');

        const send = (cards: Card[]) => {
            res.write(`data: ${JSON.stringify({ cards })}\n\n`);
        };

        send(await cardService.list());

        const unsubscribe = cardEvents.subscribe(send);
        req.on('close', () => {
            unsubscribe();
        });
        return;
    }

    if (req.method === 'PUT' && req.url === '/api/cards') {
        const body = await readBody(req);

        let payload: unknown;
        try {
            payload = parseJson(body);
        } catch {
            sendJson(res, 400, { error: 'invalid JSON body' });
            return;
        }

        const cards = parseCards((payload as { cards?: unknown }).cards);
        if (!cards) {
            sendJson(res, 400, { error: 'cards must be an array of { uid, email }' });
            return;
        }

        await cardService.update(cards);
        console.log(`[CARD] updated ${cards.length} cards`);
        cardEvents.publish(await cardService.list());
        sendJson(res, 200, { cards });
        return;
    }

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
