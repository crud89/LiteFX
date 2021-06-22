Param(
  [int] $major,
  [int] $minor,
  [int] $build = 1,
  [int] $year = (Get-Date).year
)

# Update the VersionVariables.cmake file.
$output = @'
SET(LITEFX_YEAR {0})
SET(LITEFX_RELEASE {1})
SET(LITEFX_PATCH {2})
SET(LITEFX_BUILD {3})
'@ -f $year, $major, $minor, $build

$output | Out-File -Encoding "UTF8" -FilePath .\src\cmake\VersionVariables.cmake

# Load the doxyfile.
$output = ""

Get-Content .\.doxyfile | ForEach-Object {
    if($_.StartsWith("PROJECT_NUMBER")) {
        $output += "PROJECT_NUMBER         = {0}.{1}.{2}.{3}`n" -f $major, $minor, $build, $year
    } else {
        $output += "{0}`n" -f $_
    }
}

$output | Out-File -Encoding "UTF8" -FilePath .\.doxyfile

$output = "Updated Version to {0}.{1}.{2}.{3}" -f $major, $minor, $build, $year
Write-Host -ForegroundColor Green $output