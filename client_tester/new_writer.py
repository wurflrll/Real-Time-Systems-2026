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
            print(line)
            f.flush()

async def client(client_id, queue):
    # Small stagger to preserve connection order
    await asyncio.sleep(client_id * 0.05)  # 50ms apart

    async with websockets.connect(SERVER) as ws:
        frames = 500
        start = time.time()

        payload = struct.pack("III", 5, frames, 1)

        await ws.send(payload)

        print(f"Client {client_id} sent request")

        received = 0
        first_frame_time = None

        while received < frames:
            #print("received: ", received)
            msg = await ws.recv()
            now = time.time()

            # Ignore text/debug messages
            if isinstance(msg, str):
                continue

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

    client_tasks = [
        asyncio.create_task(client(i, queue))
        for i in range(n_clients)
    ]

    await asyncio.gather(*client_tasks)

    await queue.put(None)
    await writer_task

asyncio.run(main(10))