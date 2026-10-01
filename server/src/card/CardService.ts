import type { Card, CardStore } from './CardStore.js';

export type EnrollResult = 'ok' | 'duplicate';

export class CardService {
    constructor(private readonly store: CardStore) {}

    async enroll(uid: string): Promise<EnrollResult> {
        const added = await this.store.add(uid);
        return added ? 'ok' : 'duplicate';
    }

    async verify(uid: string): Promise<boolean> {
        return this.store.contains(uid);
    }

    async find(uid: string): Promise<Card | undefined> {
        return this.store.find(uid);
    }

    async list(): Promise<Card[]> {
        return this.store.list();
    }

    async update(cards: Card[]): Promise<void> {
        await this.store.replaceAll(cards);
    }
}
