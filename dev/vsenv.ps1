#!/usr/bin/env pwsh

if (Test-Path env:VisualStudioVersion) {
    Exit 0
}

$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vsfound = Test-Path $vswhere
If (-Not $vsfound) {
    Write-Host "vswhere not found in $vswhere"
    Write-Host "Have you installed Visual Studio?"
    Write-Host "https://visualstudio.microsoft.com/vs/"
    Exit 1
}
$vspath = & $vswhere -property installationPath
Push-Location "$vspath\Common7\Tools"
cmd /c "VsDevCmd.bat -arch=amd64&set" | Foreach {
    If ($_ -match "=") {
        $v = $_.split("="); Set-Item -force -path "ENV:\$($v[0])" -value "$($v[1])"
    }
}
Pop-Location
