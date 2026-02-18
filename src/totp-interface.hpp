#pragma once

/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include <sdk.hpp>
#include <string>
#include <optional>

// Maximum length for a base32 encoded secret (16 characters = 80 bits)
constexpr size_t TOTP_SECRET_LENGTH = 16;

// Stateless TOTP component - no player extensions needed
struct ITOTPComponent : IComponent
{
	// Visit https://open.mp/uid to generate a new unique ID (different to the extension UID).
	PROVIDE_UID(0x5572409DBD24A8BB);
};
