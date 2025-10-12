#pragma once

/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include <sdk.hpp>
#include "totp-interface.hpp"
#include <Server/Components/Pawn/pawn.hpp>
#include <Impl/events_impl.hpp>

using namespace Impl;

class TOTPComponent final
	: public ITOTPComponent
	, public PlayerConnectEventHandler
	, public PawnEventHandler
{
private:
	ICore* core_ = nullptr;
	IPawnComponent* pawn_ = nullptr;
	DefaultEventDispatcher<TOTPEventHandler> eventDispatcher_;
	inline static TOTPComponent* instance_ = nullptr;

	static constexpr int MAX_FAILED_ATTEMPTS = 3;
	static constexpr int RATE_LIMIT_SECONDS = 60;

public:
	bool generateSecret(IPlayer& player, char* output) override;

	bool enableTOTP(IPlayer& player, const char* secret) override;

	bool disableTOTP(IPlayer& player) override;

	bool verifyCode(IPlayer& player, const char* code) override;

	bool isEnabled(IPlayer& player) override;

	bool isVerified(IPlayer& player) override;

	IEventDispatcher<TOTPEventHandler>& getEventDispatcher() override;

	StringView componentName() const override;

	SemanticVersion componentVersion() const override;

	void onLoad(ICore* c) override;

	void onInit(IComponentList* components) override;

	void onReady() override;

	void onFree(IComponent* component) override;

	void free() override;

	void reset() override;

	void onPlayerConnect(IPlayer& player) override;
	void onAmxLoad(IPawnScript& script) override;

	void onAmxUnload(IPawnScript& script) override;

	static TOTPComponent* getInstance();
	~TOTPComponent();
};
