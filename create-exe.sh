#!/bin/bash

APP_DIR=/c/qlcplus
ROOT_DIR=$PWD

# cleanup previous builds
rm -rf $APP_DIR
rm -rf build
mkdir build
cd build

# Build
# export QTDIR=/c/projects/Qt/6.9.0/mingw_64/
if [ -n "$QTDIR" ]; then
 if [ "$1" == "qmlui" ]; then
    cmake -DCMAKE_PREFIX_PATH="$QTDIR/lib/cmake" -Dqmlui=on ..
 else
    cmake -DCMAKE_PREFIX_PATH="$QTDIR/lib/cmake" ..
 fi
else
    echo "QTDIR not set. Aborting."
    exit 1
fi

ninja

if [ ! $? -eq 0 ]; then
    echo Compiler error. Aborting package creation.
    exit $?
fi

# Install to target
ninja install
if [ ! $? -eq 0 ]; then
    echo Installation error. Aborting package creation.
    exit $?
fi

cd ..

echo "Run windeployqt..."
cd $APP_DIR
if [ "$1" == "qmlui" ]; then
  $QTDIR/bin/windeployqt --qmldir $ROOT_DIR/qmlui/qml qlcplusengine.dll Plugins/dmxusb.dll ImediaViever.exe
  rm sceneparsers/gltfsceneexport.dll
else
  $QTDIR/bin/windeployqt qlcplusengine.dll qlcplusui.dll qlcpluswebaccess.dll Plugins/dmxusb.dll ImediaViever.exe
fi

echo "Ensure FFmpeg runtime DLLs are bundled for video playback..."
FFMPEG_DLLS=(
  avcodec-60.dll
  avformat-60.dll
  avutil-58.dll
  swresample-4.dll
  swscale-7.dll
)

for dll in "${FFMPEG_DLLS[@]}"; do
  if [ -f "$dll" ]; then
    continue
  fi

  dll_path=$(find "$QTDIR" -type f -name "$dll" | head -n 1)
  if [ -n "$dll_path" ]; then
    cp "$dll_path" .
  else
    echo "Warning: missing $dll in $QTDIR. Video playback via FFmpeg may not work in the installer package."
  fi
done

# remove uneeded stuff
rm -rf generic networkinformation qmltooling renderplugins tls translations


# Create Installer
makensis -X'SetCompressor /FINAL lzma' qlcplus*.nsi
 
