# Runs a PS-X EXE in two PCSX-Redux linked through their serial ports (SIO1),
# like two consoles joined by a link cable: the first is the SIO1 server, the
# second connects to it as the client.
#
#   powershell -ExecutionPolicy Bypass -File run-linked.ps1 -Emulator C:\pcsx-redux\pcsx-redux.exe [-Exe psx.exe]
#
# Each emulator gets its own settings folder (.pcsx-link\server, .pcsx-link\client
# next to the exe), made from your usual PCSX-Redux settings (BIOS, controls...)
# with SIO1 turned on. Your own settings aren't changed.
#
# Build the program with YarIO's emulator support (-DYARIO_EMU), so its tty
# uses the serial port.
param(
    [Parameter(Mandatory = $true)][string]$Emulator,
    [string]$Exe = 'psx.exe',
    [int]$Port = 6699,
    [int]$ServerGdbPort = 0,    # to debug either emulator with GDB (0: off)
    [int]$ClientGdbPort = 0
)
$ErrorActionPreference = 'Stop'

if (-not (Test-Path $Emulator)) { throw "PCSX-Redux not found: $Emulator" }
if (-not (Test-Path $Exe)) { throw "Program not found: $Exe (build it first)" }
$Exe = (Resolve-Path $Exe).Path
$root = Join-Path (Split-Path $Exe) '.pcsx-link'

# Your usual settings: the ones next to the emulator (portable), or in AppData
$base = Join-Path (Split-Path $Emulator) 'pcsx.json'
if (-not (Test-Path $base)) { $base = Join-Path $env:APPDATA 'pcsx-redux\pcsx.json' }

function Set-Field($object, [string]$name, $value) {
    if ($object.PSObject.Properties[$name]) { $object.$name = $value }
    else { $object | Add-Member -NotePropertyName $name -NotePropertyValue $value }
}

function Get-Section($object, [string]$name) {
    if (-not $object.PSObject.Properties[$name]) { Set-Field $object $name ([pscustomobject]@{}) }
    return $object.$name
}

function New-Settings([string]$name, [bool]$server, [int]$gdbPort, [int]$xOffset) {
    $dir = Join-Path $root $name
    New-Item -ItemType Directory -Force $dir | Out-Null
    $settings = if (Test-Path $base) { Get-Content $base -Raw | ConvertFrom-Json } else { [pscustomobject]@{} }

    $emulator = Get-Section $settings 'emulator'
    $debug = Get-Section $emulator 'Debug'
    Set-Field $debug 'SIO1Server' $server
    Set-Field $debug 'SIO1ServerPort' $Port
    Set-Field $debug 'SIO1Client' (-not $server)
    Set-Field $debug 'SIO1Clienthost' '127.0.0.1'
    Set-Field $debug 'SIO1ClientPort' $Port
    Set-Field $debug 'SIO1Mode' 0                  # Protobuf: carries RTS/DTR too
    Set-Field $debug 'GdbServer' ($gdbPort -ne 0)
    if ($gdbPort -ne 0) { Set-Field $debug 'Debug' $true; Set-Field $debug 'GdbServerPort' $gdbPort }
    Set-Field $debug 'WebServer' $false

    # Side by side
    $gui = Get-Section $settings 'gui'
    $x = if ($gui.PSObject.Properties['WindowPosX']) { [int]$gui.WindowPosX } else { 50 }
    Set-Field $gui 'WindowPosX' ($x + $xOffset)
    Set-Field $gui 'WindowMaximized' $false
    Set-Field $gui 'Fullscreen' $false

    $json = $settings | ConvertTo-Json -Depth 32
    [IO.File]::WriteAllText((Join-Path $dir 'pcsx.json'), $json)
    return $dir
}

function Start-Emulator([string]$dir) {
    $arguments = @('-portable', "`"$dir`"", '-loadexe', "`"$Exe`"", '-run')
    return Start-Process -FilePath $Emulator -ArgumentList $arguments -WorkingDirectory $dir -PassThru
}

$serverDir = New-Settings 'server' $true $ServerGdbPort 0
$clientDir = New-Settings 'client' $false $ClientGdbPort 700

$server = Start-Emulator $serverDir
Write-Host "SIO1 server: PCSX-Redux (pid $($server.Id)), listening on port $Port"
Start-Sleep -Seconds 3      # the client connects once, when it starts
$client = Start-Emulator $clientDir
Write-Host "SIO1 client: PCSX-Redux (pid $($client.Id)), connecting to 127.0.0.1:$Port"
