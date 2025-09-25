cd build/
rem clang neighbours.c -I C:/vcpkg/installed/x86-windows/include/ C:/vcpkg/installed/x86-windows/lib/WebView2Loader.dll.lib -luser32 -m32 -Oz -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -o neighbours.exe
clang -nostdlib neighbours.c -I C:/vcpkg/installed/x86-windows/include/ C:/vcpkg/installed/x86-windows/lib/WebView2Loader.dll.lib -Wl,/ENTRY:WinMainCRTStartup,/SUBSYSTEM:windows -lkernel32 -luser32 -m32 -Oz -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-threadsafe-statics -o neighbours.exe
rem -Wl,/OPT:REF /Wl,/OPT:ICF /Wl,/ENTRY:mainCRTStartup /Wl,/SUBSYSTEM:WINDOWS
rem upx neighbours.exe
rem squishy -i neighbours.exe -o neighbours.exe