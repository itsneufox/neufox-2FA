/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2025, itsneufox.
 */

#include "totp-extension.hpp"
#include <cstring>

bool TOTPExtension::isEnabled() const
{
	return enabled_;
}

bool TOTPExtension::isVerified() const
{
	return verified_;
}

void TOTPExtension::setEnabled(bool enabled)
{
	enabled_ = enabled;
	if (!enabled)
		verified_ = false;
}

void TOTPExtension::setVerified(bool verified)
{
	verified_ = verified;
}

bool TOTPExtension::hasSecret() const
{
	return !secret_.empty();
}

void TOTPExtension::setSecret(const char* secret)
{
	if (secret && std::strlen(secret) <= TOTP_SECRET_LENGTH)
	{
		secret_ = secret;
	}
	else
	{
		secret_.clear();
	}
}

const char* TOTPExtension::getSecret() const
{
	return secret_.c_str();
}

int TOTPExtension::getFailedAttempts() const
{
	return failedAttempts_;
}

void TOTPExtension::incrementFailedAttempts()
{
	failedAttempts_++;
}

void TOTPExtension::resetFailedAttempts()
{
	failedAttempts_ = 0;
}

TimePoint TOTPExtension::getLastAttempt() const
{
	return lastAttempt_;
}

void TOTPExtension::setLastAttempt(TimePoint time)
{
	lastAttempt_ = time;
}

void TOTPExtension::freeExtension()
{
	delete this;
}

void TOTPExtension::reset()
{
	verified_ = false;
	failedAttempts_ = 0;
	lastAttempt_ = TimePoint::min();
}
