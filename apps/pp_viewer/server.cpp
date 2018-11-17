
#include "core/core.h"
#include "net/tcpsimpleserver.h"
#include "server.h"

namespace
{
auto connection(int counter_, std::shared_ptr<Net::TcpConnection> connection_) -> bool
{
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

