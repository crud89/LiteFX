#Requires -Version 7.0
<#
.SYNOPSIS
    Writes the LiteFX version to src/cmake/VersionVariables.cmake and the Doxygen configuration.
.EXAMPLE
    ./write-version.ps1 -major 0 -minor 5 -build 1
#>
Param(
  [Parameter(Mandatory)] [ValidateRange(0, [int]::MaxValue)] [int] $major,
  [Parameter(Mandatory)] [ValidateRange(0, [int]::MaxValue)] [int] $minor,
  [ValidateRange(0, [int]::MaxValue)] [int] $build = 1,
  [ValidateRange(2000, 9999)] [int] $year = (Get-Date).ToUniversalTime().Year
)

$ErrorActionPreference = 'Stop'

# Resolve paths relative to the script.
$versionFile = Join-Path $PSScriptRoot 'src' 'cmake' 'VersionVariables.cmake'
$doxyFile    = Join-Path $PSScriptRoot '.doxyfile'

# Update the VersionVariables.cmake file.
$content = @"
SET(LITEFX_YEAR $year)
SET(LITEFX_RELEASE $major)
SET(LITEFX_PATCH $minor)
SET(LITEFX_BUILD $build)

"@

Set-Content -Path $versionFile -Value $content -NoNewline -Encoding utf8NoBOM

# Update the project number in the Doxygen configuration.
$version = "$major.$minor.$build.$year"
$doxygen = Get-Content -Path $doxyFile -Raw
$updated = $doxygen -replace '(?m)^PROJECT_NUMBER[ \t]*=[^\r\n]*', "PROJECT_NUMBER         = $version"

if ($updated -eq $doxygen -and $doxygen -notmatch "(?m)^PROJECT_NUMBER[ \t]*=[ \t]*$([regex]::Escape($version))") {
  throw "PROJECT_NUMBER not found in $doxyFile."
}

Set-Content -Path $doxyFile -Value $updated -NoNewline -Encoding utf8NoBOM

Write-Host -ForegroundColor Green "Updated version to $major.$minor.$build.$year."
