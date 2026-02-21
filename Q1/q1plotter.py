import matplotlib.pyplot as plt
import numpy as np
import re

times = {}

with open("./q1log.txt", "r") as file:
    p = 0
    for line in file:
        if line[0] == "P":
            p = int(re.search(r"(\d+)", line).group(1))
            times[p] = []
        elif line[0] == "T":
            t = float(re.search(r"(\d*\.\d+)", line).group(1))
            times[p].append(t)

proc = []
time = []
for p in range(1, 11):
    proc.append(p)
    time.append((sum(times[p]) / len(times[p])) * 1000.0) # convert seconds to milliseconds
expectedtime = []
for i in range(len(proc)):
    expectedtime.append(time[0]/(i+1))


plt.plot(proc, time, marker=".", linestyle="-", color="black", label="Real time")
plt.plot(proc, expectedtime, marker=".", linestyle="--", color="black", label="Expected time")

plt.grid(True)
plt.xlabel("Processors")
plt.ylabel("Time (ms)")
plt.legend()
plt.show()
