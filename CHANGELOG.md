# Changelog

All notable changes to neufox's 2FA will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-12

### Added
- Initial release of neufox's 2FA
- TOTP (Time-based One-Time Password) authentication
- 16-character base32 secrets (80 bits of entropy)
- 6-digit TOTP codes with 30-second validity
- Rate limiting (3 failed attempts with 60-second cooldown)
- Support for Windows (x86/x64) and Linux platforms
- Architecture-specific DLL naming (`neufox-2fa-x64.dll`, `neufox-2fa-x86.dll`)
- Example gamemodes (command-based and dialog-based)
- Complete API with 8 native functions
- `OnPlayerTOTPVerify` callback for verification events
- Full debug symbols (`.pdb` files) for debugging
- Docker build scripts for Linux
- Integration example with omp-base-script-master
- Comprehensive README with API documentation

### Security
- Cryptographically secure random secret generation using OpenSSL
- Input validation for all user inputs
- Rate limiting to prevent brute force attacks
- Time-window tolerance of ±30 seconds for clock drift

## [Unreleased]

### Planned Features
- Optional secret encryption in database
- Backup codes for account recovery
- QR code generation for easier setup
- Admin panel for managing player 2FA
- Statistics and logging system
- Multi-language support
- Custom time step configuration
- Emergency disable functionality for admins

---

[1.0.0]: https://github.com/yourusername/neufox-2fa/releases/tag/v1.0.0

