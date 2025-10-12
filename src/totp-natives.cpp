/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include <sdk.hpp>
#include "totp-interface.hpp"
#include "totp-natives.hpp"
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

