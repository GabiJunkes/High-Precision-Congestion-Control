import subprocess
import time

packet_sizes = [
    10240000, 
    102400000,
]
cpu_times = [7812]
algs = [
    "dctcp",
    "hpccPint",
    "timely",
    "dcqcn",
]

traffic = '90_traffic'
cenario = 'inter'

for cc_alg in algs:
    for cpu_time in cpu_times:
        for packet_size in packet_sizes:

            for iteration in range(10):
                internal_cmd = "python2 run_all_cpus.py --trace %s_%s --bw 100 --topo fatk8 --use_playground=1 --cenario=%s --cc_alg=%s --cpu_time=%s --packet_size=%s" % (traffic, iteration, cenario, cc_alg, cpu_time, packet_size)

                docker_cmd = "docker run --name sim%s-%s --rm -v \"$(pwd)/simulation:/hpcc\" -w /hpcc hpcc /bin/bash -c '%s'" % (iteration, cenario, internal_cmd)

                window_title_cmd = "--title=\"Sim%s: %s | Traffic: %s | Size: %s\"" % (iteration, cc_alg, traffic, packet_size)

                # Uses gnome-terminal to open a new window executing the docker command
                print("Spawning terminal for: %s_%s_%s" % (cc_alg, cpu_time, packet_size))
                # print(docker_cmd)

                subprocess.Popen(["gnome-terminal", window_title_cmd, "--", "bash", "-c", docker_cmd])

                time.sleep(12.2) # sleep so config files does not mixup

            print("\n[!] Batch spawned. Waiting for ALL docker containers to close...")
            
            # docker ps -q returns a list of IDs. If it's not empty, containers are running.
            while subprocess.check_output(["docker", "ps", "-q"]):
                time.sleep(120) 
            
            print("[V] Docker is clear. Moving to next configuration.\n")
# docker kill $(docker ps -q)