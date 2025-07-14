#!/bin/bash
docker run -it --rm -v "$(pwd)/simulation:/hpcc" -w /hpcc hpcc bash