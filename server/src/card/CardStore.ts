import { promises as fs } from 'node:fs';
import path from 'node:path';

export interface CardStore {
    add(uid: string): Promise<boolean>;
    contains(uid: string): Promise<boolean>;
    list(): Promise<string[]>;
}

export class JsonFileCardStore implements CardStore {
    constructor(private readonly filePath: string) {}

    private async read(): Promise<string[]> {
        try {
            const data = await fs.readFile(this.filePath, 'utf-8');
            const parsed: unknown = JSON.parse(data);
            return Array.isArray(parsed) ? (parsed as string[]) : [];
        } catch (err) {
            if ((err as NodeJS.ErrnoException).code === 'ENOENT') {
                return [];
            }
            throw err;
        }
    }

    private async write(cards: string[]): Promise<void> {
        await fs.mkdir(path.dirname(this.filePath), { recursive: true });
        await fs.writeFile(this.filePath, JSON.stringify(cards, null, 2), 'utf-8');
    }

    async add(uid: string): Promise<boolean> {
        const cards = await this.read();
        if (cards.includes(uid)) {
            return false;
        }
        cards.push(uid);
        await this.write(cards);
        return true;
    }

    async contains(uid: string): Promise<boolean> {
        const cards = await this.read();
        return cards.includes(uid);
    }

    async list(): Promise<string[]> {
        return this.read();
    }
}
