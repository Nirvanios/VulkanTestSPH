#!/usr/bin/env bash

cd ./Shaders

glslc ./particle.vert -o ../cmake-build-debug/shaders/particle.spv
#glslc ./Vertex.vert -o ../cmake-build-debug/shaders/vert.spv
glslc ./Fragment.frag -o ../cmake-build-debug/shaders/frag.spv
glslc ./particle2.comp -o ../cmake-build-debug/shaders/comp.spv

cd ..

exit