cd build
cd Release
# embed commit hash and date
LATEST_COMMIT_HASH=\"$(git log --pretty=format:'%h' -n 1)\"
enveSplash=../../src/app/GUI/envesplash.h
echo "#ifndef LATEST_COMMIT_HASH" >> $enveSplash
echo "#define LATEST_COMMIT_HASH $LATEST_COMMIT_HASH" >> $enveSplash
echo "#endif" >> $enveSplash
LATEST_COMMIT_DATE=\"$(git log -1 --format=%ai)\"
echo "#ifndef LATEST_COMMIT_DATE" >> $enveSplash
echo "#define LATEST_COMMIT_DATE $LATEST_COMMIT_DATE" >> $enveSplash
echo "#endif" >> $enveSplash
qmake ../../enve.pro
make -j 2
cd src/app
ls