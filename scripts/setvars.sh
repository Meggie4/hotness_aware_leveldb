#!/bin/bash
export NOVELSMSRC=$PWD
export DBBENCH=$NOVELSMSRC/build
export TEST_TMPDIR=/mnt/ssd
export TEST_MEMDIR=/mnt/pmemdir
#DRAM buffer size in MB
export DRAMBUFFSZ=32
#NVM buffer size in MB
export NVMINDEXSZ=14
export NVMLOGSZ=56
