rem Setup Qt environment
set PATH=C:\Qt\5.12.6\mingw73_32\bin;%PATH%

rem Copy executable file to temp folder
copy C:\Users\Christophe\GitHub\d1-graphics-tool\build-D1GraphicsTool-Desktop_Qt_5_12_6_MinGW_32_bit-Release\release\D1GraphicsTool.exe .\temp

rem Copy appropriate dependencies to exe folder
C:\Qt\5.12.6\mingw73_32\bin\windeployqt.exe .\temp
copy C:\Qt\5.12.6\mingw73_32\bin\libgcc_s_dw2-1.dll .\temp
copy "C:\Qt\5.12.6\mingw73_32\bin\libstdc++-6.dll" .\temp
copy C:\Qt\5.12.6\mingw73_32\bin\libwinpthread-1.dll .\temp
