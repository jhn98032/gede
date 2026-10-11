#!/bin/bash

set -e

./build.py clean
./build.py --verbose 


# On OpenIndiana/Illumos make ist not the same as gmake
# To get some tests to compile, we require GNU make
os=$(uname -o)
if [ "$os" = "illumos" ]; then
        Make=gmake
else
        Make=make
fi

#
for td in testapps/*;do
    if [ -d "$td" ]; then
      echo "$td"
      pushd $td > /dev/null
      $Make clean
      $Make
      popd > /dev/null
    fi
done

echo "All builds ok"


