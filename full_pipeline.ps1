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

    docker exec cass-head mkdir -p /sstable_output/wow_packets/packets

    $javaArgs = @(
        "--add-opens", "java.base/java.io=ALL-UNNAMED",
        "--add-opens", "java.base/java.nio=ALL-UNNAMED",
        "--add-opens", "java.base/sun.nio.ch=ALL-UNNAMED",
        "--add-opens", "java.base/java.lang=ALL-UNNAMED",
        "--add-opens", "java.base/java.lang.reflect=ALL-UNNAMED",
        "--add-opens", "java.base/java.util.concurrent=ALL-UNNAMED",
        "--add-opens", "java.base/jdk.internal.ref=ALL-UNNAMED",
        "--add-opens", "java.base/jdk.internal.misc=ALL-UNNAMED",
        "--add-exports", "java.base/jdk.internal.ref=ALL-UNNAMED",
        "--add-exports", "java.base/sun.nio.ch=ALL-UNNAMED",
        "--add-exports", "java.rmi/sun.rmi.registry=ALL-UNNAMED",
        "-jar", "/tools/sstable/target/sstable-1.0.jar",
        "/csv", "/sstable_output"
    )
    docker exec cass-head java @javaArgs
    docker exec cass-head sstableloader -d cass-head /sstable_output/wow_packets/packets/

    docker exec cass-head rm -rf /sstable_output/wow_packets
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

    docker exec cass-head rm -rf /sstable_output/wow_packets 2>$null
}

Invoke-RestMethod -Uri "http://localhost:9200/wow_packets/_forcemerge?max_num_segments=1&wait_for_completion=false" -Method Post | Out-Null

Write-Host ""
Write-Host ">>> DONE YIPEEEEE <<<"
