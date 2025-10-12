#pragma once

/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

// Required for most of open.mp.
#include <sdk.hpp>

// Maximum length for a base32 encoded secret (16 characters = 80 bits)
constexpr size_t TOTP_SECRET_LENGTH = 16;

// If this data is to be used in other components only share an ABI stable base class.
struct ITOTPExtension : IExtension
{
	// Visit https://open.mp/uid to generate a new unique ID (different to the component UID).
	PROVIDE_EXT_UID(0x213BD4923106B488);

	// Public methods to manage TOTP for this player.
	virtual bool isEnabled() const = 0;

	virtual bool isVerified() const = 0;

	virtual void setEnabled(bool enabled) = 0;

	virtual void setVerified(bool verified) = 0;

	virtual bool hasSecret() const = 0;

	virtual void setSecret(const char* secret) = 0;

	virtual const char* getSecret() const = 0;

	virtual int getFailedAttempts() const = 0;

	virtual void incrementFailedAttempts() = 0;

	virtual void resetFailedAttempts() = 0;

	virtual TimePoint getLastAttempt() const = 0;

	virtual void setLastAttempt(TimePoint time) = 0;
};

// If other components want to subscribe to our TOTP event they must implement this interface.
struct TOTPEventHandler
{
	// Called when a player attempts to verify a TOTP code.
	virtual void onTOTPVerify(IPlayer& player, bool success, const char* code) = 0;

	// Called when a player enables TOTP.
	virtual void onTOTPEnabled(IPlayer& player) = 0;

	// Called when a player disables TOTP.
	virtual void onTOTPDisabled(IPlayer& player) = 0;
};

// If this data is to be used in other components only share an ABI stable base class.
struct ITOTPComponent : IComponent
{
	// Visit https://open.mp/uid to generate a new unique ID (different to the extension UID).
	PROVIDE_UID(0x5572409DBD24A8BB);

	// Generate a new random secret for a player.
	virtual bool generateSecret(IPlayer& player, char* output) = 0;

	// Enable TOTP for a player with a given secret.
	virtual bool enableTOTP(IPlayer& player, const char* secret) = 0;

	// Disable TOTP for a player.
	virtual bool disableTOTP(IPlayer& player) = 0;

	// Verify a TOTP code for a player.
	virtual bool verifyCode(IPlayer& player, const char* code) = 0;

	// Check if a player has TOTP enabled.
	virtual bool isEnabled(IPlayer& player) = 0;

	// Check if a player is verified (logged in with 2FA).
	virtual bool isVerified(IPlayer& player) = 0;

	// A way for other components to look up and subscribe to this component's events.
	virtual IEventDispatcher<TOTPEventHandler>& getEventDispatcher() = 0;
};
