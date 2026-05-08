import re
import pandas as pd
import matplotlib.pyplot as plt

# Parse file
pattern = re.compile(
    r"Client (\d+): start_latency=([\d.]+)s total_latency=([\d.]+)s"
)

rows = []

with open("results.txt", "r") as f:
    for line in f:
        match = pattern.search(line)

        if match:
            client_id = int(match.group(1))
            start_latency = float(match.group(2))
            total_latency = float(match.group(3))

            rows.append({
                "client": client_id,
                "start_latency": start_latency,
                "total_latency": total_latency
            })

# Create dataframe
df = pd.DataFrame(rows)

print(df)


stats = df.groupby("client").agg({
    "start_latency": ["mean", "std"],
    "total_latency": ["mean", "std"]
})

print(stats)



avg_total = df.groupby("client")["total_latency"].mean()

avg_total.plot(kind="bar")

plt.ylabel("Average Total Latency (s)")
plt.xlabel("Client ID")
plt.title("Average Total Latency per Client (f = 500)")

plt.show()