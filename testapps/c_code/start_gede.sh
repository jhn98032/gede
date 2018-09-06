#!/bin/sh

make
make -C ../../src
#cd ../testapps/coredump
../../src/gede --args ./test


