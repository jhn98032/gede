#!/bin/sh

set -e

cd ../testapp
ls -l ./test
gdbserver localhost:2345 ./test

