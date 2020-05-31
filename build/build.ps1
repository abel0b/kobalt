#!/usr/bin/env pwsh

$profile = If(Test-Path env:PROFILE) { $env:PROFILE } Else { "release" }
$cc = If(Test-Path env:CC) { $env:CC } Else { "cl.exe" }
$cflags = If(Test-Path env:CFLAGS) { $env:CFLAGS } Else { "/DWINDOWS=1 /I../../include" }

If ($profile -Eq "release" -Or "$profile" -Eq "test" ) {
    $cflags = "$cflags /O2"
}    
ElseIf ($profile -Eq "debug") {
    $cflags = "$cflags /DDEBUG=1"
}
Else {
    Write-Host "Unknown profile $profile"
    Exit 1
}

Write-Host "profile = $profile"
Write-Host "cc = $cc"
Write-Host "cflags = $cflags"

.\build\vsenv.ps1

$distdir = "dist\$profile"
New-Item $distdir -ItemType Directory -ErrorAction SilentlyContinue
Push-Location $distdir
Get-ChildItem "../../build" -Filter *.binary | Foreach-Object {
    $outexe = $_.Basename
    $outexe = "$outexe.exe"
    $sources = Get-Content $_.FullName | Select-String -Pattern .c -SimpleMatch | ForEach-Object {
      "../../$_"
    }
    $command = "$cc $cflags $sources /Fe$outexe"
    Write-Host $command
    Invoke-Expression $command
}
Pop-Location
