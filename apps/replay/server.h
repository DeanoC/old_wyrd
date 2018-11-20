#pragma once
#ifndef WYRD_PPVIEWER_SERVER_H
#define WYRD_PPVIEWER_SERVER_H

#include "core/core.h"
#include <thread>

namespace Net { class TcpSimpleServer; struct BasicPayload; }
namespace Timing { class TickerClock; }

namespace Replay { class Replay; }

class Server
{
public:
	Server(std::shared_ptr<Replay::Replay> const& replay_);
	~Server();

protected:
	auto connection(Net::BasicPayload const& payload_) -> bool;

	std::thread serverThread;
	std::shared_ptr<Net::TcpSimpleServer> server;
	std::shared_ptr<Replay::Replay> replay;
	std::unique_ptr<Timing::TickerClock> tickerClock;
};


#endif //WYRD_PPVIEWER_SERVER_H
