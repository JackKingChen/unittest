cd ../
md build
cd build
md android
cd android
md x86
cd x86

set NDK_PATH=E:\Develop\Android\android-ndk-r20b
set CMAKE="D:\\Program Files\\CMake\\bin\\cmake.exe"

set ABI="x86"
REM set API_LEVEL=android-16
set CMAKE_PARAMS=-G "MinGW Makefiles" 
set CMAKE_PARAMS=%CMAKE_PARAMS% -DCMAKE_TOOLCHAIN_FILE=%NDK_PATH%\build\cmake\android.toolchain.cmake 
set CMAKE_PARAMS=%CMAKE_PARAMS% -DCMAKE_MAKE_PROGRAM=%NDK_PATH%\prebuilt\windows-x86_64\bin\make.exe 
REM set CMAKE_PARAMS=%CMAKE_PARAMS% -DANDROID_NATIVE_API_LEVEL=%API_LEVEL% 
set CMAKE_PARAMS=%CMAKE_PARAMS% -DANDROID_ABI=%ABI%
set CMAKE_PARAMS=%CMAKE_PARAMS% -DANDROID_TOOLCHAIN=clang
set CMAKE_PARAMS=%CMAKE_PARAMS% -DCMAKE_STRIP=%NDK_PATH%\toolchains\aarch64-linux-android-4.9\prebuilt\windows-x86_64\bin\aarch64-linux-android-strip.exe

%CMAKE% ../../../ %CMAKE_PARAMS% -DCMAKE_BUILD_TYPE=Release
%NDK_PATH%\prebuilt\windows-x86_64\bin\make.exe  -j8

cd ../../../buildscript

copy /y E:\Work\unittest\build\android\x86\unittest D:\zymnq\UserData\Android_0\unittest32
