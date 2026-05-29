#!/bin/bash

cd build
make -j
ctest --output-on-failure