# Gera installer\output\Instalar-Sonic3-Coop.exe a partir do build atual.
# Antes: compile o jogo (Release x64) e abra-o uma vez depois da última mudança nos scripts,
# para que Oxygen\sonic3air\saves\scripts.bin esteja atualizado.

$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
$game = Join-Path $root "Oxygen\sonic3air"
$dist = Join-Path $root "dist\Sonic3Coop"
$iscc = Join-Path $env:LOCALAPPDATA "Programs\Inno Setup 6\ISCC.exe"
if (-not (Test-Path $iscc)) { $iscc = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" }

# Scripts compilados precisam ser mais novos que todos os .lemon
$compiled = Get-Item (Join-Path $game "saves\scripts.bin")
$newest = Get-ChildItem (Join-Path $game "scripts") -Recurse -Filter *.lemon | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if ($newest.LastWriteTime -gt $compiled.LastWriteTime) {
	throw "saves\scripts.bin está desatualizado ($($newest.Name) é mais novo). Abra o jogo uma vez e tente de novo."
}

# Empacota os dados (o scripts.bin entra no gamedata.bin e é removido de data\ logo depois)
Copy-Item $compiled.FullName (Join-Path $game "data\scripts.bin") -Force
try {
	Start-Process (Join-Path $game "bin\Release_x64\Sonic3AIR.exe") -ArgumentList "-pack" -WorkingDirectory (Join-Path $game "bin\Release_x64") -Wait
}
finally {
	Remove-Item (Join-Path $game "data\scripts.bin")
}

# Monta a pasta de distribuição (sem ROM, sem áudio remasterizado)
if (Test-Path $dist) { Remove-Item $dist -Recurse -Force }
New-Item -ItemType Directory -Force (Join-Path $dist "data"), (Join-Path $dist "doc") | Out-Null
Copy-Item (Join-Path $game "bin\Release_x64\Sonic3AIR.exe"), (Join-Path $game "bin\Release_x64\discord_game_sdk.dll") $dist
Copy-Item (Join-Path $game "enginedata.bin"), (Join-Path $game "gamedata.bin"), (Join-Path $game "audiodata.bin"), (Join-Path $game "data\metadata.json") (Join-Path $dist "data")
Copy-Item (Join-Path $game "_master_image_template\config.json") $dist
Copy-Item (Join-Path $game "_master_image_template\doc\license.txt") (Join-Path $dist "doc")
Copy-Item (Join-Path $game "_master_image_template\doc\license-thirdparty") (Join-Path $dist "doc") -Recurse

# Compila o instalador
Push-Location $PSScriptRoot
try { & $iscc "sonic3coop.iss" } finally { Pop-Location }

# Copia versionada no repositório, ao lado deste script
Copy-Item (Join-Path $PSScriptRoot "output\Instalar-Sonic3-Coop.exe") $PSScriptRoot -Force
Write-Host "Instalador: $(Join-Path $PSScriptRoot 'Instalar-Sonic3-Coop.exe')"
