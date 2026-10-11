#!/bin/bash

BUILDDIR=$(pwd)/AppDir-build

if [ ! -e linuxdeploy-x86_64.AppImage ];then
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
        
    chmod +x linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
fi

rm -Rf $BUILDDIR
mkdir -p $BUILDDIR

./build.py --build-all --prefix=$BUILDDIR

# Install into the AppDir staging directory
./build.py --prefix=$BUILDDIR install


chmod +x linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage


# Enable qt plugin
#export EXTRA_QT_PLUGINS="svg"
#export EXTRA_QT_PLUGINS="iconengines"
export EXTRA_QT_MODULES="imageformats"
#export QT_PLUGIN_PATH=/usr/lib/x86_64-linux-gnu/qt5/plugins
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH

# Generate AppImage
#export COMPRESS_TYPE=xz
./linuxdeploy-x86_64.AppImage \
    --appdir $BUILDDIR \
    --desktop-file=AppDir/gede.desktop \
    --icon-file=AppDir/gede_icon.png \
    --executable=src/gede \
    --plugin qt \
    --output appimage 


     
