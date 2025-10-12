/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

// Include pawn-natives macros (`SCRIPT_API`) and lookups (`IPlayer&`).
#include <Server/Components/Pawn/Impl/pawn_natives.hpp>

// Include a few function implementations.  Should only be included once.
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>

// Include Impl headers for DefaultEventDispatcher
#include <Impl/events_impl.hpp>

// Include the component's definition.
#include "totp-component.hpp"

// Include the player data's definition.
#include "totp-extension.hpp"

// Include TOTP utilities
#include "totp-utils.hpp"

#include <ctime>
#include <chrono>

// Implementations of the various methods from the public API.
bool TOTPComponent::generateSecret(IPlayer& player, char* output)
{
	if (!output)
		return false;

	TOTPUtils::generateSecret(output, TOTP_SECRET_LENGTH + 1);

	return true;
}

bool TOTPComponent::enableTOTP(IPlayer& player, const char* secret)
{
	if (!secret)
		return false;

	size_t secretLen = strlen(secret);
	if (secretLen < 10 || secretLen > TOTP_SECRET_LENGTH)
		return false;

	// Validate secret is base32
	for (size_t i = 0; i < secretLen; i++)
	{
		char c = secret[i];
		if (!((c >= 'A' && c <= 'Z') || (c >= '2' && c <= '7') || (c >= 'a' && c <= 'z')))
			return false;
	}

	// Look up the custom data we added to the player for this component.
	if (ITOTPExtension* data = queryExtension<ITOTPExtension>(&player))
	{
		data->setSecret(secret);
		data->setEnabled(true);
		data->setVerified(false);
		data->resetFailedAttempts();

		// Emit event for other components
		eventDispatcher_.dispatch(&TOTPEventHandler::onTOTPEnabled, player);

		return true;
	}

	return false;
}

bool TOTPComponent::disableTOTP(IPlayer& player)
{
	// Look up the custom data we added to the player for this component.
	if (ITOTPExtension* data = queryExtension<ITOTPExtension>(&player))
	{
		data->setEnabled(false);
		data->setVerified(false);
		data->setSecret("");

		// Emit event for other components
		eventDispatcher_.dispatch(&TOTPEventHandler::onTOTPDisabled, player);

		return true;
	}

	return false;
}

bool TOTPComponent::verifyCode(IPlayer& player, const char* code)
{
	if (!code || strlen(code) != 6)
		return false;

	// Look up the custom data we added to the player for this component.
	ITOTPExtension* data = queryExtension<ITOTPExtension>(&player);
	if (!data || !data->isEnabled() || !data->hasSecret())
		return false;

	// Rate limiting - check if player is being rate limited
	auto nowSteady = std::chrono::steady_clock::now();
	TimePoint nowTimePoint = std::chrono::time_point_cast<Microseconds>(nowSteady);

	if (data->getFailedAttempts() >= MAX_FAILED_ATTEMPTS)
	{
		auto timeSinceLastAttempt = std::chrono::duration_cast<std::chrono::seconds>(
			nowTimePoint - data->getLastAttempt()
		).count();

		if (timeSinceLastAttempt < RATE_LIMIT_SECONDS)
		{
			// Still rate limited
			return false;
		}
		else
		{
			// Rate limit expired, reset
			data->resetFailedAttempts();
		}
	}

	data->setLastAttempt(nowTimePoint);

	// Get current Unix timestamp for TOTP (requires system_clock)
	auto nowSystem = std::chrono::system_clock::now();
	uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
		nowSystem.time_since_epoch()
	).count();

	// Verify the code
	bool success = TOTPUtils::verifyTOTP(data->getSecret(), code, timestamp);

	if (success)
	{
		data->setVerified(true);
		data->resetFailedAttempts();
	}
	else
	{
		data->incrementFailedAttempts();
	}

	// Emit event for other components
	eventDispatcher_.dispatch(&TOTPEventHandler::onTOTPVerify, player, success, code);

	// Tell pawn
	if (pawn_)
	{
		int playerID = player.getID();
		for (IPawnScript* script : pawn_->sideScripts())
		{
			// `forward OnPlayerTOTPVerify(playerid, bool:success);`
			script->Call("OnPlayerTOTPVerify", DefaultReturnValue_False, playerID, success);
		}
		// Call in the gamemode after all filterscripts.
		if (auto script = pawn_->mainScript())
		{
			script->Call("OnPlayerTOTPVerify", DefaultReturnValue_False, playerID, success);
		}
	}

	return success;
}

bool TOTPComponent::isEnabled(IPlayer& player)
{
	if (ITOTPExtension* data = queryExtension<ITOTPExtension>(&player))
	{
		return data->isEnabled();
	}
	return false;
}

bool TOTPComponent::isVerified(IPlayer& player)
{
	if (ITOTPExtension* data = queryExtension<ITOTPExtension>(&player))
	{
		return data->isVerified();
	}
	return false;
}

IEventDispatcher<TOTPEventHandler>& TOTPComponent::getEventDispatcher()
{
	return eventDispatcher_;
}

// Required component methods.
StringView TOTPComponent::componentName() const
{
	return "TOTP 2FA Component";
}

SemanticVersion TOTPComponent::componentVersion() const
{
	return SemanticVersion(1, 0, 0, 0);
}

void TOTPComponent::onLoad(ICore* c)
{
	// Cache core, listen to player events.
	core_ = c;
	// Register this component as wanting to be informed when a player (dis)connects.
	core_->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
	// Record the reference to `ICore` used by *pawn-natives*.
	setAmxLookups(core_);
}

void TOTPComponent::onInit(IComponentList* components)
{
	// Cache components, add event handlers here.
	pawn_ = components->queryComponent<IPawnComponent>();

	if (pawn_)
	{
		// For the legacy `amx_` C API this call sets the correct pointers so that pawn
		// function calls call the original versions within the server.
		setAmxFunctions(pawn_->getAmxFunctions());
		// For the pawn-natives system this call sets the various component references used for
		// parameter value lookups.
		setAmxLookups(components);
		// Register this component as wanting to be informed when a script is loaded.
		pawn_->getEventDispatcher().addEventHandler(this);
	}
}

void TOTPComponent::onReady()
{
	// Fire events here at earliest.
}

void TOTPComponent::onFree(IComponent* component)
{
	// Invalidate pawn pointer so it can't be used past this point.
	if (component == pawn_)
	{
		// Remove the internal pointer.
		pawn_ = nullptr;
		// Remove the pointers to the various `amx_` function implementations.
		setAmxFunctions();
		// Remove all pool lookup pointers.
		setAmxLookups();
	}
}

void TOTPComponent::free()
{
	// Deletes the component.
	delete this;
}

void TOTPComponent::reset()
{
	// Resets data when the mode changes.
	// Reset verification for all players
	if (core_)
	{
		for (IPlayer* player : core_->getPlayers().entries())
		{
			if (ITOTPExtension* data = queryExtension<ITOTPExtension>(player))
			{
				data->reset();
			}
		}
	}
}

// Connect event methods.
void TOTPComponent::onPlayerConnect(IPlayer& player)
{
	// Allocate a new copy of the extension and register it for `queryExtension` lookups.
	player.addExtension(new TOTPExtension(), true);
}

// Pawn event methods.
void TOTPComponent::onAmxLoad(IPawnScript& script)
{
	// Because we're using `SCRIPT_API` this call automatically registers the declared natives.
	pawn_natives::AmxLoad(script.GetAMX());
}

void TOTPComponent::onAmxUnload(IPawnScript& script)
{
}

// More methods to be used only in this component, with more implementation details knowledge.
TOTPComponent* TOTPComponent::getInstance()
{
	// Poor man's `Singleton`.
	if (instance_ == nullptr)
	{
		instance_ = new TOTPComponent();
	}
	return instance_;
}

// When this component is destroyed we need to tell any linked components this it is gone.
TOTPComponent::~TOTPComponent()
{
	// Clean up what you did above.
	if (pawn_)
	{
		pawn_->getEventDispatcher().removeEventHandler(this);
	}
	if (core_)
	{
		core_->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
	}
}
