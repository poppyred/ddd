#!/bin/bash

make clean
cd libbase
make clean;make;make clean
cd ../libefly_ev
make clean;make;make clean
cd ..
make
rm -rf *.o
