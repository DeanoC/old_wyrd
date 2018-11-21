#include "core/core.h"
#include "fakeclient.h"
#include "net/tcpconnection.h"
#include "net/tcpresolver.h"
#include "net/basicpayload.h"
#include "timing/pulsar.h"
#include "nlohmann/json.h"
#include "replay/items.h"

FakeClient::FakeClient()
{
	using namespace std::literals;
	Net::TcpResolver resolver;
	connection = resolver.connect("localhost"sv, 6666);

	pulsars = std::make_unique<Timing::Pulsars>();
	pulsars->add(1.0, [this]
	{
		using namespace Net;
		auto testString = "Testing";
		connection->syncWriteBasicPayload((uint32_t) (strlen(testString) + 1),
										  "TEST"_basic_payload_type,
										  testString);
	});

/*	pulsars->add(0.5, [this]
	{
		using namespace Net;
		using namespace nlohmann;
		using namespace Replay::Items;

		static float counter = 0.0f;
		counter += 0.1f;
		json log;
		log["text"] = "TestText";
		log["level"] = "Error";
		log["position"] = "[0, " + std::to_string(counter) + ", 1]";
		std::string logString = log.dump();
		connection->syncWriteBasicPayload(
			(uint32_t)logString.size()+1,
			(BasicPayloadType)LogType,
			logString.data());
	});*/

	pulsars->add(0.5, [this]
	{
		using namespace Net;
		using namespace nlohmann;
		using namespace Replay::Items;

		static bool meshSent = false;
		if(meshSent == false)
		{
			json mesh;
			mesh["name"] = "TestMesh";
			mesh["positioncount"] = 3;
			mesh["positions"] = {
					0.0f, 1.0f, 0.0f,
					1.0f, 1.0f, 0.0f,
					1.0f, 0.0f, 0.0f,
						};
			mesh["trianglecount"] = 1;
			mesh["indices"] = {0, 1, 2};

			std::string meshString = mesh.dump();
			connection->syncWriteBasicPayload(
							(uint32_t)meshString.size()+1,
							(BasicPayloadType)Replay::Items::SimpleMeshType,
							meshString.data());
			meshSent = true;
		} else
		{
			json object;
			object["name"] = "Object1";
			object["meshname"] = "TestMesh";
			object["position"] = { 1.0f, 1.0f, 1.0f };

			std::string objectString = object.dump();
			connection->syncWriteBasicPayload(
					(uint32_t)objectString.size()+1,
					(BasicPayloadType)Replay::Items::MeshObjectType,
					objectString.data());

		}

	});
}

FakeClient::~FakeClient()
{
	using namespace Net;
	connection->syncWriteBasicPayload(0, "STOP"_basic_payload_type, nullptr);
	connection.reset();
}

auto FakeClient::update() -> void
{
	pulsars->update();
}