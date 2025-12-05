# Setup Public DMATool Repository
# Creates a clean public repo with protected exe and documentation

Write-Host "`n=== DMATool Public Repository Setup ===" -ForegroundColor Cyan

# Configuration
$publicRepoName = "DMATool"
$githubUsername = "suni2k"  # Your GitHub username
$publicRepoPath = "..\DMATool-Public"

Write-Host "Creating public repository structure...`n" -ForegroundColor Yellow

# Create public repo directory
if (Test-Path $publicRepoPath) {
    Write-Host "Removing existing public repo folder..." -ForegroundColor Yellow
    Remove-Item $publicRepoPath -Recurse -Force
}

New-Item -ItemType Directory -Path $publicRepoPath -Force | Out-Null
Set-Location $publicRepoPath

# Initialize Git repository
Write-Host "Initializing Git repository..." -ForegroundColor Cyan
git init
git branch -M main

# Create directory structure
Write-Host "Creating directory structure..." -ForegroundColor Cyan
New-Item -ItemType Directory -Path "assets" -Force | Out-Null
New-Item -ItemType Directory -Path "releases" -Force | Out-Null
New-Item -ItemType Directory -Path ".github" -Force | Out-Null

# Copy README
Write-Host "Copying README..." -ForegroundColor Cyan
Copy-Item "..\DMATool\PUBLIC_README.md" ".\README.md" -Force

# Copy assets (screenshots)
Write-Host "Copying assets..." -ForegroundColor Cyan
$assetsSource = "..\DMATool\assets"
if (Test-Path $assetsSource) {
    Copy-Item "$assetsSource\*" ".\assets\" -Recurse -Force
    Write-Host "? Copied screenshots and assets" -ForegroundColor Green
} else {
    Write-Host "? Assets folder not found: $assetsSource" -ForegroundColor Yellow
    Write-Host "  You'll need to add screenshots manually" -ForegroundColor Gray
}

# Copy protected exe
Write-Host "Copying protected executable..." -ForegroundColor Cyan
$protectedExe = "..\DMATool\bin\Release-x64\DMATool.exe"
if (Test-Path $protectedExe) {
    Copy-Item $protectedExe ".\releases\DMATool.exe" -Force
    $size = [math]::Round((Get-Item ".\releases\DMATool.exe").Length / 1MB, 2)
    Write-Host "? Copied DMATool.exe ($size MB)" -ForegroundColor Green
} else {
    Write-Host "? Protected exe not found: $protectedExe" -ForegroundColor Red
    Write-Host "  Make sure you renamed DMATool.vmp.exe to DMATool.exe" -ForegroundColor Yellow
}

# Create LICENSE
Write-Host "Creating LICENSE..." -ForegroundColor Cyan
$license = @"
Copyright (c) 2025 suni2k

PROPRIETARY LICENSE

All rights reserved.

Permission is hereby granted to download and use this software (DMATool) for personal
and commercial purposes, subject to the following conditions:

1. The software is provided "AS IS" without warranty of any kind.

2. Redistribution of the software, in source or binary form, is prohibited without
   explicit written permission from the copyright holder.

3. Reverse engineering, decompilation, or disassembly of the software is strictly
   prohibited.

4. Modification of the software is prohibited.

5. The copyright notice and this permission notice shall be included in all copies
   or substantial portions of the software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For licensing inquiries, please contact via GitHub Issues.
"@

$license | Out-File -FilePath ".\LICENSE" -Encoding UTF8
Write-Host "? Created LICENSE" -ForegroundColor Green

# Create .gitignore
Write-Host "Creating .gitignore..." -ForegroundColor Cyan
$gitignore = @"
# Visual Studio
*.user
*.suo
*.sln.docstates
.vs/

# Build results
[Dd]ebug/
[Rr]elease/
x64/
[Bb]in/
[Oo]bj/

# Temp files
*.tmp
*.temp
"@

$gitignore | Out-File -FilePath ".\.gitignore" -Encoding UTF8
Write-Host "? Created .gitignore" -ForegroundColor Green

# Create GitHub issue templates
Write-Host "Creating GitHub templates..." -ForegroundColor Cyan
New-Item -ItemType Directory -Path ".github\ISSUE_TEMPLATE" -Force | Out-Null

$bugTemplate = @"
---
name: Bug Report
about: Report a bug or issue
title: '[BUG] '
labels: bug
assignees: ''
---

**Describe the bug**
A clear description of what the bug is.

**To Reproduce**
Steps to reproduce:
1. Go to '...'
2. Click on '...'
3. See error

**Expected behavior**
What you expected to happen.

**Screenshots**
If applicable, add screenshots.

**System Information:**
 - Windows Version: [e.g., Windows 11 22H2]
 - DMATool Version: [e.g., v1.0.0]
 - DMA Card Model: [e.g., Squirrel]
 - JTAG Adapter: [e.g., CH347]

**Additional context**
Any other information about the problem.
"@

$bugTemplate | Out-File -FilePath ".github\ISSUE_TEMPLATE\bug_report.md" -Encoding UTF8

$featureTemplate = @"
---
name: Feature Request
about: Suggest an idea
title: '[FEATURE] '
labels: enhancement
assignees: ''
---

**Is your feature request related to a problem?**
A clear description of the problem.

**Describe the solution you'd like**
What you want to happen.

**Describe alternatives you've considered**
Alternative solutions or features.

**Additional context**
Any other context or screenshots.
"@

$featureTemplate | Out-File -FilePath ".github\ISSUE_TEMPLATE\feature_request.md" -Encoding UTF8
Write-Host "? Created issue templates" -ForegroundColor Green

# Create initial commit
Write-Host "`nCreating initial commit..." -ForegroundColor Cyan
git add .
git commit -m "Initial commit: DMATool v1.0.0

- Professional DMA card management suite
- JTAG/Flash/Benchmark functionality
- Protected with VMProtect Ultimate
- Includes screenshots and documentation"

Write-Host "? Initial commit created" -ForegroundColor Green

# Summary
Write-Host "`n=== Repository Setup Complete! ===" -ForegroundColor Green

Write-Host "`n?? Repository Location:" -ForegroundColor Cyan
Write-Host "   $(Resolve-Path $publicRepoPath)" -ForegroundColor White

Write-Host "`n?? Repository Contents:" -ForegroundColor Cyan
Write-Host "   ? README.md - Professional documentation" -ForegroundColor Green
Write-Host "   ? LICENSE - Proprietary license" -ForegroundColor Green
Write-Host "   ? releases/DMATool.exe - Protected executable" -ForegroundColor Green
Write-Host "   ? assets/ - Screenshots" -ForegroundColor Green
Write-Host "   ? .github/ - Issue templates" -ForegroundColor Green

Write-Host "`n?? Next Steps:" -ForegroundColor Cyan
Write-Host "`n1. Create GitHub repository:" -ForegroundColor Yellow
Write-Host "   a. Go to: https://github.com/new" -ForegroundColor White
Write-Host "   b. Repository name: $publicRepoName" -ForegroundColor White
Write-Host "   c. Description: Professional DMA Card Management Suite" -ForegroundColor White
Write-Host "   d. Visibility: Public" -ForegroundColor White
Write-Host "   e. Do NOT initialize with README (we have one)" -ForegroundColor White
Write-Host "   f. Click 'Create repository'" -ForegroundColor White

Write-Host "`n2. Push to GitHub:" -ForegroundColor Yellow
Write-Host "   cd $publicRepoPath" -ForegroundColor Gray
Write-Host "   git remote add origin https://github.com/$githubUsername/$publicRepoName.git" -ForegroundColor Gray
Write-Host "   git push -u origin main" -ForegroundColor Gray

Write-Host "`n3. Create Release:" -ForegroundColor Yellow
Write-Host "   a. Go to: https://github.com/$githubUsername/$publicRepoName/releases/new" -ForegroundColor White
Write-Host "   b. Tag: v1.0.0" -ForegroundColor White
Write-Host "   c. Title: DMATool v1.0.0 - Initial Release" -ForegroundColor White
Write-Host "   d. Upload: releases/DMATool.exe" -ForegroundColor White
Write-Host "   e. Click 'Publish release'" -ForegroundColor White

Write-Host "`n4. Verify:" -ForegroundColor Yellow
Write-Host "   Visit: https://github.com/$githubUsername/$publicRepoName" -ForegroundColor White

Write-Host "`n?? Optional Improvements:" -ForegroundColor Cyan
Write-Host "   - Add more screenshots to assets/" -ForegroundColor Gray
Write-Host "   - Create a banner image for README" -ForegroundColor Gray
Write-Host "   - Add CHANGELOG.md for version history" -ForegroundColor Gray
Write-Host "   - Set up GitHub Pages for documentation" -ForegroundColor Gray

Write-Host "`n? Your public repository is ready!" -ForegroundColor Green
