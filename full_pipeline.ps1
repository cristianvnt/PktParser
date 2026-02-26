param(
    [Parameter(Mandatory=$true)]
    [string]$PktPath,
    [string]$ParserVersion = ""
)

$ErrorActionPreference = "Stop"

$csvDir = ".\csv"
$sstableOut = ".\sstable_output"

if (!(Test-Path "tools\sstable\target\sstable-1.0.jar"))
{
    Write-Host ">>> Building SSTable tool <<<"
    Push-Location tools\sstable
    mvn package -q
    Pop-Location
}

if (Test-Path "$csvDir\*.csv")
{
    Remove-Item "$csvDir\*.csv"
}
if (Test-Path $sstableOut)
{
    Remove-Item -Recurse -Force $sstableOut
}

Invoke-RestMethod -Uri "http://localhost:9200/wow_packets/_settings" `
    -Method Put -ContentType "application/json" `
    -Body '{"refresh_interval": "-1"}' | Out-Null

Write-Host ">>>>> BULK LOAD PIPELINE <<<<<"

try
{
    $parserArgs = @($PktPath, "--export")
    if ($ParserVersion)
    {
        $parserArgs += "--parser-version", $ParserVersion
    }
    & ".\build\RelWithDebInfo\PktParser.exe" @parserArgs

    & ".\utils\run_sstable.ps1" $csvDir $sstableOut

    sstableloader -d 127.0.0.1 "$sstableOut\wow_packets\packets\"
}
finally
{
    Invoke-RestMethod -Uri "http://localhost:9200/wow_packets/_settings" `
        -Method Put -ContentType "application/json" `
        -Body '{"refresh_interval": "5s"}' | Out-Null

    if (Test-Path "$csvDir\*.csv")
    {
        Remove-Item "$csvDir\*.csv"
    }
    if (Test-Path $sstableOut)
    {
        Remove-Item -Recurse -Force $sstableOut
    }
}

Invoke-RestMethod -Uri "http://localhost:9200/wow_packets/_forcemerge?max_num_segments=1&wait_for_completion=false" -Method Post | Out-Null

Write-Host ""
Write-Host ">>> DONE YIPEEEEE <<<"
