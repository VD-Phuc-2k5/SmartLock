#!/usr/bin/env python3
"""WebRTC video endpoint for JPEG frames forwarded by the SmartLock API."""

from __future__ import annotations

import asyncio
import io
import os
from typing import Optional

import av
from aiohttp import web
from aiortc import RTCPeerConnection, RTCSessionDescription, VideoStreamTrack

HOST = os.environ.get("WEBRTC_HOST", "0.0.0.0")
PORT = int(os.environ.get("WEBRTC_PORT", "8081"))
FRAME_INTERVAL = 1 / 10

latest_jpeg: Optional[bytes] = None
camera_state = "IDLE"
peer_connections: set[RTCPeerConnection] = set()


def cors_headers() -> dict[str, str]:
    return {
        "Access-Control-Allow-Origin": "*",
        "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
        "Access-Control-Allow-Headers": "Content-Type",
    }


@web.middleware
async def cors_middleware(request: web.Request, handler):
    if request.method == "OPTIONS":
        return web.Response(status=204, headers=cors_headers())
    response = await handler(request)
    response.headers.update(cors_headers())
    return response


class CameraVideoTrack(VideoStreamTrack):
    """Convert the most recent JPEG into a WebRTC-compatible video frame."""

    async def recv(self) -> av.VideoFrame:
        pts, time_base = await self.next_timestamp()
        await asyncio.sleep(FRAME_INTERVAL)

        jpeg = latest_jpeg
        if jpeg:
            try:
                with av.open(io.BytesIO(jpeg), format="mjpeg") as container:
                    frame = next(container.decode(video=0), None)
                if frame is not None:
                    frame.pts = pts
                    frame.time_base = time_base
                    return frame
            except Exception as exc:
                print(f"[WebRTC] JPEG decode failed: {exc}", flush=True)

        # Keep the media track alive while waiting for the first camera frame.
        frame = av.VideoFrame(640, 480, "yuv420p")
        frame.planes[0].update(bytes([16]) * frame.planes[0].buffer_size)
        frame.planes[1].update(bytes([128]) * frame.planes[1].buffer_size)
        frame.planes[2].update(bytes([128]) * frame.planes[2].buffer_size)
        frame.pts = pts
        frame.time_base = time_base
        return frame


async def health(_: web.Request) -> web.Response:
    return web.json_response({"ok": True, "state": camera_state}, headers=cors_headers())


async def receive_frame(request: web.Request) -> web.Response:
    global latest_jpeg
    if request.content_type != "image/jpeg":
        return web.json_response(
            {"error": "Content-Type must be image/jpeg"}, status=415,
            headers=cors_headers(),
        )

    frame = await request.read()
    if not frame or len(frame) > 1024 * 1024:
        return web.json_response(
            {"error": "empty or oversized JPEG frame"}, status=400,
            headers=cors_headers(),
        )
    if not (frame.startswith(b"\xff\xd8") and frame.endswith(b"\xff\xd9")):
        return web.json_response(
            {"error": "invalid JPEG frame"}, status=400,
            headers=cors_headers(),
        )

    latest_jpeg = frame
    return web.json_response({"success": True, "size": len(frame)}, headers=cors_headers())


async def receive_state(request: web.Request) -> web.Response:
    global camera_state
    payload = await request.json()
    state = payload.get("state") if isinstance(payload, dict) else None
    if state not in {"IDLE", "STREAMING", "CAPTURING", "STOPPED", "ERROR"}:
        return web.json_response({"error": "invalid camera state"}, status=400, headers=cors_headers())
    camera_state = state
    return web.json_response({"success": True, "state": camera_state}, headers=cors_headers())


async def offer(request: web.Request) -> web.Response:
    try:
        params = await request.json()
        if not isinstance(params, dict) or not isinstance(params.get("sdp"), str):
            return web.json_response({"error": "offer SDP is required"}, status=400, headers=cors_headers())

        pc = RTCPeerConnection()
        peer_connections.add(pc)

        @pc.on("connectionstatechange")
        async def on_connection_state_change() -> None:
            print(f"[WebRTC] Peer state: {pc.connectionState}", flush=True)
            if pc.connectionState in {"failed", "closed", "disconnected"}:
                await pc.close()
                peer_connections.discard(pc)

        pc.addTrack(CameraVideoTrack())
        await pc.setRemoteDescription(
            RTCSessionDescription(sdp=params["sdp"], type=params.get("type", "offer"))
        )
        answer = await pc.createAnswer()
        await pc.setLocalDescription(answer)

        # Return the SDP after ICE gathering so the browser receives host candidates.
        for _ in range(100):
            if pc.iceGatheringState == "complete":
                break
            await asyncio.sleep(0.05)

        local_description = pc.localDescription
        if local_description is None:
            await pc.close()
            peer_connections.discard(pc)
            return web.json_response({"error": "failed to create WebRTC answer"}, status=500, headers=cors_headers())

        return web.json_response(
            {"sdp": local_description.sdp, "type": local_description.type},
            headers=cors_headers(),
        )
    except Exception as exc:
        print(f"[WebRTC] Offer failed: {exc}", flush=True)
        return web.json_response({"error": "failed to negotiate WebRTC session"}, status=400, headers=cors_headers())


async def cleanup(_: web.Application) -> None:
    await asyncio.gather(*(pc.close() for pc in list(peer_connections)), return_exceptions=True)
    peer_connections.clear()


app = web.Application(middlewares=[cors_middleware], client_max_size=1024 * 1024)
app.router.add_get("/health", health)
app.router.add_post("/frame", receive_frame)
app.router.add_post("/state", receive_state)
app.router.add_post("/offer", offer)
app.on_shutdown.append(cleanup)

if __name__ == "__main__":
    print(f"[WebRTC] Camera media service listening on http://{HOST}:{PORT}", flush=True)
    web.run_app(app, host=HOST, port=PORT)
