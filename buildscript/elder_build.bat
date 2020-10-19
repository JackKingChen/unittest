call elder_build_v8a.bat

md build
cd build
md android
cd android
md armeabi-v7a
cd armeabi-v7a

set NDK_PATH=H:\Android\SDK\ndk-bundle\android-ndk-r13b
set CMAKE=..\\..\\..\\third_party\\cmake\\android\\windows\\bin\\cmake.exe

set ABI="armeabi-v7a"
set API_LEVEL=android-16
set CMAKE_PARAMS=-G "MinGW Makefiles" 
set CMAKE_PARAMS=%CMAKE_PARAMS% -DCMAKE_TOOLCHAIN_FILE=%NDK_PATH%\build\cmake\android.toolchain.cmake 
set CMAKE_PARAMS=%CMAKE_PARAMS% -DCMAKE_MAKE_PROGRAM=%NDK_PATH%\prebuilt\windows-x86_64\bin\make.exe 
set CMAKE_PARAMS=%CMAKE_PARAMS% -DANDROID_NATIVE_API_LEVEL=%API_LEVEL% 
set CMAKE_PARAMS=%CMAKE_PARAMS% -DBUILD_ELDER=ON
set CMAKE_PARAMS=%CMAKE_PARAMS% -DANDROID_ABI=%ABI%
set CMAKE_PARAMS=%CMAKE_PARAMS% -DANDROID_TOOLCHAIN=clang
set CMAKE_PARAMS=%CMAKE_PARAMS% -DCMAKE_STRIP=%NDK_PATH%\toolchains\arm-linux-androideabi-4.9\prebuilt\windows-x86_64\bin\arm-linux-androideabi-strip.exe

%CMAKE% ../../../ %CMAKE_PARAMS% -DCMAKE_BUILD_TYPE=Release
%NDK_PATH%\prebuilt\windows-x86_64\bin\make.exe  -j8



pause