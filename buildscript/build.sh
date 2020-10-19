cd ../../
mkdir -p build/android
cd build/android

NDK_PATH=/usr/lib/android-ndk/
CMAKE=../../third_party/cmake/android/linux/bin/cmake

ABI="armeabi-v7a"
CMAKE_PARAMS=-G "Unix Makefiles"
CMAKE_PARAMS="$CMAKE_PARAMS -DCMAKE_TOOLCHAIN_FILE=$NDK_PATH/build/cmake/android.toolchain.cmake "
CMAKE_PARAMS="$CMAKE_PARAMS -DCMAKE_MAKE_PROGRAM=$NDK_PATH/prebuilt/linux-x86_64/bin/make "
CMAKE_PARAMS="$CMAKE_PARAMS -DANDROID_NATIVE_API_LEVEL=$API_LEVEL"

$CMAKE ../../ $CMAKE_PARAMS
$CMAKE --build ./ --config Release 

