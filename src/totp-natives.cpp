/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include "totp-natives.hpp"
#include "totp-plugin.hpp"
#include "totp-player-data.hpp"
#include "totp-utils.hpp"
#include <chrono>
#include <cstring>

// Rate limiting constants
constexpr int MAX_FAILED_ATTEMPTS = 3;
constexpr int RATE_LIMIT_SECONDS = 60;

// ============================================================================
// SA-MP Native Implementations
// ============================================================================

// native bool:TOTP_GenerateSecret(playerid, output[], size = sizeof(output));
cell AMX_NATIVE_CALL n_TOTP_GenerateSecret(AMX* amx, const cell* params)
{
	int playerid = static_cast<int>(params[1]);

	PlayerTOTPData* data = PlayerDataManager::Get().GetPlayer(playerid);
	if (!data)
		return 0;

	char secret[TOTP_SECRET_LENGTH_SAMP + 1];
	TOTPUtils::generateSecret(secret, sizeof(secret));

	cell* addr;
	amx_GetAddr(amx, params[2], &addr);
	amx_SetString(addr, secret, 0, 0, static_cast<size_t>(params[3]));

	return 1;
}

// native bool:TOTP_Enable(playerid, const secret[]);
cell AMX_NATIVE_CALL n_TOTP_Enable(AMX* amx, const cell* params)
{
	int playerid = static_cast<int>(params[1]);

	PlayerTOTPData* data = PlayerDataManager::Get().GetPlayer(playerid);
	if (!data)
		return 0;

	char secret[TOTP_SECRET_LENGTH_SAMP + 1];
	cell* addr;
	amx_GetAddr(amx, params[2], &addr);
	amx_GetString(secret, addr, 0, sizeof(secret));

	size_t secretLen = strlen(secret);
	if (secretLen < 10 || secretLen > TOTP_SECRET_LENGTH_SAMP)
		return 0;

	for (size_t i = 0; i < secretLen; i++)
	{
		char c = secret[i];
		if (!((c >= 'A' && c <= 'Z') || (c >= '2' && c <= '7') || (c >= 'a' && c <= 'z')))
			return 0;
	}

	data->setSecret(secret);
	data->enabled = true;
	data->verified = false;
	data->failedAttempts = 0;

	return 1;
}

// native bool:TOTP_Disable(playerid);
cell AMX_NATIVE_CALL n_TOTP_Disable(AMX* amx, const cell* params)
{
	int playerid = static_cast<int>(params[1]);

	PlayerTOTPData* data = PlayerDataManager::Get().GetPlayer(playerid);
	if (!data)
		return 0;

	data->enabled = false;
	data->verified = false;
	data->setSecret("");

	return 1;
}

// native bool:TOTP_Verify(playerid, const code[]);
cell AMX_NATIVE_CALL n_TOTP_Verify(AMX* amx, const cell* params)
{
	int playerid = static_cast<int>(params[1]);

	PlayerTOTPData* data = PlayerDataManager::Get().GetPlayer(playerid);
	if (!data || !data->enabled || !data->hasSecret())
		return 0;

	char code[16];
	cell* addr;
	amx_GetAddr(amx, params[2], &addr);
	amx_GetString(code, addr, 0, sizeof(code));

	if (strlen(code) != 6)
		return 0;

	auto now = std::chrono::steady_clock::now();

	if (data->failedAttempts >= MAX_FAILED_ATTEMPTS)
	{
		auto timeSinceLastAttempt = std::chrono::duration_cast<std::chrono::seconds>(
			now - data->lastAttempt
		).count();

		if (timeSinceLastAttempt < RATE_LIMIT_SECONDS)
			return 0;
		else
			data->failedAttempts = 0;
	}

	data->lastAttempt = now;

	auto nowSystem = std::chrono::system_clock::now();
	uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
		nowSystem.time_since_epoch()
	).count();

	bool success = TOTPUtils::verifyTOTP(data->secret, code, timestamp);

	if (success)
	{
		data->verified = true;
		data->failedAttempts = 0;
	}
	else
	{
		data->failedAttempts++;
	}

	return success ? 1 : 0;
}

// native bool:TOTP_IsEnabled(playerid);
cell AMX_NATIVE_CALL n_TOTP_IsEnabled(AMX* amx, const cell* params)
{
	int playerid = static_cast<int>(params[1]);

	PlayerTOTPData* data = PlayerDataManager::Get().GetPlayer(playerid);
	if (!data)
		return 0;

	return data->enabled ? 1 : 0;
}

// native bool:TOTP_IsVerified(playerid);
cell AMX_NATIVE_CALL n_TOTP_IsVerified(AMX* amx, const cell* params)
{
	int playerid = static_cast<int>(params[1]);

	PlayerTOTPData* data = PlayerDataManager::Get().GetPlayer(playerid);
	if (!data)
		return 0;

	return data->verified ? 1 : 0;
}

// native bool:TOTP_GetSecret(playerid, output[], size = sizeof(output));
cell AMX_NATIVE_CALL n_TOTP_GetSecret(AMX* amx, const cell* params)
{
	int playerid = static_cast<int>(params[1]);

	PlayerTOTPData* data = PlayerDataManager::Get().GetPlayer(playerid);
	if (!data || !data->hasSecret())
		return 0;

	cell* addr;
	amx_GetAddr(amx, params[2], &addr);
	amx_SetString(addr, data->secret, 0, 0, static_cast<size_t>(params[3]));

	return 1;
}

// native TOTP_GetFailedAttempts(playerid);
cell AMX_NATIVE_CALL n_TOTP_GetFailedAttempts(AMX* amx, const cell* params)
{
	int playerid = static_cast<int>(params[1]);

	PlayerTOTPData* data = PlayerDataManager::Get().GetPlayer(playerid);
	if (!data)
		return 0;

	return data->failedAttempts;
}

// native TOTP_ResetVerification(playerid);
cell AMX_NATIVE_CALL n_TOTP_ResetVerification(AMX* amx, const cell* params)
{
	int playerid = static_cast<int>(params[1]);

	PlayerTOTPData* data = PlayerDataManager::Get().GetPlayer(playerid);
	if (!data)
		return 0;

	data->verified = false;
	return 1;
}

// ============================================================================
// SA-MP Native List
// ============================================================================

extern "C" const AMX_NATIVE_INFO native_list[] = {
	{"TOTP_GenerateSecret", n_TOTP_GenerateSecret},
	{"TOTP_Enable", n_TOTP_Enable},
	{"TOTP_Disable", n_TOTP_Disable},
	{"TOTP_Verify", n_TOTP_Verify},
	{"TOTP_IsEnabled", n_TOTP_IsEnabled},
	{"TOTP_IsVerified", n_TOTP_IsVerified},
	{"TOTP_GetSecret", n_TOTP_GetSecret},
	{"TOTP_GetFailedAttempts", n_TOTP_GetFailedAttempts},
	{"TOTP_ResetVerification", n_TOTP_ResetVerification},
	{NULL, NULL}
};

// ============================================================================
// open.mp SCRIPT_API Implementations (only compiled when not in plugin mode)
// ============================================================================

#ifndef SAMP_PLUGIN_BUILD

#include <sdk.hpp>
#include "totp-interface.hpp"
#include "totp-extension.hpp"

// native bool:TOTP_GenerateSecret(playerid, output[], size = sizeof(output));
SCRIPT_API(TOTP_GenerateSecret, bool(IPlayer& player, String& output))
{
	if (auto totp = TOTPComponent::getInstance())
	{
		char secret[TOTP_SECRET_LENGTH + 1];
		if (totp->generateSecret(player, secret))
		{
			output = secret;
			return true;
		}
	}
	return false;
}

// Enable TOTP 2FA for a player with a given secret
// native bool:TOTP_Enable(playerid, const secret[]);
SCRIPT_API(TOTP_Enable, bool(IPlayer& player, String const& secret))
{
	if (auto totp = TOTPComponent::getInstance())
	{
		return totp->enableTOTP(player, secret.c_str());
	}
	return false;
}

// Disable TOTP 2FA for a player
// native bool:TOTP_Disable(playerid);
SCRIPT_API(TOTP_Disable, bool(IPlayer& player))
{
	if (auto totp = TOTPComponent::getInstance())
	{
		return totp->disableTOTP(player);
	}
	return false;
}

// Verify a TOTP code for a player
// native bool:TOTP_Verify(playerid, const code[]);
SCRIPT_API(TOTP_Verify, bool(IPlayer& player, String const& code))
{
	if (auto totp = TOTPComponent::getInstance())
	{
		return totp->verifyCode(player, code.c_str());
	}
	return false;
}

// Check if a player has TOTP enabled
// native bool:TOTP_IsEnabled(playerid);
SCRIPT_API(TOTP_IsEnabled, bool(IPlayer& player))
{
	if (auto totp = TOTPComponent::getInstance())
	{
		return totp->isEnabled(player);
	}
	return false;
}

// Check if a player is verified with TOTP (logged in with 2FA)
// native bool:TOTP_IsVerified(playerid);
SCRIPT_API(TOTP_IsVerified, bool(IPlayer& player))
{
	if (auto totp = TOTPComponent::getInstance())
	{
		return totp->isVerified(player);
	}
	return false;
}

// Get the player's TOTP secret (for display/storage)
// native bool:TOTP_GetSecret(playerid, output[], size = sizeof(output));
SCRIPT_API(TOTP_GetSecret, bool(IPlayer& player, String& output))
{
	if (auto data = queryExtension<ITOTPExtension>(player))
	{
		if (data->hasSecret())
		{
			output = data->getSecret();
			return true;
		}
	}
	return false;
}

// Get the number of failed verification attempts for rate limiting
// native TOTP_GetFailedAttempts(playerid);
SCRIPT_API(TOTP_GetFailedAttempts, int(IPlayer& player))
{
	if (auto data = queryExtension<ITOTPExtension>(player))
	{
		return data->getFailedAttempts();
	}
	return 0;
}

// Reset verification status for a player (useful for re-login scenarios)
// native bool:TOTP_ResetVerification(playerid);
SCRIPT_API(TOTP_ResetVerification, bool(IPlayer& player))
{
	if (auto data = queryExtension<ITOTPExtension>(player))
	{
		data->setVerified(false);
		return true;
	}
	return false;
}

#endif // SAMP_PLUGIN_BUILD
