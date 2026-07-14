param(
    [string]$Version = ""
)

$Repo = "GalaxyHaze/Helios-IDE"
$InstallRoot = Join-Path $env:LOCALAPPDATA "Programs\Helios"
$TempZip = Join-Path $env:TEMP "helios-installer.zip"

function Resolve-LatestVersion {
    try {
        $response = Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases/latest"
        return $response.tag_name
    } catch {
        Write-Error "Failed to fetch the latest Helios release."
        exit 1
    }
}

if ([string]::IsNullOrWhiteSpace($Version)) {
    Write-Host "No version specified. Fetching latest release..."
    $Version = Resolve-LatestVersion
}

$isArm64 = $false
if ($env:PROCESSOR_ARCHITECTURE -match "ARM64" -or $env:PROCESSOR_ARCHITEW6432 -match "ARM64") {
    $isArm64 = $true
}

$AssetName = if ($isArm64) { "helios-windows-arm64.zip" } else { "helios-windows-amd64.zip" }
$DownloadUrl = "https://github.com/$Repo/releases/download/$Version/$AssetName"
$InstallDir = Join-Path $InstallRoot $Version

Write-Host "Downloading $DownloadUrl ..."
Invoke-WebRequest -Uri $DownloadUrl -OutFile $TempZip -UseBasicParsing

if (Test-Path $InstallDir) {
    Remove-Item $InstallDir -Recurse -Force
}
New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null

Expand-Archive -Path $TempZip -DestinationPath $InstallDir -Force
Remove-Item $TempZip -Force

$CurrentUserPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ([string]::IsNullOrWhiteSpace($CurrentUserPath)) {
    $CurrentUserPath = $InstallDir
} elseif (($CurrentUserPath -split ';') -notcontains $InstallDir) {
    $CurrentUserPath = "$CurrentUserPath;$InstallDir"
}
[Environment]::SetEnvironmentVariable("Path", $CurrentUserPath, "User")

$ShimPath = Join-Path $InstallDir "helios.cmd"
@"
@echo off
start "" "%~dp0Helios.exe"
"@ | Set-Content -Path $ShimPath -Encoding ASCII

Write-Host "Helios installed to $InstallDir" -ForegroundColor Green
Write-Host "A user PATH entry was added for this version directory." -ForegroundColor Green
Write-Host "Open a new terminal and run: helios" -ForegroundColor Green
