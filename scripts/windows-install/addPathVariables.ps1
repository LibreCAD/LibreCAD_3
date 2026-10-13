if ([string]::IsNullOrWhiteSpace($env:QT_ROOT_DIR)) {
    throw 'QT_ROOT_DIR must point to an installed Qt kit.'
}

$qtBin = Join-Path $env:QT_ROOT_DIR 'bin'
[Environment]::SetEnvironmentVariable('Path', "$qtBin;$env:Path", 'User')
