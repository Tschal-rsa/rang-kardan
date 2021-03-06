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
# bin/PA1 testcases/scene01_basic.txt output/scene01.png
# bin/PA1 testcases/scene02_cube.txt output/scene02.png
# bin/PA1 testcases/scene03_sphere.txt output/scene03.png
# bin/PA1 testcases/scene04_axes.txt output/scene04.png
# bin/PA1 testcases/scene05_bunny_200.txt output/scene05.png
# bin/PA1 testcases/scene06_bunny_1k.txt output/scene06.png
# bin/PA1 testcases/scene07_shine.txt output/scene07.png
# bin/PA1 testcases/scene08_smallpt.txt output/scene08.png
# bin/PA1 testcases/scene09_smallpt.txt output/scene09.png
# bin/PA1 testcases/scene10_bunny.txt output/scene10.png
# bin/PA1 testcases/scene11_cube.txt output/scene11.png
# bin/PA1 testcases/scene12_dof.txt output/scene12.png
# bin/PA1 testcases/scene13_bump.txt output/scene13.png
# bin/PA1 testcases/scene14_bezier.txt output/scene14.png
# bin/PA1 testcases/scene15_bezier.txt output/scene15.png
# bin/PA1 testcases/scene16_living_room.txt output/scene16.png
# bin/PA1 testcases/scene18_sophie.txt output/scene18.png
# bin/PA1 testcases/scene19_forest.txt output/scene19.png
# bin/PA1 testcases/scene20_blur.txt output/scene20.png
bin/PA1 testcases/scene23_normal.txt output/scene23.png