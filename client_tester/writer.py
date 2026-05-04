import asyncio
import websockets
import time
import struct

SERVER = "ws://187.124.174.169:8080/ws"

async def writer(queue):
    with open("results.txt", "a") as f:
        while True:
            line = await queue.get()
            if line is None:
                break
            f.write(line + "\n")
            f.flush()

async def client(client_id, queue):
    async with websockets.connect(SERVER) as ws:
        frames = 10
        start = time.time()

        text = "Header end."
        payload = struct.pack("III", 200, 5, 1) + text.encode("utf-8")

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

        result = (
            f"Client {client_id}: "
            f"start_latency={first_frame_time-start:.4f}s "
            f"total_latency={end-start:.4f}s"
        )

        await queue.put(result)

async def main(n_clients):
    queue = asyncio.Queue()

    writer_task = asyncio.create_task(writer(queue))
    client_tasks = [client(i, queue) for i in range(n_clients)]

    await asyncio.gather(*client_tasks)

    await queue.put(None)  # stop signal
    await writer_task

asyncio.run(main(2))