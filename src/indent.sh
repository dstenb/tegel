#! /bin/sh

astyle -C -N -xC79 *.hpp *.cpp && rm -f *.orig
