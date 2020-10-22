cd ../
md build
cd build
md android
cd android
md arm64-v8a
cd arm64-v8a

REM set NDK_PATH=E:\Developer\AndroidNDK\android-ndk-r15c
set NDK_PATH=C:\Users\Administrator\AppData\Local\Android\Sdk\ndk\20.0.5594570
set CMAKE=..\\..\\..\\cmake\\android\\windows\\bin\\cmake.exe

set ABI="arm64-v8a"
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

adb connect 192.168.4.105
adb push ../build/android/arm64-v8a/unittest /sdcard/
adb shell
