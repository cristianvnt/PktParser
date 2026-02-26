param(
    [ValidateSet("Debug", "Release", "RelWithDebInfo")]
    [string]$BuildType = "RelWithDebInfo",
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

if ($Clean)
{
    Write-Host "Cleaning build directory..."
    if (Test-Path build)
    { 
        Remove-Item -Recurse -Force build
    }
}

Write-Host "Building: $BuildType"

if (!(Test-Path build))
{ 
    New-Item -ItemType Directory -Path build | Out-Null
}

cmake -B build -S .
cmake --build build --config $BuildType --parallel

Write-Host "Build complete: build\$BuildType\PktParser.exe"