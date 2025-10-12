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

using namespace Impl;

class TOTPExtension final
	: public ITOTPExtension
{
private:
	bool enabled_;
	bool verified_;
	char secret_[TOTP_SECRET_LENGTH + 1];
	int failedAttempts_;
	TimePoint lastAttempt_;

public:
	TOTPExtension()
		: enabled_(false)
		, verified_(false)
		, failedAttempts_(0)
		, lastAttempt_(TimePoint::min())
	{
		secret_[0] = '\0';
	}

	bool isEnabled() const override;

	bool isVerified() const override;

	void setEnabled(bool enabled) override;

	void setVerified(bool verified) override;

	bool hasSecret() const override;

	void setSecret(const char* secret) override;

	const char* getSecret() const override;

	int getFailedAttempts() const override;

	void incrementFailedAttempts() override;

	void resetFailedAttempts() override;

	TimePoint getLastAttempt() const override;

	void setLastAttempt(TimePoint time) override;

	void freeExtension() override;

	void reset() override;
};
