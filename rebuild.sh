#!/bin/bash

rm -r build/
node-gyp configure --release
export JOBS=4
node-gyp build --release
#node-gyp configure --debug
#node-gyp build --debug
