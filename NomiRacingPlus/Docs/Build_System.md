# NIO Racing Plus - Build System Documentation

## Overview

The build system provides automated build, signing, packaging, and distribution for both Windows and macOS platforms. It includes:

- **Local build scripts** for development and testing
- **CI/CD pipeline** for automated builds on push/PR
- **Code signing** for both platforms
- **macOS notarization** for App Store / Gatekeeper compliance
- **Installer creation** (NSIS/Inno Setup for Windows, DMG for macOS)
- **Release automation** with GitHub Releases

## Directory Structure

```
NomiRacingPlus/
├── Scripts/
│   ├── build_config.sh          # Shared configuration variables
│   ├── build_macos.sh           # macOS build/sign/package script
│   ├── build_windows.bat        # Windows build/sign/package script
│   ├── entitlements.plist       # macOS entitlements for code signing
│   └── Editor/                  # UE5 Editor utility scripts
├── .github/
│   └── workflows/
│       └── build.yml            # CI/CD pipeline
└── Build/                       # Build output (gitignored)
    ├── Archive/                 # Raw UE5 build output
    ├── Packages/                # Final installers/DMGs
    └── Logs/                    # Build logs
```

## Quick Start

### macOS Local Build

```bash
cd NomiRacingPlus

# Build only (no signing)
./Scripts/build_macos.sh build --skip-sign

# Full release pipeline (requires signing certificates)
./Scripts/build_macos.sh release

# Build and package without notarization
./Scripts/build_macos.sh release --skip-notarize

# Verify existing build signatures
./Scripts/build_macos.sh verify
```

### Windows Local Build

```batch
cd NomiRacingPlus

REM Build only (no signing)
Scripts\build_windows.bat build --skip-sign

REM Full release pipeline
Scripts\build_windows.bat release

REM Create ZIP only (no installer)
Scripts\build_windows.bat package --installer zip

REM Verify signatures
Scripts\build_windows.bat verify
```

## Configuration

### Environment Variables

#### macOS Signing

| Variable | Description | Example |
|----------|-------------|---------|
| `MAC_SIGNING_IDENTITY` | Code signing identity | `"Developer ID Application: Name (TEAM_ID)"` |
| `MAC_TEAM_ID` | Apple Team ID | `"ABCDE12345"` |
| `MAC_NOTARIZE_APPLE_ID` | Apple ID for notarization | `"developer@example.com"` |
| `MAC_NOTARIZE_PASSWORD` | App-specific password | `"xxxx-xxxx-xxxx-xxxx"` |
| `MAC_NOTARIZE_TEAM_ID` | Team ID for notarization | `"ABCDE12345"` |

#### Windows Signing

| Variable | Description | Example |
|----------|-------------|---------|
| `WIN_SIGNING_CERT` | Path to PFX certificate | `"C:\certs\signing.pfx"` |
| `WIN_SIGNING_PASSWORD` | Certificate password | `"password123"` |
| `WIN_SIGNTOOL_PATH` | Path to signtool.exe | Auto-detected from Windows SDK |

#### General

| Variable | Description | Default |
|----------|-------------|---------|
| `UE5_ROOT` | Path to UE5 installation | Auto-detected |
| `LOG_LEVEL` | Logging verbosity | `INFO` |

### Setting Up macOS Code Signing

1. **Obtain a Developer ID Application certificate** from Apple Developer portal
2. **Export as .p12** from Keychain Access
3. **Import to CI** (base64 encode for GitHub Secrets):
   ```bash
   base64 -i certificate.p12 | pbcopy
   ```
4. **Create an app-specific password** at appleid.apple.com
5. **Store notarization credentials**:
   ```bash
   xcrun notarytool store-credentials "notarytool-profile" \
     --apple-id "developer@example.com" \
     --team-id "ABCDE12345" \
     --password "xxxx-xxxx-xxxx-xxxx"
   ```

### Setting Up Windows Code Signing

1. **Obtain a code signing certificate** (e.g., from DigiCert, Sectigo)
2. **Export as .pfx** file
3. **For CI**: base64 encode and store as GitHub Secret:
   ```powershell
   [Convert]::ToBase64String([IO.File]::ReadAllBytes("signing.pfx")) | Set-Clipboard
   ```

## Commands Reference

### macOS Script (`build_macos.sh`)

| Command | Description |
|---------|-------------|
| `build` | Build the UE5 project |
| `sign` | Code sign an existing build |
| `notarize` | Notarize and staple |
| `package` | Create DMG installer |
| `release` | Full pipeline: build -> sign -> notarize -> package |
| `verify` | Verify code signatures |
| `clean` | Remove build artifacts |

#### Options

| Option | Description |
|--------|-------------|
| `--build-type TYPE` | Development, Shipping, or Debug |
| `--ue5 PATH` | UE5 installation path |
| `--skip-sign` | Skip code signing |
| `--skip-notarize` | Skip notarization |
| `--skip-package` | Skip DMG creation |
| `--verbose` | Enable verbose output |

### Windows Script (`build_windows.bat`)

| Command | Description |
|---------|-------------|
| `build` | Build the UE5 project |
| `sign` | Code sign an existing build |
| `package` | Create installer or ZIP |
| `release` | Full pipeline: build -> sign -> package |
| `verify` | Verify code signatures |
| `clean` | Remove build artifacts |

#### Options

| Option | Description |
|--------|-------------|
| `--build-type TYPE` | Development, Shipping, or Debug |
| `--ue5 PATH` | UE5 installation path |
| `--skip-sign` | Skip code signing |
| `--skip-package` | Skip installer creation |
| `--installer TYPE` | nsis, inno, or zip |
| `--verbose` | Enable verbose output |

## CI/CD Pipeline

### GitHub Actions Workflow

The `.github/workflows/build.yml` pipeline runs on:
- **Push** to `main` or `develop` branches
- **Pull requests** to `main`
- **Tags** matching `v*` (triggers release)
- **Manual dispatch** via GitHub UI

### Pipeline Stages

1. **Preflight** - Validate configuration, check for secrets
2. **Windows Build** - Build, sign, package
3. **macOS Build** - Build, sign, notarize, package as DMG
4. **Tests** - Run UE5 automation tests
5. **Code Quality** - Check for TODOs, validate files
6. **Release** - Create GitHub Release with artifacts

### Required GitHub Secrets

| Secret | Description |
|--------|-------------|
| `MAC_CERTIFICATE_BASE64` | macOS certificate (.p12) base64-encoded |
| `MAC_CERTIFICATE_PASSWORD` | Certificate password |
| `MAC_SIGNING_IDENTITY` | Signing identity string |
| `MAC_TEAM_ID` | Apple Team ID |
| `MAC_NOTARIZE_APPLE_ID` | Apple ID |
| `MAC_NOTARIZE_PASSWORD` | App-specific password |
| `WIN_CERTIFICATE_BASE64` | Windows certificate (.pfx) base64-encoded |
| `WIN_CERTIFICATE_PASSWORD` | Certificate password |

### Triggering a Release

```bash
# Tag a release
git tag v1.0.0
git push origin v1.0.0

# Or use workflow_dispatch with create_release=true
```

## macOS Entitlements

The `entitlements.plist` file grants necessary permissions:

| Entitlement | Purpose |
|-------------|---------|
| `allow-jit` | JIT compilation (game engine) |
| `allow-unsigned-executable-memory` | Engine memory management |
| `disable-library-validation` | Plugin loading |
| `network.client` | Online features |
| `network.server` | Multiplayer hosting |
| `audio-input` | Voice chat |
| `usb` | USB controllers |
| `gamecontroller` | Game controllers |

## Installer Details

### Windows NSIS Installer

- Creates Start Menu shortcuts
- Desktop shortcut (optional)
- Add/Remove Programs entry
- Uninstaller
- LZMA2 compression
- Supports English and Simplified Chinese

### Windows Inno Setup Installer

- Alternative to NSIS
- Similar features
- Better Windows 11 integration

### macOS DMG

- Drag-to-install interface
- Applications symlink
- Signed and notarized
- High compression (UDZO/zlib)

## Troubleshooting

### macOS Issues

**"unidentified developer" error:**
- Ensure the app is signed with a Developer ID certificate
- Run `xcrun stapler validate App.app` to check notarization

**Notarization fails:**
- Check Apple Developer portal for certificate validity
- Verify app-specific password is correct
- Review notarization log: `xcrun notarytool log <submission-id>`

**Gatekeeper blocks the app:**
```bash
# Remove quarantine attribute
xattr -d com.quarantine /path/to/App.app
```

### Windows Issues

**signtool not found:**
- Install Windows SDK
- Or set `WIN_SIGNTOOL_PATH` explicitly

**Certificate not trusted:**
- Ensure the certificate chain is complete
- Use `/tr` for timestamp (not `/t`)

**NSIS not found:**
- Install NSIS from nsis.sourceforge.io
- Or use `--installer zip` for ZIP-only packaging

## Build Output

After a successful release build, the `Build/Packages/` directory contains:

```
Build/Packages/
├── NomiRacingPlus_1.0.0_windows.zip          # Windows ZIP
├── NomiRacingPlus_1.0.0_windows.zip.sha256   # Checksum
├── NomiRacingPlus_1.0.0_Setup.exe            # Windows installer (if NSIS/Inno available)
├── NomiRacingPlus_1.0.0_Setup.exe.sha256     # Checksum
├── NomiRacingPlus_1.0.0_macos.dmg            # macOS disk image
└── NomiRacingPlus_1.0.0_macos.dmg.sha256     # Checksum
```
