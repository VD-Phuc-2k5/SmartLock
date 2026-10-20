'use client';

import { useEffect, useState } from 'react';

const API_URL = (process.env.NEXT_PUBLIC_API_URL ?? 'http://localhost:3000').replace(/\/$/, '');
const CAMERA_WS_URL = API_URL.replace(/^https:/, 'wss:').replace(/^http:/, 'ws:') + '/api/camera/stream';

type CameraState = 'IDLE' | 'STREAMING' | 'CAPTURING' | 'STOPPED' | 'ERROR';

export default function CameraViewer() {
    const [imageUrl, setImageUrl] = useState<string | null>(null);
    const [cameraState, setCameraState] = useState<CameraState>('IDLE');
    const [connected, setConnected] = useState(false);

    useEffect(() => {
        let cancelled = false;
        let socket: WebSocket | null = null;
        let reconnectTimer: number | undefined;
        let currentUrl: string | null = null;
        let retryDelay = 500;

        const connect = () => {
            if (cancelled) return;
            socket = new WebSocket(CAMERA_WS_URL);
            socket.binaryType = 'blob';

            socket.onopen = () => {
                setConnected(true);
                retryDelay = 500;
            };

            socket.onmessage = (event: MessageEvent) => {
                if (cancelled) return;

                if (typeof event.data === 'string') {
                    try {
                        const message = JSON.parse(event.data) as { type?: string; state?: CameraState };
                        if (message.type === 'state' && message.state) {
                            setCameraState(message.state);
                        }
                    } catch {
                        // Ignore malformed status messages.
                    }
                    return;
                }

                if (!(event.data instanceof Blob)) return;
                const nextUrl = URL.createObjectURL(event.data);
                if (currentUrl) URL.revokeObjectURL(currentUrl);
                currentUrl = nextUrl;
                setImageUrl(nextUrl);
            };

            socket.onerror = () => socket?.close();
            socket.onclose = () => {
                setConnected(false);
                if (cancelled) return;
                reconnectTimer = window.setTimeout(() => {
                    retryDelay = Math.min(retryDelay * 2, 5000);
                    connect();
                }, retryDelay);
            };
        };

        connect();
        return () => {
            cancelled = true;
            window.clearTimeout(reconnectTimer);
            socket?.close();
            if (currentUrl) URL.revokeObjectURL(currentUrl);
        };
    }, []);

    const statusLabel: Record<CameraState, string> = {
        IDLE: 'Nhấn B trên keypad để bắt đầu căn chỉnh',
        STREAMING: 'Đang stream — căn chỉnh camera, nhấn C để chụp',
        CAPTURING: 'Đang lưu ảnh cuối cùng...',
        STOPPED: 'Đã chụp và lưu ảnh cuối cùng',
        ERROR: 'Có lỗi khi chụp ảnh',
    };

    return (
        <section className="w-full max-w-2xl">
            <div className="mb-3 flex items-center justify-between gap-3">
                <h2 className="text-xl font-semibold">Camera Smart Lock</h2>
                <span className={`flex items-center gap-2 text-xs ${connected ? 'text-green-600' : 'text-amber-500'}`}>
                    <span className={`h-2 w-2 rounded-full ${connected ? 'bg-green-500' : 'bg-amber-500'}`} />
                    {connected ? 'WebSocket connected' : 'Connecting...'}
                </span>
            </div>

            <div className="relative aspect-video overflow-hidden rounded-lg border border-black/12 bg-black">
                {imageUrl && (
                    <img src={imageUrl} alt="Smart Lock Camera" className="block h-full w-full object-contain" />
                )}
                {!imageUrl && (
                    <div className="absolute inset-0 flex items-center justify-center px-4 text-center text-sm text-white">
                        Đang chờ hình ảnh camera...
                    </div>
                )}
                {cameraState === 'STREAMING' && (
                    <div className="absolute left-2 top-2 flex items-center gap-2 rounded bg-red-600/90 px-2 py-1 text-xs text-white">
                        <span className="h-2 w-2 animate-pulse rounded-full bg-white" /> LIVE
                    </div>
                )}
            </div>
            <p className="mt-3 text-sm text-gray-600" aria-live="polite">{statusLabel[cameraState]}</p>
            <p className="mt-1 text-xs text-gray-500">B: bắt đầu stream · C: dừng stream và lưu ảnh đã căn chỉnh.</p>
        </section>
    );
}