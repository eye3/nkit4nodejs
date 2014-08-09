#!/bin/bash

rm -r build/
node-gyp configure --release
node-gyp build --release
#node-gyp configure --debug
#node-gyp build --debug
