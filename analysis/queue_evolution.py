# salva como plot_queue.py
import matplotlib.pyplot as plt

times = []
queue_sizes = []

with open("queue_trace_dctcp.txt", "r") as f:
    for line in f:
        parts = line.strip().split()
        if len(parts) < 4:
            continue
        try:
            time = int(parts[0])  # converte de ns para ms
            queue = int(parts[3])      # em bytes
            times.append(time)
            queue_sizes.append(queue)
        except ValueError:
            continue

plt.figure(figsize=(10, 5))
plt.plot(times, queue_sizes, marker=',', linestyle='-')
plt.xlabel("Tempo (ns)")
plt.ylabel("Tamanho da fila (bytes)")
plt.title("Evolução do tamanho da fila ao longo do tempo")
plt.grid(True)
plt.tight_layout()
plt.savefig("queue_evolution.png", dpi=300)
plt.show()