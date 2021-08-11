#Requires -Version 7

$ReleaseName = 'd1-graphics-tool'
$ReleaseVersion = '0.5.0-beta5'
$ReleaseOs = 'windows'
$ReleaseArchitecture = 'x64'
$ReleaseZipFileName = "$ReleaseName-$ReleaseVersion-$ReleaseOs-$ReleaseArchitecture.zip"

$QtPath = 'C:\Qt\6.1.2\mingw81_64\bin'
$ReleaseExePath = '..\build-D1GraphicsTool-Desktop_Qt_6_1_2_MinGW_64_bit-Release\release\D1GraphicsTool.exe'

if( Test-Path -Path .\$ReleaseZipFileName ) {
    Remove-Item -Path .\$ReleaseZipFileName -Confirm:$false -Force
}

if( $env:Path -notlike "*$QtPath*") {
    $env:Path += ";$QtPath"
}

if( Test-Path -Path .\$ReleaseName ) {
    Remove-Item -Path .\$ReleaseName -Recurse -Confirm:$false -Force
} else {
    New-Item -Path .\$ReleaseName -ItemType Directory
}

# Copy release executable
Copy-Item -Path $ReleaseExePath -Destination .\$ReleaseName

# Copy Qt binaries
windeployqt .\$ReleaseName
# Copy other dependencies
Copy-Item -Path $QtPath\libgcc_s_seh-1.dll -Destination .\$ReleaseName
Copy-Item -Path $QtPath\libstdc++-6.dll -Destination .\$ReleaseName
Copy-Item -Path $QtPath\libwinpthread-1.dll -Destination .\$ReleaseName


Compress-Archive -Path .\$ReleaseName -DestinationPath $ReleaseZipFileName

Remove-Item -Path .\$ReleaseName -Recurse -Confirm:$false -Force
