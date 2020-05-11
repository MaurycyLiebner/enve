cd build
cd Release
qmake CONFIG+=build_examples ../../enve.pro
make -j 2
cd src/app