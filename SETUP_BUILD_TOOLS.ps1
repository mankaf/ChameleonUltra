# Chameleon Ultra v2.2 Build Setup Check Script
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "Chameleon Ultra v2.2 - Build Environment Setup" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""

Write-Host "Checking Prerequisites..." -ForegroundColor Yellow
Write-Host ""

# Check Git
try { $git = Get-Command git -ErrorAction SilentlyContinue } catch {}
if ($null -eq $git) {
    Write-Host "  [!] Git not found. Install from: https://git-scm.com/download/win" -ForegroundColor Red
} else {
    Write-Host "  ✅ Git: installed" -ForegroundColor Green
}

# Check Python3
try { $python = Get-Command python -ErrorAction SilentlyContinue } catch {}
if ($null -eq $python) {
    Write-Host "  [!] Python not found. Install from: https://www.python.org/downloads/windows/" -ForegroundColor Red
} else {
    Write-Host "  ✅ Python3: $($python.Version)" -ForegroundColor Green
}

# Check nrfutil (Nordic CLI)
try { $nrfutil = Get-Command nrfutil -ErrorAction SilentlyContinue } catch {}
if ($null -eq $nrfutil) {
    Write-Host "  [!] nRF CLI tools not found - REQUIRED for building!" -ForegroundColor Red
    Write-Host "      Install from: https://github.com/NordicSemiconductor/nrfconnect-cli/releases" -ForegroundColor Yellow
} else {
    Write-Host "  ✅ nRF CLI: installed" -ForegroundColor Green
}

# Check make/make.exe
try { $make = Get-Command make -ErrorAction SilentlyContinue } catch {}
if ($null -eq $make) {
    Write-Host "  [!] Make not found (will install via Nordic SDK)" -ForegroundColor Gray
} else {
    Write-Host "  ✅ Make: installed" -ForegroundColor Green
}

Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "INSTALLATION ORDER:" -ForegroundColor Yellow
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "1. Git (if missing):" -ForegroundColor Gray
Write-Host "   Download: https://git-scm.com/download/win" -ForegroundColor Gray
Write-Host ""
Write-Host "2. Python 3.x (if missing):" -ForegroundColor Gray  
Write-Host "   Download: https://www.python.org/downloads/windows/" -ForegroundColor Gray
Write-Host ""
Write-Host "3. Nordic nRF CLI Tools (REQUIRED):" -ForegroundColor Gray
Write-Host "   Option A: From GitHub (recommended)" -ForegroundColor Gray
Write-Host "     Download: https://github.com/NordicSemiconductor/nrfconnect-cli/releases/tag/2024.03" -ForegroundColor Gray
Write-Host "     Extract and add to PATH or run PowerShell from the extracted folder" -ForegroundColor Gray
Write-Host ""
Write-Host "   Option B: Nordic Command Line Tools (full SDK)" -ForegroundColor Gray
Write-Host "     Download: https://www.nordicsemi.com/products/nrf-connect-for-desktop-and-mobile-sdk" -ForegroundColor Gray
Write-Host ""
Write-Host "=========================================" -ForegroundColor Cyan
