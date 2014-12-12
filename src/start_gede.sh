#!/bin/bash

make
cd ../testapp
../src/gede --args ./test

