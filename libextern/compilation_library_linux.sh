#/bin/bash
NBPROC=$(nproc)
echo $NBPROC

LOCALPATH=$(pwd)
echo $LOCALPATH

export PKG_CONFIG_PATH=$HOME/ffmpeg_build/lib/pkgconfig

echo "Cuda not found in system"
cp header.h ../include/header.h

unzip vcpkg-2024.08.23_linux.zip
mv vcpkg-2024.08.23 vcpkg

unzip ports_vcpkg.zip

x11=$XDG_SESSION_TYPE
echo $x11

if [ "$x11" = "x11" ]
then
    cp ports_vcpkg/x11/wxwidgets/portfile.cmake vcpkg/ports/wxwidgets/portfile.cmake 
fi

cp -r ports_vcpkg/opencv4 vcpkg/ports

cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg install wxwidgets
./vcpkg install x265
./vcpkg install libde265
./vcpkg install opencv4[contrib,core,ffmpeg,ipp,jpeg,openmp,png,tiff,webp,openexr,opengl,gtk]
./vcpkg install opencl
./vcpkg install tbb
./vcpkg install exiv2[video,xmp,bmff]
./vcpkg install libavif[aom,dav1d]
./vcpkg install libmediainfo
./vcpkg install libexif
./vcpkg install jasper
./vcpkg install libraw[dng-lossy,openmp]
./vcpkg install poppler
./vcpkg install sqlite3
./vcpkg install freeimage
./vcpkg install libjxl
./vcpkg install libepoxy
./vcpkg install boost-lexical-cast
cd ..

#Compile heif-master
unzip heif-master.zip
mv heif-3.6.2 heif-master
cd heif-master/srcs 
cmake ../srcs -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON
make -j$NBPROC
cd ..
cd ..

unzip ffmpeg-master_linux.zip
unzip opencl_dll.zip

#Compile qpdf
FILE=release-qpdf-10.3.2.zip
if [ ! -f FILE ]; then
    wget https://github.com/qpdf/qpdf/archive/refs/tags/release-qpdf-10.3.2.zip
    unzip release-qpdf-10.3.2.zip
fi

#LOCALPATH=$HOME/developpement/git/Regards/libextern
LOCALPATH=$(pwd)
echo $LOCALPATH


export pc_libjpeg_LIBS=$LOCALPATH/vcpkg/installed/x64-linux/lib/libjpeg.a


cd qpdf-release-qpdf-10.3.2
./autogen.sh
./configure --prefix="$HOME/ffmpeg_build" --disable-shared --enable-crypto-native --disable-crypto-openssl --disable-crypto-gnutls 
make -j$NBPROC CXXFLAGS="-I$LOCALPATH/vcpkg/installed/x64-linux/include"

sudo make install
cd ..

rm $LOCALPATH/vcpkg/installed/x64-linux/lib/libpng.a
rm $LOCALPATH/vcpkg/installed/x64-linux/include/nanosvg.h
rm $LOCALPATH/vcpkg/installed/x64-linux/include/nanosvgrast.h
rm $LOCALPATH/vcpkg/installed/x64-linux/lib/libpng.a

