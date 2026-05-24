# HPCC simulation

This fork adds a dockerfile for easy development from the original repository.

## How to run

### Build

Build docker and name it hpcc
```
docker build -t hpcc .
```

Run docker with folder `/simulation` mounted to it, so you can modify files inside simulation without the need to rebuild dockerfile
```
docker run -it --rm -v "$(pwd)/simulation:/hpcc" -w /hpcc hpcc bash
```
Or you can configure the start script to easy start this docker file for future use
```
chmod +x ./start.sh
```
And everytime you want to run this docker, run 
```
./start.sh
```

Now, inside hpcc docker, run this command to configure waf to build ns3 with gcc 5
```
CC=gcc-5 CXX=g++-5 python2 ./waf configure
```

Build ns3 with
```
python2 ./waf build
```

Now you can follow README section inside `/simulation` at **Run** section.
- Note: everytime you need to run waf, run it with python2, like the command above.

For generating traffic, you can use the commands
```
docker run --rm -v "$(pwd)/simulation:/hpcc" -w /hpcc hpcc /bin/bash -c 'cd traffic_gen; python2 ./traffic_gen.py -c FbHdp_distribution.txt -n 320 -l 0.3 -b 100G -t 20 -o 30_traffic.txt' 

docker run --rm -v "$(pwd)/simulation:/hpcc" -w /hpcc hpcc /bin/bash -c 'cd traffic_gen; python2 ./traffic_gen.py -c FbHdp_distribution.txt -n 320 -l 0.5 -b 100G -t 20 -o 50_traffic.txt' 
```

Read more at `/simulation/traffic_gen/README.md`