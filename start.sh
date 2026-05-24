#!/bin/bash
docker run -it --rm --name sim1 -v "$(pwd)/simulation:/hpcc" -w /hpcc hpcc bash