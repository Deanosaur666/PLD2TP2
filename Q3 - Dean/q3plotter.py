import matplotlib.pyplot as plt
import numpy as np
import re

times1 = {}
maxp = 0

C = None

with open("./q3log1.txt", "r") as file:
    p = 0
    for line in file:
        if line[0] == "P":
            p = int(re.search(r"(\d+)", line).group(1))
            times1[p] = []
            maxp = max(maxp, p)
        elif line[0] == "T":
            t = float(re.search(r"(\d*\.\d+)", line).group(1))
            times1[p].append(t)
        elif line[0] == "C":
            c = int(re.search(r"(\d+) primes", line).group(1))
            if C == None:
                C = c
            elif c != C:
                print("WRONG NUMBER! %d does not match %d!" % (c, C))

proc = []
time1 = []
for p in range(1, maxp+1):
    time1.append((sum(times1[p]) / len(times1[p])) * 1000.0) # convert seconds to milliseconds

times2 = {}
with open("./q3log2.txt", "r") as file:
    p = 0
    for line in file:
        if line[0] == "P":
            p = int(re.search(r"(\d+)", line).group(1))
            times2[p] = []
            maxp = max(maxp, p)
        elif line[0] == "T":
            t = float(re.search(r"(\d*\.\d+)", line).group(1))
            times2[p].append(t)
        elif line[0] == "C":
            c = int(re.search(r"(\d+) primes", line).group(1))
            if C == None:
                C = c
            elif c != C:
                print("WRONG NUMBER! %d does not match %d!" % (c, C))

print(C)

time2 = []
proc = []
for p in range(1, maxp+1):
    proc.append(p)
    time2.append((sum(times2[p]) / len(times2[p])) * 1000.0) # convert seconds to milliseconds



plt.plot(proc, time1, marker=".", linestyle="-", color="black", label="Normal sieve time")
plt.plot(proc, time2, marker=".", linestyle="-", color="red", label="Pipelined sieve time")

plt.grid(True)
plt.xlabel("Processors")
plt.ylabel("Time (ms)")
plt.legend()
plt.show()
