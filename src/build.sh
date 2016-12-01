#!/bin/bash

BUILDDIR="../build"

if [ ! -d "$BUILDDIR" ]
then
  mkdir $BUILDDIR
fi

g++ -Wall -o ../build/program $1 `sdl2-config --cflags --libs` --std=c++11 -lSDL2_image
