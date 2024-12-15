Gede
----

Gede is a graphical frontend (GUI) to GDB written in C++ and using the Qt5/Qt6 toolkit.
Gede can be compiled and run on (all?) Linux distributions, FreeBSD and macOS.
Gede supports debugging programs written in Ada, FreeBasic, C++, C, Rust, Fortran and Go.

.. image:: https://gede.dexar.se/uploads/Main/screenshot_1.13.1_a.png
   :alt: Screenshot
   :width: 300px
   :align: left

Gede was written by Johan Henriksson.
See LICENSE file for license information.

The icons used are from the NetBeans project and licensed under the
terms of the NetBeans License Agreement.

For more information see https://gede.dexar.se.

Appimage
========
An Appimage is supplied with each release.
To use it:

  chmod a+x Gede-x86_64.AppImage
  ./Gede-x86_64.AppImage

Dependencies
============

Gede depends on Qt5 or Qt6 and exuberant-ctags.

Make sure that you have all dependencies installed.

On Debian/Ubuntu/Mint:

    # sudo apt-get install gdb

    # sudo apt-get install g++

    # sudo apt-get install exuberant-ctags

    # sudo apt-get install python

    # sudo apt-get install qt5-qmake qtbase5-dev libqt5serialport5-dev

On Redhat/Centos:

    # sudo yum install gdb

    # sudo yum install gcc

    # sudo yum install gcc-c++

    # sudo yum install ctags

    # sudo yum install qt5-designer qt5-devel qt5-qtserialport

On FreeBSD:

    # pkg install gdb

    # pkg install universal-ctags-g20180225

    # pkg install qt5

    # pkg install qt5-qtserialport

    # pkg install gcc

On macOS (use homebrew):

    # brew install universal-ctags

    # brew install qt@5

    # brew link --force qt@5

Building
========

Extract:

    # tar -xvJf gede-x.y.z.tar.xz


Compile and install (to /usr/local/bin):

    # sudo make install

Gede can now be launched:

    # gede

