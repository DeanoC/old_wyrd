#pragma once
#ifndef WYRD_FAKECLIENT_H
#define WYRD_FAKECLIENT_H

#include "core/core.h"

namespace Net { class TcpConnection; }
namespace Timing { class Pulsar; }

class FakeClient
{
public:
	FakeClient();
	~FakeClient();

	auto update() -> void;

private:
	std::unique_ptr<Timing::Pulsar> pulsar;
	std::unique_ptr<Net::TcpConnection> connection;
};


#endif //WYRD_FAKECLIENT_H
