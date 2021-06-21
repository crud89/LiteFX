Param(
  [int] $major,
  [int] $minor,
  [int] $build = 1,
  [int] $year = (Get-Date).year
)

$output = @'
SET(LITEFX_YEAR {0})
SET(LITEFX_RELEASE {1})
SET(LITEFX_PATCH {2})
SET(LITEFX_BUILD {3})
'@ -f $year, $major, $minor, $build

$output | Out-File -FilePath .\VERSION