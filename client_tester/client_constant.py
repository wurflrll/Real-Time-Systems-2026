import asyncio
import websockets
import time
import struct

SERVER = "ws://187.124.174.169:8080/ws"

FRAMES = 2000
STAGGER_SECONDS = 0.05


async def writer(queue):
    with open("results.txt", "a") as f:
        while True:
            line = await queue.get()

            if line is None:
                break

            f.write(line + "\n")
            print(line)
            f.flush()


async def run_single_client(client_id, queue):
    try:
        async with websockets.connect(SERVER) as ws:
            start = time.time()

            payload = struct.pack("III", 5, FRAMES, 1)

            await ws.send(payload)

            print(f"Client {client_id} sent request")

            received = 0
            first_frame_time = None

            while received < FRAMES:
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
                f"start_latency={first_frame_time - start:.4f}s "
                f"total_latency={end - start:.4f}s"
            )

            await queue.put(result)

    except Exception as e:
        await queue.put(f"Client {client_id} ERROR: {e}")


async def client_worker(worker_id, queue, stop_time):
    """
    Continuously creates new connections until stop_time is reached.
    Each finished connection is immediately replaced by another.
    """

    # Initial stagger
    await asyncio.sleep(worker_id * STAGGER_SECONDS)

    connection_count = 0

    while time.time() < stop_time:
        client_id = f"{worker_id}-{connection_count}"

        await run_single_client(client_id, queue)

        connection_count += 1


async def main(n_clients, duration_seconds):
    queue = asyncio.Queue()

    writer_task = asyncio.create_task(writer(queue))

    stop_time = time.time() + duration_seconds

    workers = [
        asyncio.create_task(client_worker(i, queue, stop_time))
        for i in range(n_clients)
    ]

    await asyncio.gather(*workers)

    await queue.put(None)
    await writer_task


if __name__ == "__main__":
    asyncio.run(main(
        n_clients=5,
        duration_seconds=300
    ))