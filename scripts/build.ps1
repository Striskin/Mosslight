param([switch]$Test,[switch]$Smoke,[switch]$Run,[string]$Compiler='g++')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
Set-Location -LiteralPath $root
$memory = Join-Path $root 'Whats_This.txt'
function Record-Build([string]$result) {
    $text = Get-Content -LiteralPath $memory -Raw
    $text = [regex]::Replace($text,'(?s)\r?\nLAST BUILD\r?\n.*$','')
    $text += "`nLAST BUILD`n$(Get-Date -Format 'yyyy-MM-dd HH:mm:ss'): $result`n"
    [IO.File]::WriteAllText($memory,$text,[Text.UTF8Encoding]::new($false))
}
try {
    if (!(Get-Command $Compiler -ErrorAction SilentlyContinue)) { throw 'g++ was not found. Install MSYS2 UCRT64 GCC and add its bin folder to PATH. See README.md.' }
    New-Item -ItemType Directory -Force -Path build,third_party | Out-Null
    $ray = Join-Path $root 'third_party/raylib-5.5_win64_mingw-w64'
    if (!(Test-Path "$ray/include/raylib.h")) {
        $archive = Join-Path $root 'third_party/raylib-5.5.zip'
        & curl.exe -L --fail --retry 2 'https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip' -o $archive
        if ($LASTEXITCODE -ne 0) { throw 'raylib download failed.' }
        $expected = 'FE8FEDA1B92FC02826E2D19C631D648FD0563B93B144893FE18140E9FE4FB6FD'
        if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ne $expected) { throw 'raylib archive checksum mismatch.' }
        Expand-Archive -LiteralPath $archive -DestinationPath third_party -Force
    }
    $core = @('collision','combat','dialogue','enemies','game','inventory','maps','merchant','npcs','player','procedural','save','targeting') | ForEach-Object { "src/$_.cpp" }
    $flags = @('-std=c++17','-O2','-Wall','-Wextra','-Wpedantic','-Isrc','-static','-static-libgcc','-static-libstdc++')
    & $Compiler @flags "-I$ray/include" @core src/main.cpp src/render.cpp src/ui.cpp src/audio.cpp "$ray/lib/libraylib.a" -lopengl32 -lgdi32 -lwinmm -o build/Mosslight.exe
    if ($LASTEXITCODE -ne 0) { throw 'Game compilation failed.' }
    Copy-Item -LiteralPath data -Destination build -Recurse -Force
    Copy-Item -LiteralPath "$ray/LICENSE" -Destination build/RAYLIB-LICENSE.txt -Force
    Copy-Item -LiteralPath README.md -Destination build/README.md -Force
    Record-Build 'Game compiled successfully. Next: run simulation tests and the graphical smoke test.'
    if ($Test) {
        & $Compiler @flags @core tests/tests.cpp -o build/MosslightTests.exe
        if ($LASTEXITCODE -ne 0) { throw 'Test compilation failed.' }
        & .\build\MosslightTests.exe "$root/data" "$root/build/test-output"
        if ($LASTEXITCODE -ne 0) { throw 'Simulation tests failed.' }
        Record-Build 'Game and simulation tests compiled; simulation tests passed. Next: graphical smoke test and human playtesting.'
    }
    if ($Smoke) {
        & .\build\Mosslight.exe --smoke-test --captures "$root/build/captures" --mute
        if ($LASTEXITCODE -ne 0) { throw 'Graphical smoke test failed.' }
        $checks = if ($Test) { 'Simulation tests and graphical smoke test passed.' } else { 'Graphical smoke test passed; simulation tests were not requested in this invocation.' }
        Record-Build "Game compiled. $checks Next: inspect captures and perform human balance/playtesting."
    }
    Write-Host 'Ready: build\Mosslight.exe'
    if ($Run) { Start-Process -FilePath "$root/build/Mosslight.exe" -WorkingDirectory "$root/build" }
} catch {
    Record-Build "FAILED: $($_.Exception.Message) Next: fix the reported failure and rerun scripts/build.ps1 -Test -Smoke."
    throw
}
