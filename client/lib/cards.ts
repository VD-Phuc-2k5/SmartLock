export interface Card {
    uid: string;
    email: string;
}

export interface CardApi {
    list(): Promise<Card[]>;
    save(cards: Card[]): Promise<Card[]>;
    eventsUrl(): string;
}

export class HttpCardApi implements CardApi {
    constructor(private readonly baseUrl: string) {}

    async list(): Promise<Card[]> {
        const res = await fetch(`${this.baseUrl}/api/cards`);
        if (!res.ok) {
            throw new Error(`Failed to fetch cards: ${res.status}`);
        }
        const data = (await res.json()) as { cards: Card[] };
        return data.cards;
    }

    async save(cards: Card[]): Promise<Card[]> {
        const res = await fetch(`${this.baseUrl}/api/cards`, {
            method: 'PUT',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ cards }),
        });
        if (!res.ok) {
            throw new Error(`Failed to save cards: ${res.status}`);
        }
        const data = (await res.json()) as { cards: Card[] };
        return data.cards;
    }

    eventsUrl(): string {
        return `${this.baseUrl}/api/cards/events`;
    }
}

export const cardApi: CardApi = new HttpCardApi(
    process.env.NEXT_PUBLIC_API_URL ?? 'http://localhost:3000',
);
