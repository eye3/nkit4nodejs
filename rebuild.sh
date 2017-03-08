#!/bin/bash

rm -r build/
export JOBS=4
node-gyp configure --release
node-gyp build --release
#node-gyp configure --debug
#node-gyp build --debug
