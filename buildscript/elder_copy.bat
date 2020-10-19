set PPT_CORE_PATH="H:\Workspace\adhoc-sdk-android\module\libs"
set PPT_SCREENCAST_PATH="H:\Workspace\adhoc-screencapture-android\module\libs"
set PPT_MULTIDISPATCH_PATH="H:\Workspace\adhoc-multidispatch-android\module\libs"

set ABIS=arm64-v8a armeabi-v7a
for %%i IN (%ABIS%) do (
    xcopy "../../build/android/%%i/lib/%%i/libsdklib_adhoc.so" %PPT_CORE_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libadhoc_screencast.so" %PPT_SCREENCAST_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libadhoc_multidispatch.so" %PPT_MULTIDISPATCH_PATH%\%%i /F /Y
)

pause
