export const config = {
    mqtt: {
        brokerUrl: process.env.MQTT_BROKER_URL ?? 'mqtt://localhost:1883',
    },
    http: {
        port: Number(process.env.HTTP_PORT ?? 3000),
    },
    otp: {
        length: 6,
        ttlMs: 5 * 60 * 1000,
    },
};
