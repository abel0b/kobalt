#!/usr/bin/env pwsh

$profile = If($args.count -gt 0) { $args[0] } Else { "release" }
$clangfound = Get-Command "clang-cl"
$cc = If(Test-Path env:CC) { $env:CC }
ElseIf($clangfound) { "clang-cl" }
Else { "cl" }
$cflags = If(Test-Path env:CFLAGS) { $env:CFLAGS } Else { "/I..\..\src\include /DWINDOWS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS" }

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

.\dev\vsenv.ps1

$builddir = "build\$profile"
New-Item $builddir -ItemType Directory -ErrorAction SilentlyContinue
Push-Location $builddir
Get-ChildItem "../../dev" -Filter *.binary | Foreach-Object {
    $outexe = $_.Basename
    $outexe = "$outexe.exe"
    $sources = Get-Content $_.FullName | Select-String -Pattern .c -SimpleMatch | ForEach-Object {
      "..\..\$_"
    }
    $sources = $sources -replace "/","\"
    $command = "$cc $cflags $sources /Fe$outexe"
    Write-Host $command
    Invoke-Expression $command
}
Pop-Location
