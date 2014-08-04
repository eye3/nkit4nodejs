#!/bin/bash

rm -r build/
node-gyp configure --release
export NKIT_ROOT=$HOME/env
node-gyp build

