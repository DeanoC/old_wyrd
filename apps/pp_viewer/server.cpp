
#include "core/core.h"
#include "server.h"
#include "net/tcpsimpleserver.h"
#include "net/tcpconnection.h"
#include "net/basicpayload.h"

namespace
{
auto connection(Net::BasicPayload const& payload_) -> bool
{
	using namespace std::literals;
	using namespace Net;

	if(payload_.type == "STOP"_basic_payload_type) return false;

	assert(payload_.size < 255u);
	assert(payload_.type == "TEST"_basic_payload_type);
	LOG_S(INFO)<< (char const*)payload_.getPayload();

	return true;
}

}

Server::Server()
{
	server = std::make_shared<Net::TcpSimpleServer>(6666, &connection);

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

