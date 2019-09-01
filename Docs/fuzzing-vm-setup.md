*Note*: unless specified all commands should be run in an Admin Powershell prompt.

## Stop and disable useless services

```powershell
# disable some features
Set-MpPreference -DisableRealtimeMonitoring $true

# disable the services
$services = @("WinDefend", "WSearch", "WerSvc", "wuauserv", "TrustedInstaller", "TroubleShootingSvc")
$services+= @("DiagTrack", "DiagSvc", "diagnosticshub.standardcollector.service")

foreach($service in $services)
{
	Stop-Service -Name $service
	Set-Service -StartupType Disabled -Name $service
}
```

## Define the crash behavior

```powershell
$crash = Get-WmiObject Win32_OSRecoveryConfiguration -EnableAllPrivileges
$crash | Set-WmiInstance -Arguments @{ AutoReboot=$False }
# 0 = None
# 1 = Complete memory dump
# 2 = Kernel memory dump
# 3 = Small memory dump
$crash | Set-WmiInstance -Arguments @{ DebugInfoType=1  }
$crash | Set-WmiInstance -Arguments @{ OverwriteExistingDebugFile=$False }
New-Item -ItemType Directory c:\dumps
$crash | Set-WmiInstance -Arguments @{ DebugFilePath="c:\dumps" }
$crash | Set-WmiInstance -Arguments @{ WriteToSystemLog=$False }
```


## Make sure the target time is synchro

```powershell
Stop-Service -Name W32Time
Push-Location
Set-Location HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\DateTime\Servers
Set-ItemProperty . 0 "10.0.0.1"
Set-ItemProperty . "(Default)" "0"
Set-Location HKLM:\SYSTEM\CurrentControlSet\services\W32Time\Parameters
Set-ItemProperty . NtpServer "10.0.0.1"
Pop-Location
Start-Service -Name W32Time
```

## Blackhole some MS domains

```powershell
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/hugsy/modern.ie-vagrant/master/scripts/DisableWin10Telemetry.ps1" -OutFile "c:\temp\DisableWin10Telemetry.ps1"
Set-ExecutionPolicy Bypass
&"c:\temp\DisableWin10Telemetry.ps1"
Remove-Item "c:\temp\DisableWin10Telemetry.ps1"
```
