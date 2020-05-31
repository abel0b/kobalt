#!/usr/bin/env pwsh

if (Test-Path env:VisualStudioVersion) {
    Exit 0
} 

$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vsfound = Test-Path $vswhere
if (-Not $vsfound) {
    Write-Host "vswhere not found in $vswhere"
    Write-Host "Have you installed Visual Studio?"
    Exit 1
}
$vspath = & $vswhere -property installationPath
$env:VSCMD_SKIP_SENDTELEMETRY = 'No way'
Push-Location "$vspath\Common7\Tools"
cmd /c "VsDevCmd.bat&set" | Foreach {
    if ($_ -match "=") {
        $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
    }
}
Pop-Location
