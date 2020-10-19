set ADHOC_CORE_PATH="D:\Workspace\adhoc-core-android\module\libs\"
set ADHOC_MULTIDISPATCH_PATH="H:\Workspace\adhoc-multidispatch-android\module\libs\"
set ADHOC_CONTROL_PATH="H:\Workspace\adhoc-control-gesture\ControlGesture\libs\"
set ADHOC_TIMESYNC_PATH="H:\Workspace\adhoc-timesync-android\module\libs\"
set ADHOC_SCREENMONITOR_PATH="H:\Workspace\adhoc-screenmonitor-module\module\libs\"
set ADHOC_SCREENCAST_PATH="H:\Workspace\adhoc-screencapture-android\module\libs\"
set ADHOC_AIRPLAY_PATH="H:\Workspace\adhoc-airplay-android\module\libs\"

set ABIS=arm64-v8a armeabi-v7a
for %%i IN (%ABIS%) do (
    xcopy "../../build/android/%%i/lib/%%i/libadhoc_core.so" %ADHOC_CORE_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libadhoc_multidispatch.so" %ADHOC_MULTIDISPATCH_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libadhoc_remotecontrol.so" %ADHOC_CONTROL_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libadhoc_timesync.so" %ADHOC_TIMESYNC_PATH%\%%i /F /Y

    xcopy "../../build/android/%%i/lib/%%i/libadhoc_remotescreen.so" %ADHOC_SCREENMONITOR_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libavcodec-58.so" %ADHOC_SCREENMONITOR_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libavformat-58.so" %ADHOC_SCREENMONITOR_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libavutil-56.so" %ADHOC_SCREENMONITOR_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libswresample-3.so" %ADHOC_SCREENMONITOR_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libswscale-5.so" %ADHOC_SCREENMONITOR_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libndmediasdk.so" %ADHOC_SCREENMONITOR_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libadhoc_screencast.so" %ADHOC_SCREENCAST_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libadhoc_airplay.so" %ADHOC_AIRPLAY_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libraopserver.so" %ADHOC_AIRPLAY_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libfdk-aac.so" %ADHOC_AIRPLAY_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libjdns_sd.so" %ADHOC_AIRPLAY_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libavcodec-58.so" %ADHOC_AIRPLAY_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libavformat-58.so" %ADHOC_AIRPLAY_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libavutil-56.so" %ADHOC_AIRPLAY_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libswresample-3.so" %ADHOC_AIRPLAY_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libswscale-5.so" %ADHOC_AIRPLAY_PATH%\%%i /F /Y
    xcopy "../../build/android/%%i/lib/%%i/libndmediasdk.so" %ADHOC_AIRPLAY_PATH%\%%i /F /Y
)


pause