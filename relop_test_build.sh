#!/bin/bash

set -e

./build.py clean
./build.py --verbose 

#
for td in testapps/*;do
    if [ -d "$td" ]; then
      echo "$td"
      pushd $td > /dev/null
      make clean
      make
      popd > /dev/null
    fi
done

echo "All builds ok"


