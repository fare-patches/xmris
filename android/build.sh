#!/usr/bin/env bash
# Build from the same SDL release for its Java activity, JNI glue and C library.
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
out="$root/build/android"
sdl="$out/SDL3-3.4.20"
mkdir -p "$out"
if [[ ! -d "$sdl" ]]; then
  curl -fL --retry 3 https://github.com/libsdl-org/SDL/releases/download/release-3.4.20/SDL3-3.4.20.tar.gz -o "$out/sdl.tar.gz"
  (cd "$out" && echo 'f3bb06f55bed9557227a700af2ea62e49497ebdf58037732a731a5fb3ef8581f  sdl.tar.gz' | sha256sum -c - && tar xf sdl.tar.gz)
fi
make -C "$root" -f Makefile.sdl bitmaps/sprites
sdk=${ANDROID_HOME:?Set ANDROID_HOME to the Android SDK directory}
ndk="$sdk/ndk/28.2.13676358"
cmake="$sdk/cmake/3.22.1/bin"
tools="$sdk/build-tools/35.0.0"
platform="$sdk/platforms/android-35/android.jar"
export XMRIS_SOURCE="$root"
stage="$out/stage"
mkdir -p "$stage/lib/arm64-v8a" "$out/classes" "$out/res/drawable-nodpi"
"$cmake/cmake" -S "$root/android" -B "$out/native" -G Ninja \
  -DCMAKE_MAKE_PROGRAM="$cmake/ninja" -DCMAKE_TOOLCHAIN_FILE="$ndk/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21 -DANDROID_STL=c++_static \
  -DCMAKE_BUILD_TYPE=Release -DSDL_SOURCE="$sdl" -DSDL_SHARED=ON -DSDL_STATIC=OFF
"$cmake/cmake" --build "$out/native" --target main --parallel 6
strip="$ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip"
for name in main SDL3; do
  so=$(find "$out/native" -name "lib$name.so" -type f -print -quit)
  "$strip" --strip-unneeded "$so" -o "$stage/lib/arm64-v8a/lib$name.so"
done
activity="$sdl/android-project/app/src/main"
find "$activity/java" -name '*.java' > "$out/java-sources"
javac --release 8 -classpath "$platform" -d "$out/classes" @"$out/java-sources"
jar cf "$out/classes.jar" -C "$out/classes" .
"$tools/d8" --min-api 21 --lib "$platform" --output "$stage" "$out/classes.jar"
cp "$root/xmris.png" "$out/res/drawable-nodpi/xmris.png"
"$tools/aapt2" compile --dir "$out/res" -o "$out/resources.zip"
"$tools/aapt2" link -I "$platform" --manifest "$root/android/AndroidManifest.xml" \
  -o "$out/unsigned.apk" "$out/resources.zip"
(cd "$stage" && zip -q "$out/unsigned.apk" classes.dex lib/arm64-v8a/*.so)
"$tools/zipalign" -f -P 16 4 "$out/unsigned.apk" "$out/aligned.apk"
key="${ANDROID_USER_HOME:-$HOME/.android}/debug.keystore"
if [[ ! -f "$key" ]]; then
  mkdir -p "$(dirname "$key")"
  keytool -genkeypair -keystore "$key" -storepass android -keypass android \
    -alias androiddebugkey -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Android Debug"
fi
"$tools/apksigner" sign --ks "$key" --ks-pass pass:android --key-pass pass:android \
  --ks-key-alias androiddebugkey --out "$out/xmris-arm64-debug.apk" "$out/aligned.apk"
"$tools/apksigner" verify "$out/xmris-arm64-debug.apk"
