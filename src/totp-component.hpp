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

class TOTPComponent final
	: public ITOTPComponent
	, public PawnEventHandler
{
private:
	ICore* core_ = nullptr;
	IPawnComponent* pawn_ = nullptr;
	inline static TOTPComponent* instance_ = nullptr;

public:
	StringView componentName() const override;

	SemanticVersion componentVersion() const override;

	void onLoad(ICore* c) override;

	void onInit(IComponentList* components) override;

	void onReady() override;

	void onFree(IComponent* component) override;

	void free() override;

	void reset() override;

	void onAmxLoad(IPawnScript& script) override;

	void onAmxUnload(IPawnScript& script) override;

	static TOTPComponent* getInstance();
	~TOTPComponent();
};
