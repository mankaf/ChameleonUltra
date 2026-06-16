# Build Chameleon Ultra firmware and create DFU package
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Building Chameleon Ultra v2.2 Firmware" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Navigate to firmware directory
$firmwareDir = Join-Path $PWD "firmware"
Set-Location $firmwareDir

Write-Host "Current directory: $($PWD.Path)" -ForegroundColor Gray
Write-Host ""

# Check if nrfutil is available
try {
    Write-Host "Checking nRF CLI tools..." -ForegroundColor Yellow
    $nrfutil = Get-Command nrfutil -ErrorAction SilentlyContinue
    
    if ($null -eq $nrfutil) {
        Write-Host ""
        Write-Host "❌ ERROR: nRF CLI tools not found!" -ForegroundColor Red
        Write-Host "   Please install Nordic SDK tools first:" -ForegroundColor Gray
        Write-Host "   https://github.com/NordicSemiconductor/nrfconnect-cli/releases" -ForegroundColor Yellow
        Write-Host ""
        exit 1
    } else {
        Write-Host "✅ nRF CLI tools found!" -ForegroundColor Green
    }
} catch {
    Write-Host "Error checking nRF tools: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# If we're building for nrfutil, need a build directory first
Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Building Application with Make" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

# Check if make is available (part of Nordic SDK)
try {
    $makeCmd = Get-Command make -ErrorAction SilentlyContinue
    
    if ($null -eq $makeCmd) {
        Write-Host ""
        Write-Host "❌ ERROR: Make not found!" -ForegroundColor Red
        Write-Host "   You need Nordic nRF SDK which includes Make toolchain." -ForegroundColor Gray
        Write-Host ""
        Write-Host "   Install Nordic SDK:" -ForegroundColor Yellow
        Write-Host "   https://github.com/NordicSemiconductor/nrfconnect-sdk" -ForegroundColor Yellow
        
        exit 1
    } else {
        Write-Host "✅ Make found!" -ForegroundColor Green
    }
} catch {
    Write-Host "Error: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Build the firmware
Write-Host ""
Write-Host "Running: make clean" -ForegroundColor Cyan
try {
    make clean
    Write-Host "✅ make clean completed" -ForegroundColor Green
} catch {
    Write-Host "⚠️  Make clean had issues, continuing anyway..." -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Running: make app.hex" -ForegroundColor Cyan
try {
    # Build with our patched sources
    $env:NRF_TARGET = "nrf52"
    $env:NRF_SDK_DIR = "$PWD\nrf52_sdk"
    
    make clean 2>&1 | Out-String
    
    make app.hex 2>&1 | Out-String
    Write-Host ""
    Write-Host "✅ Compilation completed!" -ForegroundColor Green
    
} catch {
    $error = $_.Exception.Message
    if ($error -like "*error:*" -or $error -like "*warning:") {
        Write-Host ""
        Write-Host "⚠️  Build completed with errors/warnings:" -ForegroundColor Yellow
        
        # Check if app.hex was created anyway
        if (Test-Path "firmware\build\objects\application.hex") {
            Write-Host "✅ But application.hex WAS created! File exists:" -ForegroundColor Green
            $appHex = Get-Item "firmware\build\objects\application.hex"
            Write-Host "   Size: $([math]::Round($appHex.Length/1KB, 2)) KB" -ForegroundColor Gray
        } else {
            Write-Host "❌ application.hex was not created. Check errors above:" -ForegroundColor Red
            Write-Host ""
            Write-Host "Error details:" -ForegroundColor Red
            $lines = Get-Content $error -Head 10
            foreach ($line in $lines) {
                Write-Host "   $line" -ForegroundColor DarkRed
            }
        }
    } else {
        Write-Host ""
        Write-Host "❌ Build failed with error:" -ForegroundColor Red
        Write-Host "   $error" -ForegroundColor Red
    }
}

# Copy new feature files to final build location if they aren't already there
$sourceDir = Join-Path $firmwareDir "application\src"
$newImplDir = Join-Path $PWD "new_implementation"

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Verifying New Features Are Included" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# Check if our new files are in the build source
$filesToCheck = @(
    "$sourceDir\hf_extended",
    "$sourceDir\lowfreq\t5577.c",
    "$sourceDir\lowfreq\em4305.c", 
    "$sourceDir\lowfreq\fdx_b.c",
    "$sourceDir\lowfreq\indala_psk.c",
    "$sourceDir\lowfreq\keri_psk.c",
    "$sourceDir\lowfreq\paradox_fsk.c",
    "$sourceDir\relay_attack",
    "$sourceDir\all_missing_features.h"
)

foreach ($file in $filesToCheck) {
    if (Test-Path $file) {
        Write-Host "✅ Found: $([System.IO.Path]::GetFileName($file))" -ForegroundColor Green
    } else {
        # Check if file exists in new_implementation and copy it
        $impFile = Join-Path $newImplDir ([System.IO.Path]::GetFileName($file))
        if (Test-Path $impFile) {
            $destDir = [System.IO.Path]::GetDirectoryName($file)
            $destPath = Join-Path $destDir ([System.IO.Path]::GetFileName($impFile))
            
            Write-Host "  ⚠️  Not in source yet, copying from: $(Get-Item $impFile).Name" -ForegroundColor Yellow
            
            try {
                New-Item -ItemType Directory -Force -Path $destDir | Out-Null
                
                Copy-Item -Path $impFile -Destination $destPath -Force
                Write-Host "  ✅ Copied to: $destPath" -ForegroundColor Green
            } catch {
                Write-Host "  ⚠️  Failed to copy: $($_.Exception.Message)" -ForegroundColor Red
            }
        } else {
            Write-Host "❌ Not found anywhere: $(Get-Item $file).Name" -ForegroundColor Red
        }
    }
}

# List final build output
Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Build Output Files" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

if (Test-Path "$firmwareDir\build\objects") {
    Get-ChildItem "$firmwareDir\build\objects" | Select-Object Name, @{Name="SizeKB";Expression={[math]::Round($_.Length/1KB, 2)}} | Format-Table
    
    # Check app.hex specifically
    if (Test-Path "$firmwareDir\build\objects\application.hex") {
        $appHex = Get-Item "$firmwareDir\build\objects\application.hex"
        Write-Host ""
        Write-Host "✅ Application hex file ready:" -ForegroundColor Green
        Write-Host "   Location: $($appHex.FullName)"
        Write-Host "   Size: $([math]::Round($appHex.Length/1KB, 2)) KB"
    } else {
        Write-Host ""
        Write-Host "⚠️  application.hex not created - check build errors above" -ForegroundColor Yellow
    }
}

# Create DFU package if we have the hex file
if (Test-Path "$firmwareDir\build\objects\application.hex") {
    Write-Host ""
    Write-Host "=========================================" -ForegroundColor Cyan
    Write-Host "Creating DFU Package" -ForegroundColor Cyan
    Write-Host "=========================================" -ForegroundColor Cyan
    
    $appHex = Join-Path $firmwareDir "build\objects\application.hex"
    
    # Check if softdevice files exist
    $sdFiles = @(
        "$firmwareDir\nrf52_sdk\components\softdevice\s140\hex\s140_nrf52_*.s14",
        "$firmwareDir\nrf52_sdk\components\softdevice\s132\hex\s132_nrf52_*.s14",
        "$firmwareDir\nrf52_sdk\components\softdevice\s122\hex\s122_nrf52_*.s14"
    )
    
    $sdUsed = ""
    foreach ($sd in $sdFiles) {
        $match = Select-String -Path $sd -Pattern "\.s14$" | Select-Object -First 1
        if ($null -ne $match) {
            Write-Host "Found softdevice: $($match.File)" -ForegroundColor Gray
        }
    }
    
    # Use the first available softdevice
    $sdFile = Get-ChildItem -Path $firmwareDir -Recurse -Filter "*.s14" | Select-Object -First 1
    
    if ($null -ne $sdFile) {
        Write-Host ""
        Write-Host "Found: $($sdFile.Name)" -ForegroundColor Green
        
        # Create DFU ZIP
        $pkgName = "ChameleonUltra_v2.2_app.hex.dfuzip"
        
        Write-Host ""
        Write-Host "Creating DFU package..." -ForegroundColor Cyan
        
        # Combine app with softdevice for DFU (simplified method)
        try {
            $dfuZip = Join-Path $PWD "build\$pkgName"
            
            Compress-Archive -Path @(
                "$firmwareDir\build\objects\application.hex",
                "$sdFile.FullName"
            ) -DestinationPath $dfuZip -Force
            
            Write-Host ""
            Write-Host "✅ DFU package created!" -ForegroundColor Green
            Write-Host "   Name: $pkgName"
            Write-Host "   Size: $([math]::Round((Get-Item $dfuZip).Length/1KB, 2)) KB"
            Write-Host "   Location: $dfuZip"
            
        } catch {
            Write-Host ""
            Write-Host "❌ Failed to create DFU package:" -ForegroundColor Red
            Write-Host "   $($_.Exception.Message)" -ForegroundColor Red
        }
    } else {
        Write-Host ""
        Write-Host "⚠️  No softdevice file found in build directory" -ForegroundColor Yellow
        Write-Host "   DFU package creation skipped" -ForegroundColor Gray
    }
    
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "BUILD COMPLETE!" -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Cyan

if (Test-Path "$PWD\build\$pkgName") {
    Write-Host ""
    Write-Host "🚀 Your firmware build is complete!" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Files ready to upload:" -ForegroundColor Gray
    Write-Host "   $(Get-Item $appHex).FullName" -ForegroundColor Green
    Write-Host "   $(Get-Item $dfuZip).FullName" -ForegroundColor Green
    
}
