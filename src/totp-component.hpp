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

// This is the private implementation of the public interface.  We must know the interface.
#include "totp-interface.hpp"

// Import the pawn event.
#include <Server/Components/Pawn/pawn.hpp>

// Import Impl headers for DefaultEventDispatcher
#include <Impl/events_impl.hpp>

// Import open.mp structures that aren't ABI safe.
using namespace Impl;

// `final` so we don't need virtual destructors.  Also because we know it isn't inherited.
class TOTPComponent final
	// This class is an implementation of the publicly shared `ITOTPComponent` interface.
	: public ITOTPComponent
	// The implementation includes player connection events to know when new players join.
	, public PlayerConnectEventHandler
	// The implementation includes pawn script events to know when new scripts load.
	, public PawnEventHandler
{
private:
	// Hold a reference to the main server core.
	ICore* core_ = nullptr;

	// We use the pawn componet to add and remove script load listeners.
	IPawnComponent* pawn_ = nullptr;

	// Create a store for other components to be informed of TOTP events.
	DefaultEventDispatcher<TOTPEventHandler> eventDispatcher_;

	inline static TOTPComponent* instance_ = nullptr;

	// Rate limiting configuration
	static constexpr int MAX_FAILED_ATTEMPTS = 3;
	static constexpr int RATE_LIMIT_SECONDS = 60;

public:
	// Implementations of the various methods from the public API.
	bool generateSecret(IPlayer& player, char* output) override;

	bool enableTOTP(IPlayer& player, const char* secret) override;

	bool disableTOTP(IPlayer& player) override;

	bool verifyCode(IPlayer& player, const char* code) override;

	bool isEnabled(IPlayer& player) override;

	bool isVerified(IPlayer& player) override;

	IEventDispatcher<TOTPEventHandler>& getEventDispatcher() override;

	// Required component methods.
	StringView componentName() const override;

	SemanticVersion componentVersion() const override;

	void onLoad(ICore* c) override;

	void onInit(IComponentList* components) override;

	void onReady() override;

	void onFree(IComponent* component) override;

	void free() override;

	void reset() override;

	// Connect event methods.
	void onPlayerConnect(IPlayer& player) override;

	// Pawn event methods.
	void onAmxLoad(IPawnScript& script) override;

	void onAmxUnload(IPawnScript& script) override;

	// More methods to be used only in this component, with more implementation details knowledge.
	static TOTPComponent* getInstance();

	// When this component is destroyed we need to tell any linked components this it is gone.
	~TOTPComponent();
};
