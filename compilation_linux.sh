#!/bin/bash
NBPROC=$(nproc)
echo $NBPROC

#Install Depedency
sudo apt install g++
sudo apt install git
sudo apt install build-essential
sudo apt install cmake
sudo apt install yasm
sudo apt install nasm
sudo apt install meson
sudo apt install autoconf
sudo apt install libtool
sudo apt install patchelf
#sudo apt install opencl-headers
sudo apt install libwebkit2gtk-4.0-dev
sudo apt install freeglut3-dev
sudo apt install mesa-common-dev
#sudo apt install libcurl4-openssl-dev
sudo apt install libssh-dev
sudo apt install liblzma-dev
sudo apt install libva-dev
sudo apt install libsane-dev
sudo apt install libvdpau-dev
sudo apt install libgnutls28-dev
sudo apt install libunistring-dev
sudo apt install libsdl2-dev
sudo apt install libgcrypt20-dev
sudo apt install libsystemd-dev
#sudo apt install libjpeg-dev
sudo apt install curl zip unzip tar
sudo apt install bison
sudo apt install gperf
sudo apt install dos2unix
sudo apt install libgtkglext1-dev


#Install External Library
cd libextern
dos2unix compilation_library_linux.sh
chmod +x compilation_library_linux.sh
./compilation_library_linux.sh
cd ..

