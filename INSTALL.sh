#!/bin/bash

cmake -D CORE_PRE_COMPILE=True -D CORE_COMPILE=FALSE . && make
cmake -D CORE_PRE_COMPILE=False -D CORE_COMPILE=True . && make
