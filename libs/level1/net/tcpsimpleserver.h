#pragma once

#ifndef WYRD_NET_TCPSIMPLESERVER_H
#define WYRD_NET_TCPSIMPLESERVER_H

#include "core/core.h"
#include "net/tcpconnection.h"
#include <functional>
#include <atomic>

namespace Net {

// simple server doesn't do anything but call the callback when its gets
// a client, so the function callback allows a limited form of cooperative
// thread sharing. The callback is called repeatly whilst it returns false
// between each call it will call cooperatively yield, the counter passed in
// starts at 0 and increments at every call
class TcpSimpleServer
{
public:
	using ConnectionFunc = std::function<bool (int counter_, std::shared_ptr<TcpConnection>)>;

	TcpSimpleServer(uint16_t port_,
					ConnectionFunc connectionFunc_);
	~TcpSimpleServer();

	// note will loop here until stop() so probably wants its own thread
	auto start() -> void;

	auto stop() -> void;

private:
	struct TcpSimpleServerImpl* impl;
};

}

#endif //WYRD_NET_TCPSIMPLESERVER_H
