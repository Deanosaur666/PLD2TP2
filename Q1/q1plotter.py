import matplotlib.pyplot as plt
import numpy as np
import re

with open("./q1log.txt", "r") as file:
    fileStr = file.read()

pattern = r"mpirun -n (\d+).*\n.*Time \(mean ± σ\):\s+(\d+\.\d+) ms"
matches = re.findall(pattern, fileStr)

proc, time = zip(*matches)
proc, time = list(map(int, proc)), list(map(float, time))
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
