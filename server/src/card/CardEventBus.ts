import type { Card } from './CardStore.js';

export type CardListener = (cards: Card[]) => void;

export class CardEventBus {
    private readonly listeners = new Set<CardListener>();

    subscribe(listener: CardListener): () => void {
        this.listeners.add(listener);
        return () => {
            this.listeners.delete(listener);
        };
    }

    publish(cards: Card[]): void {
        for (const listener of this.listeners) {
            listener(cards);
        }
    }
}
