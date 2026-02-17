/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include "totp-natives.hpp"
#include "totp-plugin.hpp"
#include "totp-utils.hpp"
#include <chrono>
#include <cstring>

// ============================================================================
// SA-MP Native Implementations
// ============================================================================

// native bool:TOTP_GenerateSecret(output[], size = sizeof(output));
cell AMX_NATIVE_CALL n_TOTP_GenerateSecret(AMX* amx, const cell* params)
{
	auto secret = TOTPUtils::generateSecret();
	if (!secret)
		return 0;

	cell* addr;
	amx_GetAddr(amx, params[1], &addr);
	amx_SetString(addr, secret->c_str(), 0, 0, static_cast<size_t>(params[2]));

	return 1;
}

// native bool:TOTP_Verify(const secret[], const code[]);
cell AMX_NATIVE_CALL n_TOTP_Verify(AMX* amx, const cell* params)
{
	char secret[128];
	char code[16];

	cell* addr;
	amx_GetAddr(amx, params[1], &addr);
	amx_GetString(secret, addr, 0, sizeof(secret));

	amx_GetAddr(amx, params[2], &addr);
	amx_GetString(code, addr, 0, sizeof(code));

	// Validate inputs
	if (strlen(secret) == 0 || strlen(code) != 6)
		return 0;

	// Get current timestamp
	auto now = std::chrono::system_clock::now();
	uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
		now.time_since_epoch()
	).count();

	bool success = TOTPUtils::verifyTOTP(secret, code, timestamp);

	return success ? 1 : 0;
}

// ============================================================================
// SA-MP Native List
// ============================================================================

extern "C" const AMX_NATIVE_INFO native_list[] = {
	{"TOTP_GenerateSecret", n_TOTP_GenerateSecret},
	{"TOTP_Verify", n_TOTP_Verify},
	{NULL, NULL}
};

// ============================================================================
// open.mp SCRIPT_API Implementations (only compiled when not in plugin mode)
// ============================================================================

#ifndef SAMP_PLUGIN_BUILD

#include <sdk.hpp>
#include "totp-interface.hpp"

// native bool:TOTP_GenerateSecret(output[], size = sizeof(output));
SCRIPT_API(TOTP_GenerateSecret, bool(String& output))
{
	auto secret = TOTPUtils::generateSecret();
	if (secret)
	{
		output = *secret;
		return true;
	}
	return false;
}

// native bool:TOTP_Verify(const secret[], const code[]);
SCRIPT_API(TOTP_Verify, bool(String const& secret, String const& code))
{
	if (secret.empty() || code.length() != 6)
		return false;

	auto now = std::chrono::system_clock::now();
	uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
		now.time_since_epoch()
	).count();

	return TOTPUtils::verifyTOTP(secret.data(), code.data(), timestamp);
}

#endif // SAMP_PLUGIN_BUILD
