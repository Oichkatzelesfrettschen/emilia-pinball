#!/bin/sh
# Cross-build Emilia Pinball (GPLv2 engine + assets) for ReactOS/i686.
# SDL2 + OpenGL backend, table modules statically linked (RZR_LIBSTATIC),
# msvcrt CRT so the exe loads on ReactOS.  Data found relative via EM_DATADIR.
set -eu

REPO=$(cd "$(dirname "$0")" && pwd)
S=/tmp/claude-1000/-home-eirikr-Projects/72002929-672c-4daa-aeb4-221b73db1111/scratchpad
SDL=$S/SDL2-2.30.11/i686-w64-mingw32
SDLIMG=$S/SDL2_image-2.8.2/i686-w64-mingw32
SDLMIX=$S/SDL2_mixer-2.8.1/i686-w64-mingw32
OUT=$REPO/bin-reactos
CXX=i686-w64-mingw32-g++

mkdir -p "$OUT/obj"
cd "$REPO"

SHIM=emilia-msvcrt-shim.h
CXXFLAGS="-O2 -std=gnu++14 -w -mcrtdll=msvcrt-os
  -DHAVE_CONFIG_H -DRZR_LIBSTATIC -include $SHIM
  -I. -Isrc -Ibase -Iaddon
  -I$SDL/include/SDL2 -I$SDLIMG/include/SDL2 -I$SDLMIX/include/SDL2"

# LoaderModule.cpp #includes both table module .cpp; do not compile those.
# The msvcrt quick_exit/vsnprintf shim is compiled in alongside engine TUs.
SOURCES=$(find base addon src -name '*.cpp' | grep -vE 'Module(Tux|Professor)\.cpp')
SOURCES="$SOURCES emilia-msvcrt-shim.cpp"

echo "Compiling $(echo "$SOURCES" | wc -l) translation units..."
for src in $SOURCES; do
  obj="$OUT/obj/$(echo "$src" | tr '/' '_').o"
  $CXX $CXXFLAGS -c "$src" -o "$obj"
done

echo "Linking Pinball.exe..."
$CXX -mcrtdll=msvcrt-os -mwindows -static-libgcc -static-libstdc++ "$OUT"/obj/*.o \
  -L"$SDL/lib" -L"$SDLIMG/lib" -L"$SDLMIX/lib" \
  -lmingw32 -lSDL2 -lSDL2_image -lSDL2_mixer \
  -lopengl32 -lglu32 \
  -Wl,-Bstatic -lwinpthread -Wl,-Bdynamic \
  -o "$OUT/Pinball.exe"

echo "Done: $OUT/Pinball.exe"
i686-w64-mingw32-objdump -p "$OUT/Pinball.exe" | grep 'DLL Name'
