/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>
#include <Impl/events_impl.hpp>
#include "totp-component.hpp"
#include "totp-extension.hpp"
#include "totp-utils.hpp"
#include <ctime>
#include <chrono>

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

	for (size_t i = 0; i < secretLen; i++)
	{
		char c = secret[i];
		if (!((c >= 'A' && c <= 'Z') || (c >= '2' && c <= '7') || (c >= 'a' && c <= 'z')))
			return false;
	}

	if (ITOTPExtension* data = queryExtension<ITOTPExtension>(&player))
	{
		data->setSecret(secret);
		data->setEnabled(true);
		data->setVerified(false);
		data->resetFailedAttempts();
		eventDispatcher_.dispatch(&TOTPEventHandler::onTOTPEnabled, player);

		return true;
	}

	return false;
}

bool TOTPComponent::disableTOTP(IPlayer& player)
{
	if (ITOTPExtension* data = queryExtension<ITOTPExtension>(&player))
	{
		data->setEnabled(false);
		data->setVerified(false);
		data->setSecret("");
		eventDispatcher_.dispatch(&TOTPEventHandler::onTOTPDisabled, player);

		return true;
	}

	return false;
}

bool TOTPComponent::verifyCode(IPlayer& player, const char* code)
{
	if (!code || strlen(code) != 6)
		return false;

	ITOTPExtension* data = queryExtension<ITOTPExtension>(&player);
	if (!data || !data->isEnabled() || !data->hasSecret())
		return false;

	auto nowSteady = std::chrono::steady_clock::now();
	TimePoint nowTimePoint = std::chrono::time_point_cast<Microseconds>(nowSteady);

	if (data->getFailedAttempts() >= MAX_FAILED_ATTEMPTS)
	{
		auto timeSinceLastAttempt = std::chrono::duration_cast<std::chrono::seconds>(
			nowTimePoint - data->getLastAttempt()
		).count();

		if (timeSinceLastAttempt < RATE_LIMIT_SECONDS)
			return false;
		else
			data->resetFailedAttempts();
	}

	data->setLastAttempt(nowTimePoint);

	auto nowSystem = std::chrono::system_clock::now();
	uint64_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(
		nowSystem.time_since_epoch()
	).count();

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

	eventDispatcher_.dispatch(&TOTPEventHandler::onTOTPVerify, player, success, code);

	if (pawn_)
	{
		int playerID = player.getID();
		for (IPawnScript* script : pawn_->sideScripts())
			script->Call("OnPlayerTOTPVerify", DefaultReturnValue_False, playerID, success);

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
	core_ = c;
	core_->getPlayers().getPlayerConnectDispatcher().addEventHandler(this);
	setAmxLookups(core_);
}

void TOTPComponent::onInit(IComponentList* components)
{
	pawn_ = components->queryComponent<IPawnComponent>();

	if (pawn_)
	{
		setAmxFunctions(pawn_->getAmxFunctions());
		setAmxLookups(components);
		pawn_->getEventDispatcher().addEventHandler(this);
	}
}

void TOTPComponent::onReady()
{
}

void TOTPComponent::onFree(IComponent* component)
{
	if (component == pawn_)
	{
		pawn_ = nullptr;
		setAmxFunctions();
		setAmxLookups();
	}
}

void TOTPComponent::free()
{
	delete this;
}

void TOTPComponent::reset()
{
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

void TOTPComponent::onPlayerConnect(IPlayer& player)
{
	player.addExtension(new TOTPExtension(), true);
}

void TOTPComponent::onAmxLoad(IPawnScript& script)
{
	pawn_natives::AmxLoad(script.GetAMX());
}

void TOTPComponent::onAmxUnload(IPawnScript& script)
{
}

TOTPComponent* TOTPComponent::getInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new TOTPComponent();
	}
	return instance_;
}

TOTPComponent::~TOTPComponent()
{
	if (pawn_)
	{
		pawn_->getEventDispatcher().removeEventHandler(this);
	}
	if (core_)
	{
		core_->getPlayers().getPlayerConnectDispatcher().removeEventHandler(this);
	}
}
