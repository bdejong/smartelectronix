<#
.SYNOPSIS
  Automates windows builds

.DESCRIPTION
  Provides a way to build for windows using a simple one-liner. It defaults to
  x64 release builds but can be used to build any configuration.

.PARAMETER Platform
  Selects the target platform, x86, x64 or both will the 'All' option.

.PARAMETER Configuration
  Selects the target configuration. Either release or debug.

.EXAMPLE
  Build an x86 debug binary

  .\build x86 Debug

.EXAMPLE
  Build release binaries for all platforms

  .\build All
#>
Param
  (
    [Parameter(Mandatory=$false, Position=1)]
    [ValidateSet('x86','x64','All')]
    [String]
    $Platform = 'x64',

    [Parameter(Mandatory=$false, Position=2)]
    [ValidateSet('Release','Debug')]
    [String]
    $Configuration = 'Release'
  )

# Make sure cmake is found
Get-Command cmake | Out-Null

if ($Platform -eq "All") {
  $targets = @('x86', 'x64')
}
else {
  $targets = @($Platform)
}

$targets | ForEach-Object {
  # Setup
  $treeDirectory = "build$_"

  $generator = "Visual Studio 14 2015"
  if ($_ -eq "x64") {
    $generator += " Win64"
  }

  # Generate project
  if (!(Test-Path $treeDirectory)) {
    new-item $treeDirectory -ItemType Directory | Out-Null
  }

  # Generate visual studio project files
  cmake -E chdir $treeDirectory cmake -DPLUGIN_ARCH="$_" -G "$generator" ../
  if ($LASTEXITCODE -ne 0) { throw "cmake failed" }

  # Build
  cmake --build "$treeDirectory" --config "$Configuration"
  if ($LASTEXITCODE -ne 0) { throw "build failed" }

  cmake -E chdir $treeDirectory ctest --verbose --build-config "$Configuration"
  if ($LASTEXITCODE -ne 0) { throw "build failed" }
}
