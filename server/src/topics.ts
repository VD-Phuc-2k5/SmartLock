export const Topics = {
    otpRequest: 'smartlock/device/+/otp/request',
    cardEnroll: 'smartlock/device/+/card/enroll',
    cardVerify: 'smartlock/device/+/card/verify',
    otp: (deviceId: string) => `smartlock/device/${deviceId}/otp`,
    cardEnrollResult: (deviceId: string) => `smartlock/device/${deviceId}/card/enroll/result`,
    cardVerifyResult: (deviceId: string) => `smartlock/device/${deviceId}/card/verify/result`,
};

export function extractDeviceId(topic: string): string | null {
    const parts = topic.split('/');
    return parts.length >= 3 ? parts[2] : null;
}
