#Requires -Version 7

$ReleaseName = 'd1-graphics-tool'
$ReleaseVersion = 'beta'
$ReleaseOs = 'windows'
$ReleaseArchitecture = 'x64'
$ReleaseZipFileName = "$ReleaseName-$ReleaseVersion-$ReleaseOs-$ReleaseArchitecture.zip"

$QtPath = 'D:\a\d1-graphics-tool\Qt\6.2.4\msvc2019_64\bin'
$ReleaseExePath = '..\release\D1GraphicsTool.exe'

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
