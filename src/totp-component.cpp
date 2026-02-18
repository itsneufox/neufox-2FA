/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include <Server/Components/Pawn/Impl/pawn_natives.hpp>
#include <Server/Components/Pawn/Impl/pawn_impl.hpp>
#include "totp-component.hpp"

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
	// Stateless - nothing to reset
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
}
