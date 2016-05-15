#!/bin/bash

cd ..

rm -rf ./Release-build

./bootstrap.sh --prefix=/home/bdarchiev/env \
    --with-boost=/home/bdarchiev/env --boost-version=1.53 \
    --with-yajl=/home/bdarchiev/env \
    --release \
    --with-vx --with-pic

cd -
