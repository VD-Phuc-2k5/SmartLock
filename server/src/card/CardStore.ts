import { promises as fs } from 'node:fs';
import path from 'node:path';

export interface Card {
    uid: string;
    email: string;
}

export interface CardStore {
    add(uid: string): Promise<boolean>;
    contains(uid: string): Promise<boolean>;
    find(uid: string): Promise<Card | undefined>;
    list(): Promise<Card[]>;
    replaceAll(cards: Card[]): Promise<void>;
}

function normalizeCard(value: unknown): Card {
    if (typeof value === 'string') {
        return { uid: value, email: '' };
    }
    if (value && typeof value === 'object') {
        const card = value as { uid?: unknown; email?: unknown };
        return {
            uid: typeof card.uid === 'string' ? card.uid : '',
            email: typeof card.email === 'string' ? card.email : '',
        };
    }
    return { uid: '', email: '' };
}

export class JsonFileCardStore implements CardStore {
    constructor(private readonly filePath: string) {}

    private async read(): Promise<Card[]> {
        try {
            const data = await fs.readFile(this.filePath, 'utf-8');
            const parsed: unknown = JSON.parse(data);
            return Array.isArray(parsed) ? parsed.map(normalizeCard) : [];
        } catch (err) {
            if ((err as NodeJS.ErrnoException).code === 'ENOENT') {
                return [];
            }
            throw err;
        }
    }

    private async write(cards: Card[]): Promise<void> {
        await fs.mkdir(path.dirname(this.filePath), { recursive: true });
        await fs.writeFile(this.filePath, JSON.stringify(cards, null, 2), 'utf-8');
    }

    async add(uid: string): Promise<boolean> {
        const cards = await this.read();
        if (cards.some((card) => card.uid === uid)) {
            return false;
        }
        cards.push({ uid, email: '' });
        await this.write(cards);
        return true;
    }

    async contains(uid: string): Promise<boolean> {
        const cards = await this.read();
        return cards.some((card) => card.uid === uid);
    }

    async find(uid: string): Promise<Card | undefined> {
        const cards = await this.read();
        return cards.find((card) => card.uid === uid);
    }

    async list(): Promise<Card[]> {
        return this.read();
    }

    async replaceAll(cards: Card[]): Promise<void> {
        await this.write(cards);
    }
}
