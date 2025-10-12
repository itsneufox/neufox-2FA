/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#if defined _INC_neufox_2fa
	#endinput
#endif
#define _INC_neufox_2fa

/**
 * <library name="neufox-2fa" summary="neufox's 2FA - TOTP Two-Factor Authentication Component">
 *   <license>
 *     (c) Copyright 2025, neufox.
 *   </license>
 *   <summary pawndoc="true">
 *     This library uses the enhanced <em>pawndoc.xsl</em> from
 *     <a href="https://github.com/pawn-lang/pawndoc">pawn-lang/pawndoc</a>.
 *     This XSL has features such as library and markdown support, and will not
 *     render this message when used.
 *   </summary>
 * </library>
 */

/// <p/>

#pragma tabsize 4

/**
 * <library>neufox-2fa</library>
 * <summary>Maximum length of a TOTP secret (16 characters, base32 encoded)</summary>
 */
const TOTP_SECRET_LENGTH = 16;

/**
 * <library>neufox-2fa</library>
 * <summary>TOTP code length (6 digits)</summary>
 */
const TOTP_CODE_LENGTH = 6;

/**
 * <library>neufox-2fa</library>
 * <summary>Generate a new random TOTP secret for a player.</summary>
 * <param name="playerid">The ID of the player.</param>
 * <param name="output">Array to store the generated secret (must be at least 17 characters).</param>
 * <param name="size">Size of the output array.</param>
 * <remarks>
 *   Generates a random 16-character base32 encoded secret that can be used with authenticator apps
 *   like Google Authenticator, Authy, or Microsoft Authenticator.
 * </remarks>
 * <returns>
 *   <b><c>true</c></b> - Secret was generated successfully.<br />
 *   <b><c>false</c></b> - Failed to generate secret.
 * </returns>
 */
native bool:TOTP_GenerateSecret(playerid, output[], size = sizeof(output));

/**
 * <library>neufox-2fa</library>
 * <summary>Enable TOTP 2FA authentication for a player.</summary>
 * <param name="playerid">The ID of the player.</param>
 * <param name="secret">The base32 encoded secret (16 characters).</param>
 * <remarks>
 *   Enables TOTP 2FA for the player. The secret must be a valid 16-character base32 string.
 *   After enabling, the player will need to verify codes using TOTP_Verify.
 * </remarks>
 * <returns>
 *   <b><c>true</c></b> - TOTP was enabled successfully.<br />
 *   <b><c>false</c></b> - Failed to enable TOTP (invalid secret or player not connected).
 * </returns>
 */
native bool:TOTP_Enable(playerid, const secret[]);

/**
 * <library>neufox-2fa</library>
 * <summary>Disable TOTP 2FA authentication for a player.</summary>
 * <param name="playerid">The ID of the player.</param>
 * <remarks>Disables TOTP 2FA for the player and clears their secret.</remarks>
 * <returns>
 *   <b><c>true</c></b> - TOTP was disabled successfully.<br />
 *   <b><c>false</c></b> - Failed to disable TOTP.
 * </returns>
 */
native bool:TOTP_Disable(playerid);

/**
 * <library>neufox-2fa</library>
 * <summary>Verify a TOTP code for a player.</summary>
 * <param name="playerid">The ID of the player.</param>
 * <param name="code">The 6-digit TOTP code to verify.</param>
 * <remarks>
 *   Verifies a TOTP code against the player's secret. The code is time-based and changes every 30 seconds.
 *   Includes a time window of ±30 seconds to account for clock drift.
 *   Rate limited to 3 failed attempts per 60 seconds.
 * </remarks>
 * <returns>
 *   <b><c>true</c></b> - Code is valid and player is now verified.<br />
 *   <b><c>false</c></b> - Code is invalid, TOTP not enabled, or rate limited.
 * </returns>
 */
native bool:TOTP_Verify(playerid, const code[]);

/**
 * <library>neufox-2fa</library>
 * <summary>Check if a player has TOTP 2FA enabled.</summary>
 * <param name="playerid">The ID of the player.</param>
 * <returns>
 *   <b><c>true</c></b> - TOTP is enabled for this player.<br />
 *   <b><c>false</c></b> - TOTP is not enabled.
 * </returns>
 */
native bool:TOTP_IsEnabled(playerid);

/**
 * <library>neufox-2fa</library>
 * <summary>Check if a player is verified with TOTP (has successfully entered a valid code).</summary>
 * <param name="playerid">The ID of the player.</param>
 * <remarks>
 *   Players must verify their TOTP code after connecting if they have TOTP enabled.
 *   This function returns true only after a successful verification.
 * </remarks>
 * <returns>
 *   <b><c>true</c></b> - Player is verified with TOTP.<br />
 *   <b><c>false</c></b> - Player is not verified.
 * </returns>
 */
native bool:TOTP_IsVerified(playerid);

/**
 * <library>neufox-2fa</library>
 * <summary>Get the player's TOTP secret.</summary>
 * <param name="playerid">The ID of the player.</param>
 * <param name="output">Array to store the secret.</param>
 * <param name="size">Size of the output array.</param>
 * <remarks>
 *   WARNING: Keep secrets secure! Only use this for displaying the secret when first enabling TOTP.
 *   Consider storing secrets in a database instead of retrieving them repeatedly.
 * </remarks>
 * <returns>
 *   <b><c>true</c></b> - Secret was retrieved successfully.<br />
 *   <b><c>false</c></b> - Player has no secret or TOTP is not enabled.
 * </returns>
 */
native bool:TOTP_GetSecret(playerid, output[], size = sizeof(output));

/**
 * <library>neufox-2fa</library>
 * <summary>Get the number of failed verification attempts for a player.</summary>
 * <param name="playerid">The ID of the player.</param>
 * <remarks>Used for rate limiting. After 3 failed attempts, the player is rate limited for 60 seconds.</remarks>
 * <returns>Number of failed verification attempts.</returns>
 */
native TOTP_GetFailedAttempts(playerid);

/**
 * <library>neufox-2fa</library>
 * <param name="playerid">The ID of the player who attempted verification.</param>
 * <param name="success">Whether the verification was successful.</param>
 * <summary>This callback is triggered when a player attempts to verify a TOTP code.</summary>
 * <remarks>
 *   This callback is called every time a player tries to verify a TOTP code.
 *   Use this to implement custom authentication flows, logging, or additional security measures.
 * </remarks>
 */
forward OnPlayerTOTPVerify(playerid, bool:success);

