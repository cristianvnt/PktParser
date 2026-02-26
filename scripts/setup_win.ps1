$ErrorActionPreference = "Stop"

# vcpkg
if (!$env:VCPKG_ROOT -or !(Test-Path "$env:VCPKG_ROOT"))
{
    Write-Host "Installing vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
    C:\vcpkg\bootstrap-vcpkg.bat
    [Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "Machine")
    $env:VCPKG_ROOT = "C:\vcpkg"
    Write-Host "VCPKG_ROOT set - restart terminal after setup"
}
else
{
    Write-Host "vcpkg found at $env:VCPKG_ROOT"
}

# Python
if (!(Get-Command python3 -ErrorAction SilentlyContinue) -and !(Get-Command python -ErrorAction SilentlyContinue))
{
    Write-Host "Python not found. Install from https://www.python.org/downloads/" -ForegroundColor Yellow
}
else
{
    Write-Host "Installing Python dependencies..."
    pip install psycopg2-binary python-dotenv
}

# Java
if (!(Get-Command java -ErrorAction SilentlyContinue))
{
    Write-Host "Java not found. Install JDK 17" -ForegroundColor Yellow
}
else
{
    Write-Host "Java found: $(java --version 2>&1 | Select-Object -First 1)"
}

# Maven
if (!(Get-Command mvn -ErrorAction SilentlyContinue))
{
    Write-Host "Maven not found. Install from https://maven.apache.org/download.cgi" -ForegroundColor Yellow
}
else
{
    Write-Host "Maven found"
}

Write-Host ""
Write-Host ">>> Setup complete <<<" -ForegroundColor Green