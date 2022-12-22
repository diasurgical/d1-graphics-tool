#!/bin/bash

# Copy latest release build
mkdir -p debian/usr/bin/
cp build/D1GraphicsTool debian/usr/bin/
# Strip symbols
strip -s debian/usr/bin/D1GraphicsTool

# Copy short cut icon
mkdir -p debian/opt/d1-graphics-tool/
cp source/icon.svg debian/opt/d1-graphics-tool/

# Build .deb package
dpkg-deb --build debian

# Rename package
mv debian.deb d1-graphics-tool.deb
