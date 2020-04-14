import urllib.request
import tarfile
import shutil
import os
import uuid
from distutils.dir_util import copy_tree

print('libmypaint:')

libmypaintTmp = 'libmypaintTmp'
libmypaintTar = 'libmypaint-1.5.1-1.pkg.tar.xz'

def removeDirIfExists(path):
    if os.path.exists(path):
        new_name = path + str(uuid.uuid4())
        os.rename(path, new_name)
        shutil.rmtree(new_name)

removeDirIfExists(libmypaintTmp)
os.mkdir(libmypaintTmp)
os.chdir(libmypaintTmp)

url = 'http://repo.msys2.org/mingw/x86_64/mingw-w64-x86_64-libmypaint-1.5.1-1-any.pkg.tar.xz'

urllib.request.urlretrieve(url, libmypaintTar)

print('    Downloaded...')

tar = tarfile.open(libmypaintTar)
tar.extractall()
tar.close()

print('    Extracted...')

os.chdir('..')

includes = 'libmypaint/libmypaint'
removeDirIfExists(includes)
shutil.copytree(libmypaintTmp + '/mingw64/include/libmypaint',
                includes)

print('    Copied includes...')

libs = 'libmypaint/.libs'
removeDirIfExists(libs)
os.mkdir(libs)
copy_tree(libmypaintTmp + '/mingw64/bin', libs)
copy_tree(libmypaintTmp + '/mingw64/lib', libs)
os.rename(libs + '/libmypaint.a', libs + '/mypaint.a')
os.rename(libs + '/libmypaint.dll.a', libs + '/mypaint.dll.a')
os.rename(libs + '/libmypaint-1-5-1.dll', libs + '/mypaint.dll')

print('    Copied libraries...')

shutil.rmtree(libmypaintTmp)

print('    Cleaned up...\n')
