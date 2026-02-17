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
 * <library name="neufox-2fa" summary="neufox's 2FA - TOTP Two-Factor Authentication Plugin">
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
 * <summary>Generate a new random TOTP secret.</summary>
 * <param name="output">Array to store the generated secret (must be at least 17 characters).</param>
 * <param name="size">Size of the output array.</param>
 * <remarks>
 *   Generates a random 16-character base32 encoded secret that can be used with authenticator apps
 *   like Google Authenticator, Authy, or Microsoft Authenticator.
 *   This is a stateless function - it does not store the secret anywhere.
 *   You must store the secret in your database or gamemode variables.
 * </remarks>
 * <returns>
 *   <b><c>true</c></b> - Secret was generated successfully.<br />
 *   <b><c>false</c></b> - Failed to generate secret.
 * </returns>
 */
native bool:TOTP_GenerateSecret(output[], size = sizeof(output));

/**
 * <library>neufox-2fa</library>
 * <summary>Verify a TOTP code against a secret.</summary>
 * <param name="secret">The base32 encoded secret (16 characters).</param>
 * <param name="code">The 6-digit TOTP code to verify.</param>
 * <remarks>
 *   Verifies a TOTP code against the provided secret. The code is time-based and changes every 30 seconds.
 *   Includes a time window of ±30 seconds to account for clock drift.
 *   This is a stateless function - it does not track failed attempts or player state.
 *   You must implement your own rate limiting and state management.
 * </remarks>
 * <returns>
 *   <b><c>true</c></b> - Code is valid for the given secret.<br />
 *   <b><c>false</c></b> - Code is invalid or parameters are incorrect.
 * </returns>
 */
native bool:TOTP_Verify(const secret[], const code[]);
