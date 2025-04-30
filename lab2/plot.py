t1n3 = (0.008301 + 0.010370 + 0.013167 + 0.006707 + 0.004722)/5
t2n3 = (0.012728 + 0.009708 + 0.022870 + 0.010158 + 0.009432)/5
t4n3 = (0.023443 + 0.065109 + 0.020097 + 0.047570 + 0.049391)/5
t1n6 = (0.180307 + 0.145342 + 0.145515 + 0.129947 + 0.151348)/5
t2n6 = (0.151949 + 0.115916 + 0.129085 + 0.133565 + 0.147471)/5
t4n6 = (0.193228 + 0.234760 + 0.174998 + 0.164953 + 0.170134)/5

import matplotlib.pyplot as plt

threads = [1, 2, 4]
tempos = [t1n6, t2n6, t4n6]
aceleracoes = [tempos[0]/t for t in tempos]
eficiencias = [a/t for a, t in zip(aceleracoes, threads)]

plt.figure(figsize=(12, 4))

plt.subplot(1, 3, 1)
plt.plot(threads, tempos, color='blue')
plt.title("Tempo para 10^6")
plt.xlabel("Threads")
plt.ylabel("segundos")

plt.subplot(1, 3, 2)
plt.plot(threads, aceleracoes, color='green')
plt.title("Aceleração para 10^6")
plt.xlabel("Threads")
plt.ylabel("aceleração")

plt.subplot(1, 3, 3)
plt.plot(threads, eficiencias, color='red')
plt.title("Eficiência para 10^6")
plt.xlabel("Threads")
plt.ylabel("eficiência")

plt.tight_layout()
plt.show()
