/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include "totp-plugin.hpp"
#include "totp-player-data.hpp"
#include "version.hpp"

// ============================================================================
// Global variables for SA-MP plugin mode
// ============================================================================

void* pAMXFunctions = nullptr;
logprintf_t logprintf = nullptr;
bool isPluginMode = false;

// ============================================================================
// SA-MP Plugin Exports
// ============================================================================

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void** ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = reinterpret_cast<logprintf_t>(ppData[PLUGIN_DATA_LOGPRINTF]);
	isPluginMode = true;

	logprintf(" ");
	logprintf(" =======================================");
	logprintf("  neufox-2fa v%s loaded successfully", PLUGIN_VERSION);
	logprintf("  TOTP 2FA Authentication Plugin");
	logprintf(" =======================================");
	logprintf(" ");

	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	PlayerDataManager::Destroy();

	logprintf(" ");
	logprintf(" neufox-2fa: Plugin unloaded.");
	logprintf(" ");
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX* amx)
{
	return amx_Register(amx, native_list, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX* amx)
{
	return AMX_ERR_NONE;
}

// ============================================================================
// open.mp Component Entry Point
// ============================================================================

#ifndef SAMP_PLUGIN_BUILD

#include <sdk.hpp>
#include "totp-interface.hpp"
#include "totp-component.hpp"

COMPONENT_ENTRY_POINT()
{
	return TOTPComponent::getInstance();
}

#endif // SAMP_PLUGIN_BUILD
