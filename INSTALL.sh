#!/bin/bash

cmake -DCORE_COMPILE_STEP=ON -DCMAKE_BUILD_TYPE=Release . && make
