#include "core/core.h"
#include "fakeclient.h"
#include "net/tcpconnection.h"
#include "net/tcpresolver.h"
#include "net/basicpayload.h"

FakeClient::FakeClient()
{
	using namespace std::literals;
	Net::TcpResolver resolver;
	connection = resolver.connect("localhost"sv, 6666);
}

FakeClient::~FakeClient()
{
	using namespace Net;
	connection->syncWriteBasicPayload(0,"STOP"_basic_payload_type, nullptr);
	connection.reset();
}

auto FakeClient::update() -> void
{
	using namespace Net;
	auto testString = "Testing123";
	connection->syncWriteBasicPayload((uint32_t)(strlen(testString) + 1),"TEST"_basic_payload_type, testString);
}