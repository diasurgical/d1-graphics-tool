rem Setup Qt environment
set PATH=C:\Qt\5.15.2\mingw81_64\bin;%PATH%

rem Copy executable file to temp folder
copy C:\Users\Christophe\GitHub\d1-graphics-tool\build-D1GraphicsTool-Desktop_Qt_5_15_2_MinGW_64_bit-Release\release\D1GraphicsTool.exe .\temp

rem Copy appropriate dependencies to exe folder
C:\Qt\5.15.2\mingw81_64\bin\windeployqt.exe .\temp
copy C:\Qt\5.15.2\mingw81_64\bin\libgcc_s_seh-1.dll .\temp
copy "C:\Qt\5.15.2\mingw81_64\bin\libstdc++-6.dll" .\temp
copy C:\Qt\5.15.2\mingw81_64\bin\libwinpthread-1.dll .\temp
