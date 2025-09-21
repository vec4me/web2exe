clang++ .\main.cpp -I C:\vcpkg\installed\x64-windows\include\ C:\vcpkg\installed\x64-windows\lib\WebView2Loader.dll.lib -luser32 -m64 -Oz -o main.exe
upx main.exe
rem squishy-x64 -i main.exe -o main.exe