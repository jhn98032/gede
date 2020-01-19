#!/bin/bash

set -e

./build.py clean
./build.py --verbose --use-qt4
./build.py clean
./build.py --verbose --use-qt5

