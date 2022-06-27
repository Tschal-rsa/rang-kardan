#!/usr/bin/env bash

# If project not ready, generate cmake file.
if [[ ! -d build ]]; then
    echo "good"
else
    rm -rf build
fi
mkdir -p build
cd build
cmake ..
make -j
cd ..

# Run all testcases. 
# You can comment some lines to disable the run of specific examples.
mkdir -p output
# bin/PA1 testcases/scene01_basic.txt output/scene01.bmp
# bin/PA1 testcases/scene02_cube.txt output/scene02.bmp
# bin/PA1 testcases/scene03_sphere.txt output/scene03.bmp
# bin/PA1 testcases/scene04_axes.txt output/scene04.bmp
# bin/PA1 testcases/scene05_bunny_200.txt output/scene05.bmp
# bin/PA1 testcases/scene06_bunny_1k.txt output/scene06.bmp
# bin/PA1 testcases/scene07_shine.txt output/scene07.bmp
# bin/PA1 testcases/scene08_smallpt.txt output/scene08.bmp
# bin/PA1 testcases/scene09_smallpt.txt output/scene09.bmp
# bin/PA1 testcases/scene10_bunny.txt output/scene10.bmp
# bin/PA1 testcases/scene11_cube.txt output/scene11.bmp
# bin/PA1 testcases/scene12_dof.txt output/scene12.bmp
bin/PA1 testcases/scene13_bump.txt output/scene13.bmp
# bin/PA1 testcases/scene14_bezier.txt output/scene14.bmp
# bin/PA1 testcases/scene15_bezier.txt output/scene15.bmp