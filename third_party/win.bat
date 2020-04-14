python win_libmypaint.py
cd libmypaint\.libs
dumpbin /exports mypaint.dll > exports.txt
echo LIBRARY MYPAINT> mypaint.def
echo EXPORTS >> mypaint.def
for /f "skip=19 tokens=4" %%A in (exports.txt) do echo %%A >> mypaint.def
lib /def:mypaint.def /out:mypaint.lib /machine:x64
cd ..\..
echo Created mypaint lib

python win_ffmpeg.py