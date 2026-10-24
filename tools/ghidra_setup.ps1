# Create (or recreate) the Ghidra project ghidra_project\WandOfGamelon with cdi_zelda.mod imported, named and
# analysed. Run from the project root:  pwsh tools\ghidra_setup.ps1
# The project contains the game's code: never publish it.
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$ghidra = Join-Path $root "tools\ghidra\ghidra_12.1.4_PUBLIC"
$env:JAVA_HOME = "C:\Program Files\Microsoft\jdk-21.0.11.10-hotspot"
$proj = Join-Path $root "ghidra_project"
New-Item -ItemType Directory -Force $proj | Out-Null
& (Join-Path $ghidra "support\analyzeHeadless.bat") $proj WandOfGamelon `
    -import (Join-Path $root "extract\modules\cdi_zelda.mod") -overwrite `
    -loader BinaryLoader -processor "68000:BE:32:default" `
    -scriptPath (Join-Path $root "tools\ghidra_scripts") -preScript WoGSetup.java $root
