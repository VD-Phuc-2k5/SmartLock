'use client';

import { useEffect, useRef, useState } from 'react';

const API_URL = (process.env.NEXT_PUBLIC_API_URL ?? 'http://localhost:3000').replace(/\/$/, '');
const WEBRTC_URL = (
    process.env.NEXT_PUBLIC_WEBRTC_URL ?? API_URL.replace(/:\d+$/, ':8081')
).replace(/\/$/, '');

type CameraState = 'IDLE' | 'STREAMING' | 'CAPTURING' | 'STOPPED' | 'ERROR';

export default function CameraViewer() {
    const videoRef = useRef<HTMLVideoElement>(null);
    const [cameraState, setCameraState] = useState<CameraState>('IDLE');
    const [connectionState, setConnectionState] = useState('connecting');

    useEffect(() => {
        let cancelled = false;
        let peer: RTCPeerConnection | null = null;
        let stateTimer: number | undefined;

        const connectWebRtc = async () => {
            try {
                const pc = new RTCPeerConnection({ iceServers: [] });
                peer = pc;
                pc.addTransceiver('video', { direction: 'recvonly' });

                pc.ontrack = (event) => {
                    if (cancelled || !videoRef.current) return;
                    videoRef.current.srcObject = event.streams[0] ?? new MediaStream([event.track]);
                    void videoRef.current.play().catch(() => undefined);
                };

                pc.onconnectionstatechange = () => {
                    if (cancelled) return;
                    setConnectionState(pc.connectionState);
                };

                const offer = await pc.createOffer();
                await pc.setLocalDescription(offer);

                // This implementation uses non-trickle ICE; wait for host candidates.
                if (pc.iceGatheringState !== 'complete') {
                    await new Promise<void>((resolve) => {
                        let settled = false;
                        const finish = () => {
                            if (settled) return;
                            settled = true;
                            window.clearTimeout(timeout);
                            pc.removeEventListener('icegatheringstatechange', onGatheringChange);
                            resolve();
                        };
                        const onGatheringChange = () => {
                            if (pc.iceGatheringState === 'complete') finish();
                        };
                        const timeout = window.setTimeout(finish, 5000);
                        pc.addEventListener('icegatheringstatechange', onGatheringChange);
                    });
                }

                if (cancelled || !pc.localDescription) return;
                const response = await fetch(`${WEBRTC_URL}/offer`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({
                        sdp: pc.localDescription.sdp,
                        type: pc.localDescription.type,
                    }),
                });
                if (!response.ok) throw new Error(`WebRTC signaling failed (${response.status})`);
                const answer = await response.json() as RTCSessionDescriptionInit;
                if (!cancelled) await pc.setRemoteDescription(answer);
            } catch (error) {
                console.error('[Camera] WebRTC connection failed:', error);
                if (!cancelled) setConnectionState('failed');
            }
        };

        const refreshState = async () => {
            try {
                const response = await fetch(`${API_URL}/api/camera/state`, { cache: 'no-store' });
                if (!response.ok) return;
                const result = await response.json() as { state?: CameraState };
                if (!cancelled && result.state) setCameraState(result.state);
            } catch {
                // Keep the last known camera state if the API is temporarily unavailable.
            }
        };

        void connectWebRtc();
        void refreshState();
        stateTimer = window.setInterval(() => void refreshState(), 1000);

        return () => {
            cancelled = true;
            if (stateTimer !== undefined) window.clearInterval(stateTimer);
            if (peer) {
                peer.ontrack = null;
                peer.onconnectionstatechange = null;
                peer.close();
            }
            if (videoRef.current) videoRef.current.srcObject = null;
        };
    }, []);

    const statusLabel: Record<CameraState, string> = {
        IDLE: 'Nhấn B trên keypad để bắt đầu căn chỉnh',
        STREAMING: 'Đang stream — căn chỉnh camera, nhấn C để chụp',
        CAPTURING: 'Đang lưu ảnh cuối cùng...',
        STOPPED: 'Đã chụp và lưu ảnh cuối cùng',
        ERROR: 'Có lỗi khi chụp ảnh',
    };

    const isConnected = connectionState === 'connected';

    return (
        <section className="w-full max-w-2xl">
            <div className="mb-3 flex items-center justify-between gap-3">
                <h2 className="text-xl font-semibold">Camera Smart Lock</h2>
                <span className={`flex items-center gap-2 text-xs ${isConnected ? 'text-green-600' : 'text-amber-500'}`}>
                    <span className={`h-2 w-2 rounded-full ${isConnected ? 'bg-green-500' : 'bg-amber-500'}`} />
                    {isConnected ? 'WebRTC connected' : `WebRTC ${connectionState}`}
                </span>
            </div>

            <div className="relative aspect-video overflow-hidden rounded-lg border border-black/12 bg-black">
                <video
                    ref={videoRef}
                    autoPlay
                    muted
                    playsInline
                    className="block h-full w-full object-contain"
                    aria-label="Smart Lock live camera"
                />
                {cameraState === 'STREAMING' && (
                    <div className="absolute left-2 top-2 flex items-center gap-2 rounded bg-red-600/90 px-2 py-1 text-xs text-white">
                        <span className="h-2 w-2 animate-pulse rounded-full bg-white" /> LIVE
                    </div>
                )}
                {connectionState === 'failed' && (
                    <div className="absolute inset-0 flex items-center justify-center px-4 text-center text-sm text-white">
                        Không kết nối được WebRTC. Hãy kiểm tra dịch vụ camera trên cổng 8081.
                    </div>
                )}
            </div>
            <p className="mt-3 text-sm text-gray-600" aria-live="polite">{statusLabel[cameraState]}</p>
            <p className="mt-1 text-xs text-gray-500">B: bắt đầu stream · C: dừng stream và lưu ảnh đã căn chỉnh.</p>
        </section>
    );
}
