# Runs the game into West Coast driving (normal rules) without a person at the
# keyboard, for measurements (docs/guides/driving-measurement.md).
#
# Keys are posted to the game window as WM_KEYDOWN/WM_KEYUP with
# REXBOX_INPUT=keyboard; keybd_event is not used because it goes to whichever
# window has the focus. The schedule from launch (40 s Enter, 43 s Enter,
# 46 s Z, 49 s Enter, 52 s Z, 57 s Z) reaches driving with no pause. Extra
# environment variables (REXBOX_D3D8_PROFILE, REXBOX_SAMPLE, ...) apply to
# this run only; run each measurement in its own PowerShell process so they
# do not carry over.
param(
    [string]$Log = "drive.log",
    [int]$Seconds = 130,
    [hashtable]$Env = @{},
    [string]$Exe = ""
)

$ErrorActionPreference = "Stop"

$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
if ($Exe -eq "")
{
    $Exe = Join-Path $Root "build\win32_x86\src\app\Release\rect3.exe"
}

if (-not ("DriveWindow" -as [type]))
{
    Add-Type @"
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class DriveWindow {
  delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc f, IntPtr l);
  [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
  // The process's SDL window, visible or hidden (REXBOX_DISPLAY=hidden).
  public static IntPtr Find(uint pid) {
    IntPtr found = IntPtr.Zero;
    EnumWindows((h, l) => {
      uint p; GetWindowThreadProcessId(h, out p);
      if (p == pid) {
        var name = new StringBuilder(256); GetClassName(h, name, 256);
        if (name.ToString().StartsWith("SDL")) { found = h; return false; }
      }
      return true; }, IntPtr.Zero);
    return found;
  }
}
"@
}

# The variables are set for the game's process only and restored after it starts.
$variables = @{ REXBOX_INPUT = "keyboard" } + $Env
$saved = @{}
foreach ($name in $variables.Keys)
{
    $saved[$name] = [Environment]::GetEnvironmentVariable($name)
    [Environment]::SetEnvironmentVariable($name, [string]$variables[$name])
}
try
{
    $process = Start-Process -FilePath $Exe -ArgumentList "roms" -WorkingDirectory $Root `
        -RedirectStandardError $Log -RedirectStandardOutput "$Log.out" -PassThru
}
finally
{
    foreach ($name in $saved.Keys)
    {
        [Environment]::SetEnvironmentVariable($name, $saved[$name])
    }
}
$start = Get-Date

# (seconds from launch, virtual key, scan code)
$schedule = @(@(40, 0x0D, 0x1C), @(43, 0x0D, 0x1C), @(46, 0x5A, 0x2C), @(49, 0x0D, 0x1C), @(52, 0x5A, 0x2C),
              @(57, 0x5A, 0x2C))
foreach ($key in $schedule)
{
    while (((Get-Date) - $start).TotalSeconds -lt $key[0]) { Start-Sleep -Milliseconds 100 }
    $window = [DriveWindow]::Find([uint32]$process.Id)
    if ($window -eq [IntPtr]::Zero)
    {
        Write-Warning ("no game window at {0:N0} s" -f ((Get-Date) - $start).TotalSeconds)
        continue
    }
    # lParam: repeat count 1 and the scan code; key up also sets bits 30-31.
    [void][DriveWindow]::PostMessage($window, 0x100, [IntPtr]$key[1], [IntPtr](1 -bor ($key[2] -shl 16)))
    Start-Sleep -Milliseconds 150
    [void][DriveWindow]::PostMessage($window, 0x101, [IntPtr]$key[1], [IntPtr](0xC0000001 -bor ($key[2] -shl 16)))
}

while (((Get-Date) - $start).TotalSeconds -lt $Seconds -and -not $process.HasExited) { Start-Sleep -Milliseconds 500 }
if (-not $process.HasExited)
{
    Stop-Process -Id $process.Id -Force
}
