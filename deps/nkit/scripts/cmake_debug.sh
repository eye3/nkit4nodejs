#!/bin/bash

cd ..

rm -rf ./Debug-build

./bootstrap.sh --prefix=/home/bdarchiev/env \
    --with-boost=/home/bdarchiev/env --boost-version=1.54 \
    --with-yajl=/home/bdarchiev/env \
    --debug \
    --with-vx --with-pic
cd -
