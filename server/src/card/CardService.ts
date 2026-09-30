import type { CardStore } from './CardStore.js';

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
}
