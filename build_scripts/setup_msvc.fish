#!/bin/fish

set -e WINEPREFIX
set -e WINEARCH
set -e MSVC_PATH

# set the wine prefix and arch
set WINE_HOME (pwd)/.wineenv
set WINEPREFIX $WINE_HOME
set WINEARCH win64
set MSVC_INSTALL_PATH $HOME/msvc/
set MSVC_PATH $HOME/msvc/bin/x64

git submodule init
git submodule update

./build_scripts/msvc-wine/vsdownload.py --dest $MSVC_INSTALL_PATH --accept-license
./build_scripts/msvc-wine/install.sh $MSVC_INSTALL_PATH

wineserver -k
wineserver -p
wine wineboot

winetricks settings win10
