import re
import statistics

with open("results.txt", "r") as f:
    data = f.read()

pattern = r"start_latency=([\d.]+)s\s+total_latency=([\d.]+)s"

start_latencies = []
total_latencies = []

for match in re.finditer(pattern, data):
    start_latencies.append(float(match.group(1)))
    total_latencies.append(float(match.group(2)))

print("Start Latency")
print("Mean:", statistics.mean(start_latencies))
print("Std Dev:", statistics.stdev(start_latencies))

print("\nTotal Latency")
print("Mean:", statistics.mean(total_latencies))
print("Std Dev:", statistics.stdev(total_latencies))