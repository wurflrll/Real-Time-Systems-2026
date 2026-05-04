import asyncio
import websockets
import time
import random
import struct

SERVER = "ws://187.124.174.169:8080/ws"

async def client(client_id):
    async with websockets.connect(SERVER) as ws:

        frames = 10
        start = time.time()
        
        text = "Header end."

        payload = struct.pack("III", 200, 5, 1) + text.encode("utf-8")# + b"\x00"

        await ws.send(payload)

        received = 0
        first_frame_time = None

        while received < frames:
            msg = await ws.recv()
            now = time.time()

            if first_frame_time is None:
                first_frame_time = now

            received += 1

        end = time.time()

        print(
            f"Client {client_id}: "
            f"start_latency={first_frame_time-start:.4f}s "
            f"total_latency={end-start:.4f}s"
        )

async def main(n_clients):
    tasks = [client(i) for i in range(n_clients)]
    await asyncio.gather(*tasks)

asyncio.run(main(2))  # simulate 50 users