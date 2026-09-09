param(
    [Parameter(Mandatory = $true)][string]$Port,
    [string]$Python = 'python'
)
$ErrorActionPreference = 'Stop'
$wakeBinDir = Join-Path $PSScriptRoot 'bin\esp32s3_ch340'
$wakeFiles = @('bootloader.bin', 'partitions.bin', 'boot_app0.bin', 'firmware.bin')
foreach ($wakeName in $wakeFiles) {
    if (-not (Test-Path -LiteralPath (Join-Path $wakeBinDir $wakeName))) {
        throw "Missing binary: $wakeName"
    }
}
$wakeArgs = @(
    '-m', 'esptool', '--chip', 'esp32s3', '--port', $Port, '--baud', '115200',
    '--before', 'default_reset', '--after', 'hard_reset',
    'write_flash', '--flash_mode', 'dio', '--flash_freq', '80m', '--flash_size', '16MB',
    '0x0000', (Join-Path $wakeBinDir 'bootloader.bin'),
    '0x8000', (Join-Path $wakeBinDir 'partitions.bin'),
    '0xe000', (Join-Path $wakeBinDir 'boot_app0.bin'),
    '0x10000', (Join-Path $wakeBinDir 'firmware.bin')
)
& $Python @wakeArgs
if ($LASTEXITCODE -ne 0) { throw "esptool failed: $LASTEXITCODE" }
Write-Host 'XboxWake 0.3.0 flashed. Open the serial monitor at 115200 baud; send INFO.'
