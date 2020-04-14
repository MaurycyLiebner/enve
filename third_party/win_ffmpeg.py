import os
import zipfile
import urllib.request

print('FFmpeg:')

ffmpegShared = 'ffmpeg-shared.zip'
url = 'https://ffmpeg.zeranoe.com/builds/win64/shared/ffmpeg-4.2.2-win64-shared.zip'
# urllib.request.urlretrieve(url, ffmpegShared)
opener = urllib.request.URLopener()
opener.addheader('User-Agent', 'whatever')
filename, headers = opener.retrieve(url, ffmpegShared)

print('    Downloaded shared...')

ffmpegDev = 'ffmpeg-dev.zip'
url = 'https://ffmpeg.zeranoe.com/builds/win64/dev/ffmpeg-4.2.2-win64-dev.zip'
# urllib.request.urlretrieve(url, ffmpegDev)
opener = urllib.request.URLopener()
opener.addheader('User-Agent', 'whatever')
filename, headers = opener.retrieve(url, ffmpegDev)

print('    Downloaded dev...')

with zipfile.ZipFile(ffmpegShared, 'r') as zip_ref:
    zip_ref.extractall()

print('    Unzipped shared...')

with zipfile.ZipFile(ffmpegDev, 'r') as zip_ref:
    zip_ref.extractall()

print('    Unzipped dev...')

os.remove(ffmpegShared)
os.remove(ffmpegDev)

print('    Cleaned up...')
