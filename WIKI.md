## Table of Contents
- [Introduction](#introduction)
- [Installation](#installation)
- [Database Setup](#database-setup)
- [Function Reference](#function-reference)
- [Complete Examples](#complete-examples)
- [Best Practices](#best-practices)

---

## Introduction

The neufox's 2FA plugin provides Time-based One-Time Password (TOTP) authentication for SA:MP/open.mp servers. It adds an extra layer of security by requiring players to enter a 6-digit code from an authenticator app (Google Authenticator, Authy, etc.) in addition to their password.

### Features
- **Stateless Design** - No player data stored in plugin memory
- Generate cryptographically secure TOTP secrets
- Verify 6-digit codes with time-based validation (±30 second window)
- **You control everything** - Store secrets where you want, implement your own rate limiting
- Lightweight and fast

### Architecture

This plugin is designed to be **stateless**. It only provides cryptographic functions for generating secrets and verifying codes. All state management (storing secrets, tracking verification status, rate limiting) is handled by **your gamemode**.

---

## Installation

1. Drop the `.dll` (Windows) or `.so` (Linux) file in your `plugins` folder (SA:MP) or `components` folder (open.mp)
2. Include the header in your gamemode:

```pawn
#include <neufox-2fa>
```

3. For SA:MP, add to `server.cfg`:
```
plugins neufox-2fa
```

For open.mp, the component will be automatically loaded.

---

## Database Setup

Add a `totp_secret` column to your player accounts table:

**Examples:**

```sql
ALTER TABLE `player_accounts`
ADD COLUMN `totp_secret` VARCHAR(16) DEFAULT '';
```
```sql
CREATE TABLE `player_accounts` (
  `account_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `username` varchar(24) NOT NULL,
  `password_hash` CHAR(60) BINARY NOT NULL,
  `totp_secret` VARCHAR(16) DEFAULT '',
  `register_date` datetime DEFAULT current_timestamp(),
  PRIMARY KEY (`account_id`),
  UNIQUE KEY `username` (`username`)
);
```

---

## Function Reference

### `TOTP_GenerateSecret(secret[], size = sizeof(secret))`
Generates a new random TOTP secret.

**Parameters:**
- `secret[]` - Output array to store the generated secret (must be at least 17 characters)
- `size` - Size of the output array (optional, defaults to sizeof)

**Returns:**
- `true` on success
- `false` on failure

**Example:**
```pawn
new secret[TOTP_SECRET_LENGTH + 1];
if (TOTP_GenerateSecret(secret))
{
    printf("Generated secret: %s", secret);
    // Store this in your database
}
else
{
    printf("Failed to generate 2FA secret.");
}
```

---

### `TOTP_Verify(const secret[], const code[])`
Verifies a 6-digit TOTP code against a secret.

**Parameters:**
- `secret[]` - The TOTP secret (16 characters, base32 encoded)
- `code[]` - The 6-digit code to verify

**Returns:**
- `true` if code is valid for the given secret
- `false` if code is invalid or parameters are incorrect

**Example:**
```pawn
// Load secret from database or gamemode variable
if (TOTP_Verify(PlayerData[playerid][pTOTPSecret], inputtext))
{
    SendClientMessage(playerid, COLOR_GREEN, "2FA verification successful!");
    // Allow login
}
else
{
    SendClientMessage(playerid, COLOR_RED, "Invalid 2FA code. Please try again.");
    // Implement your own rate limiting here
}
```

---

## Complete Examples

### Example 1: Enable 2FA During Registration

```pawn
hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (dialogid == DIALOG_REGISTER && response)
    {
        // After registration, generate 2FA secret
        new secret[TOTP_SECRET_LENGTH + 1];
        if (!TOTP_GenerateSecret(secret))
        {
            SendClientMessage(playerid, COLOR_RED, "Failed to generate 2FA secret.");
            return 1;
        }

        // Save to database
        new query[256];
        mysql_format(g_DatabaseHandle, query, sizeof(query),
            "UPDATE `player_accounts` SET `totp_secret` = '%e' WHERE `username` = '%e'",
            secret, GetPlayerNameEx(playerid)
        );
        mysql_tquery(g_DatabaseHandle, query);

        // Show secret to player
        new dialogText[256];
        format(dialogText, sizeof(dialogText),
            "Your 2FA Secret: {FFFF00}%s\n\n\
            {FFFFFF}Add this to your authenticator app (Google Authenticator, Authy, etc.)\n\
            and enter the 6-digit code to verify:",
            secret
        );

        ShowPlayerDialog(playerid, DIALOG_2FA_VERIFY, DIALOG_STYLE_INPUT,
            "Two-Factor Authentication", dialogText, "Verify", "Skip");
    }

    if (dialogid == DIALOG_2FA_VERIFY && response)
    {
        // Load secret from database
        new secret[TOTP_SECRET_LENGTH + 1];
        cache_get_value_name(0, "totp_secret", secret);

        if (TOTP_Verify(secret, inputtext))
        {
            SendClientMessage(playerid, COLOR_GREEN, "2FA enabled successfully!");
        }
        else
        {
            SendClientMessage(playerid, COLOR_RED, "Invalid code. 2FA setup cancelled.");

            // Clear secret from database
            new query[128];
            mysql_format(g_DatabaseHandle, query, sizeof(query),
                "UPDATE `player_accounts` SET `totp_secret` = '' WHERE `username` = '%e'",
                GetPlayerNameEx(playerid)
            );
            mysql_tquery(g_DatabaseHandle, query);
        }
    }

    return 1;
}
```

---

### Example 2: Verify 2FA During Login

```pawn
enum E_PLAYER_DATA
{
    E_PLAYER_TOTP_SECRET[TOTP_SECRET_LENGTH + 1],
    bool:E_PLAYER_LOGGED_IN,
    E_PLAYER_FAILED_2FA_ATTEMPTS,
    E_PLAYER_LAST_2FA_ATTEMPT
};
new PlayerData[MAX_PLAYERS][E_PLAYER_DATA];

hook OnPasswordCheck(playerid, bool:match)
{
    if (match)
    {
        // Load 2FA secret from database
        cache_get_value_name(0, "totp_secret", PlayerData[playerid][E_PLAYER_TOTP_SECRET]);

        // Check if player has 2FA enabled
        if (strlen(PlayerData[playerid][E_PLAYER_TOTP_SECRET]) > 0)
        {
            ShowPlayerDialog(playerid, DIALOG_2FA_LOGIN, DIALOG_STYLE_INPUT,
                "Two-Factor Authentication",
                "Enter the 6-digit code from your authenticator app:",
                "Verify", "Quit");
        }
        else
        {
            // No 2FA, login directly
            SetPlayerLoggedIn(playerid, true);
        }
    }

    return 1;
}

hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (dialogid == DIALOG_2FA_LOGIN)
    {
        if (!response) return Kick(playerid);

        // Rate limiting (3 attempts per 60 seconds)
        new currentTime = gettime();
        if (PlayerData[playerid][E_PLAYER_FAILED_2FA_ATTEMPTS] >= 3)
        {
            if (currentTime - PlayerData[playerid][E_PLAYER_LAST_2FA_ATTEMPT] < 60)
            {
                SendClientMessage(playerid, COLOR_RED, "Too many failed attempts. Wait 60 seconds.");
                return 1;
            }
            else
            {
                // Reset after cooldown
                PlayerData[playerid][E_PLAYER_FAILED_2FA_ATTEMPTS] = 0;
            }
        }

        // Verify code
        if (TOTP_Verify(PlayerData[playerid][E_PLAYER_TOTP_SECRET], inputtext))
        {
            SetPlayerLoggedIn(playerid, true);
            PlayerData[playerid][E_PLAYER_FAILED_2FA_ATTEMPTS] = 0;
            SendClientMessage(playerid, COLOR_GREEN, "2FA verified! Logged in successfully.");
        }
        else
        {
            PlayerData[playerid][E_PLAYER_FAILED_2FA_ATTEMPTS]++;
            PlayerData[playerid][E_PLAYER_LAST_2FA_ATTEMPT] = currentTime;

            new string[128];
            format(string, sizeof(string),
                "Invalid 2FA code. Failed attempts: %d/3",
                PlayerData[playerid][E_PLAYER_FAILED_2FA_ATTEMPTS]
            );
            SendClientMessage(playerid, COLOR_RED, string);

            // Show dialog again
            ShowPlayerDialog(playerid, DIALOG_2FA_LOGIN, DIALOG_STYLE_INPUT,
                "Two-Factor Authentication",
                "Enter the 6-digit code from your authenticator app:",
                "Verify", "Quit");
        }
    }

    return 1;
}
```

---

### Example 3: Disable 2FA Command

```pawn
CMD:disable2fa(playerid, params[])
{
    if (!PlayerData[playerid][E_PLAYER_LOGGED_IN])
        return SendClientMessage(playerid, COLOR_RED, "You must be logged in.");

    if (strlen(PlayerData[playerid][E_PLAYER_TOTP_SECRET]) == 0)
        return SendClientMessage(playerid, COLOR_RED, "You don't have 2FA enabled.");

    // Clear from database
    new query[128];
    mysql_format(g_DatabaseHandle, query, sizeof(query),
        "UPDATE `player_accounts` SET `totp_secret` = '' WHERE `account_id` = %d",
        GetPlayerAccountID(playerid)
    );
    mysql_tquery(g_DatabaseHandle, query);

    // Clear from memory
    PlayerData[playerid][E_PLAYER_TOTP_SECRET][0] = EOS;

    SendClientMessage(playerid, COLOR_YELLOW, "Two-Factor Authentication has been disabled.");

    return 1;
}
```

---

## Best Practices

### 1. **Always Store Secrets Securely**
```pawn
// Good: Store in database with proper escaping
mysql_format(db, query, sizeof(query),
    "UPDATE `accounts` SET `totp_secret` = '%e' WHERE `id` = %d",
    secret, accountID
);
```

### 2. **Implement Rate Limiting**
```pawn
// Good: Track failed attempts and implement cooldown
if (failedAttempts >= 3)
{
    if (gettime() - lastAttempt < 60)
    {
        return SendClientMessage(playerid, -1, "Too many failed attempts. Wait 60 seconds.");
    }
}
```

### 3. **Display Secrets Clearly**
```pawn
// Good: Split secret into colored segments for easier reading
new seg1[5], seg2[5], seg3[5], seg4[5];
strmid(seg1, secret, 0, 4);
strmid(seg2, secret, 4, 8);
strmid(seg3, secret, 8, 12);
strmid(seg4, secret, 12, 16);

format(dialogText, sizeof(dialogText),
    "Your 2FA Secret:\n{00FF00}%s{FFFF00}%s{FF6600}%s{00FFFF}%s\n\n\
    Add this to your authenticator app.",
    seg1, seg2, seg3, seg4
);
```

### 4. **Clear Secrets on Disconnect**
```pawn
// Good: Clear sensitive data from memory
hook OnPlayerDisconnect(playerid, reason)
{
    PlayerData[playerid][E_PLAYER_TOTP_SECRET][0] = EOS;
    PlayerData[playerid][E_PLAYER_FAILED_2FA_ATTEMPTS] = 0;
    return 1;
}
```

### 5. **Validate Input**
```pawn
// Good: Check code length before verification
if (strlen(inputtext) != TOTP_CODE_LENGTH)
{
    return SendClientMessage(playerid, -1, "2FA code must be exactly 6 digits.");
}

// Check if it's numeric
for (new i = 0; i < TOTP_CODE_LENGTH; i++)
{
    if (inputtext[i] < '0' || inputtext[i] > '9')
    {
        return SendClientMessage(playerid, -1, "2FA code must contain only numbers.");
    }
}
```

---

For issues or questions, please visit: [GitHub Repository](https://github.com/itsneufox/neufox-2FA)
