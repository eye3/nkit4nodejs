#!/bin/bash

rm -r build/
node-gyp configure --release
node-gyp build
