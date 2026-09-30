import { randomInt } from 'node:crypto';

export interface OtpGenerator {
    generate(): string;
}

export class RandomOtpGenerator implements OtpGenerator {
    constructor(private readonly length: number = 6) {}

    generate(): string {
        const max = 10 ** this.length;
        return randomInt(0, max).toString().padStart(this.length, '0');
    }
}
