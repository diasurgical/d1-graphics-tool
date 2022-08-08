#!/bin/bash

# Copy latest release build
cp build-D1GraphicsTool-Desktop_Qt_6_2_4_GCC_64bit-Release/D1GraphicsTool debian/usr/bin/
# Strip symbols
strip -s debian/usr/bin/D1GraphicsTool

# Copy short cut icon
cp source/icon.svg debian/opt/d1-graphics-tool/

# Build .deb package
dpkg-deb --build debian

# Rename package
mv debian.deb d1-graphics-tool.deb
