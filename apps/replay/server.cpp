
#include "core/core.h"
#include "server.h"
#include "net/tcpsimpleserver.h"
#include "net/tcpconnection.h"
#include "net/basicpayload.h"

#include "timing/tickerclock.h"
#include "replay/replay.h"

auto Server::connection(Net::BasicPayload const& payload_) -> bool
{
	using namespace std::literals;
	using namespace Net;
	using namespace Replay;

	if(payload_.type == "STOP"_basic_payload_type) return false;
	char const* data = (char const*)payload_.getPayload();
	replay->add(ItemType(payload_.type), std::string(data, data+payload_.size));

	return true;
}

Server::Server(std::shared_ptr<Replay::Replay> const& replay_) :
	replay(replay_)
{
	using namespace Net;
	using namespace std::placeholders;

	TcpSimpleServer::ConnectionFunc func = std::bind(&Server::connection, this, _1);
	server = std::make_shared<Net::TcpSimpleServer>(6666, func);

	serverThread = std::thread(
			[](std::shared_ptr<Net::TcpSimpleServer> server){
				server->start();
			}, server);
}

Server::~Server()
{
	server->stop();
	serverThread.join();
}

