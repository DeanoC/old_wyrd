#pragma once
#ifndef WYRD_FAKECLIENT_H
#define WYRD_FAKECLIENT_H

#include "core/core.h"

namespace Net { class TcpConnection; }
namespace Timing { class TickerClock; }

class FakeClient
{
public:
	FakeClient();
	~FakeClient();

	auto update() -> void;

private:
	double timeElapsed;
	std::unique_ptr<Timing::TickerClock> ticker;
	std::unique_ptr<Net::TcpConnection> connection;
};


#endif //WYRD_FAKECLIENT_H
