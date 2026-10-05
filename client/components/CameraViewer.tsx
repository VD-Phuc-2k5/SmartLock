'use client';

import { useEffect, useState } from 'react';

const API_URL = process.env.NEXT_PUBLIC_API_URL ?? 'http://localhost:3000';
const REFRESH_MS = 1000;

type Status = 'loading' | 'ready' | 'empty' | 'error';

function Spinner() {
    return (
        <div
            className="h-8 w-8 animate-spin rounded-full border-2 border-white/30 border-t-white"
            role="status"
            aria-label="Đang tải"
        />
    );
}

export default function CameraViewer() {
    const [imageUrl, setImageUrl] = useState<string | null>(null);
    const [status, setStatus] = useState<Status>('loading');
    const [error, setError] = useState<string | null>(null);

    useEffect(() => {
        let cancelled = false;
        let currentUrl: string | null = null;
        let timer: number | undefined;
        const controller = new AbortController();

        async function loadFrame() {
            try {
                const response = await fetch(`${API_URL}/api/camera/frame`, {
                    cache: 'no-store',
                    signal: controller.signal,
                });

                if (cancelled) return;

                if (response.status === 404) {
                    setStatus((prev) => (prev === 'ready' ? prev : 'empty'));
                    return;
                }

                if (!response.ok) {
                    throw new Error(`Camera API: ${response.status}`);
                }

                const blob = await response.blob();
                if (cancelled) return;

                const nextUrl = URL.createObjectURL(blob);
                if (currentUrl) URL.revokeObjectURL(currentUrl);
                currentUrl = nextUrl;

                setImageUrl(nextUrl);
                setStatus('ready');
                setError(null);
            } catch (err) {
                if (cancelled || (err instanceof DOMException && err.name === 'AbortError')) {
                    return;
                }
                setError(err instanceof Error ? err.message : 'Failed to load camera');
                setStatus('error');
            }
        }

        async function loop() {
            await loadFrame();
            if (!cancelled) {
                timer = window.setTimeout(loop, REFRESH_MS);
            }
        }

        void loop();

        return () => {
            cancelled = true;
            controller.abort();
            window.clearTimeout(timer);
            if (currentUrl) URL.revokeObjectURL(currentUrl);
        };
    }, []);

    return (
        <section className="w-full max-w-2xl">
            <div className="mb-4 flex items-center justify-between">
                <h2 className="text-xl font-semibold">Camera</h2>

                {status === 'ready' && (
                    <span className="flex items-center gap-2 text-xs text-green-600">
                        <span className="h-2 w-2 animate-pulse rounded-full bg-green-500" />
                        Live
                    </span>
                )}
            </div>

            <div className="relative aspect-video overflow-hidden rounded-lg border border-black/12 bg-black">
                {imageUrl && (
                    <img
                        src={imageUrl}
                        alt="Smart Lock Camera"
                        className="block h-full w-full object-contain"
                    />
                )}

                {status === 'loading' && (
                    <div className="absolute inset-0 flex flex-col items-center justify-center gap-3 text-sm text-white">
                        <Spinner />
                        <span>Đang kết nối camera...</span>
                    </div>
                )}

                {status === 'empty' && !imageUrl && (
                    <div className="absolute inset-0 flex flex-col items-center justify-center gap-3 text-sm text-white">
                        <Spinner />
                        <span>Đang chờ hình ảnh từ camera...</span>
                    </div>
                )}

                {status === 'error' && !imageUrl && (
                    <div className="absolute inset-0 flex items-center justify-center px-4 text-center text-sm text-red-300">
                        {error}
                    </div>
                )}

                {status === 'error' && imageUrl && (
                    <div className="absolute left-2 top-2 rounded bg-red-600/90 px-2 py-1 text-xs text-white">
                        Mất kết nối, đang thử lại...
                    </div>
                )}
            </div>
        </section>
    );
}