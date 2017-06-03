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

.PARAMETER Generator
  Selects the supported Cmake generator. Defaults to MSVC 2015.

.PARAMETER ProjectOnly
  Enable this switch to only generate the vs project files and not run a build.

.EXAMPLE
  Build x86 debug binaries

  .\build x86 Debug

.EXAMPLE
  Build release binaries for all platforms

  .\build All
  
.EXAMPLE 
  Generate visual studio project files without running a build
  
  .\build All -ProjectOnly
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
    $Configuration = 'Release',
    
    [Parameter(Mandatory=$false, Position=3)]
    [ValidateSet('Visual Studio 14 2015','Visual Studio 15 2017')]
    [String]
    $Generator = 'Visual Studio 14 2015',
    
    [Parameter(Mandatory=$false)]
    [Switch]
    $ProjectOnly
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
  $treeDirectory = "CMakeBuild/$_"

  if ($_ -eq "x64") {
    $postfix = " Win64"
  }

  # Generate project
  if (!(Test-Path $treeDirectory)) {
    new-item $treeDirectory -ItemType Directory | Out-Null
  }

  # Generate visual studio project files
  cmake -E chdir $treeDirectory cmake -G "$Generator$postfix" ../../
  if ($LASTEXITCODE -ne 0) { throw "cmake failed" }

  # Build
  if (!$ProjectOnly) {
    cmake --build "$treeDirectory" --config "$Configuration"
    if ($LASTEXITCODE -ne 0) { throw "build failed" }

    cmake -E chdir $treeDirectory ctest --build-config "$Configuration"
    if ($LASTEXITCODE -ne 0) { throw "build failed" }
  }
}
