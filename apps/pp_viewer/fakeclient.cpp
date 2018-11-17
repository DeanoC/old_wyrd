#include "core/core.h"
#include "fakeclient.h"
#include "net/tcpconnection.h"
#include "net/tcpresolver.h"

FakeClient::FakeClient()
{
	using namespace std::literals;
	Net::TcpResolver resolver;
	connection = resolver.connect("localhost"sv, 6666);
}

FakeClient::~FakeClient()
{
	connection.reset();
}

auto FakeClient::update() -> void
{
}