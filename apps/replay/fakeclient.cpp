#include "core/core.h"
#include "fakeclient.h"
#include "net/tcpconnection.h"
#include "net/tcpresolver.h"
#include "net/basicpayload.h"
#include "timing/pulsar.h"
#include "replay/items.h"
#include "picojson/picojson.h"

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

	pulsars->add(0.3, [this]
	{
		using namespace Net;
		using namespace Replay::Items;

		static float counter = 0.0f;
		counter += 0.1f;
		if(counter > 1.0f) counter = -1.0f;

		picojson::object log;
		log["text"] = picojson::value("TestText"s);
		log["level"] = picojson::value("Error"s);
		log["position"] =
				picojson::value("[0, " + std::to_string(counter) + ", -1]"s);

		picojson::value o{log};
		std::string logString = o.serialize();
		connection->syncWriteBasicPayload(
				(uint32_t) logString.size() + 1,
				(BasicPayloadType) LogType,
				logString.data());
	});

	pulsars->add(0.5, [this]
	{
		using namespace Net;
		using namespace Replay::Items;

		static bool meshSent = false;
		if(meshSent == false)
		{
			picojson::object mesh;
			mesh["name"] = picojson::value{"TestMesh"s};
			mesh["positioncount"] = picojson::value{3.0};
			mesh["positions"] = picojson::value{ picojson::array{
					picojson::value{0.0}, picojson::value{1.0}, picojson::value{0.0},
					picojson::value{1.0}, picojson::value{1.0}, picojson::value{0.0},
					picojson::value{1.0}, picojson::value{0.0}, picojson::value{0.0},
			}};
			mesh["trianglecount"] = picojson::value{1.0};
			mesh["indices"] = picojson::value{ picojson::array{
					picojson::value{0.0}, picojson::value{1.0}, picojson::value{2.0}
			}};

			picojson::value o{mesh};
			std::string meshString = o.serialize();
			connection->syncWriteBasicPayload(
					(uint32_t) meshString.size() + 1,
					(BasicPayloadType) Replay::Items::SimpleMeshType,
					meshString.data());
			meshSent = true;
		} else
		{
			static double counter = 0.0;
			counter += 0.1;
			if(counter > 1.0) counter = -1.0;

			picojson::object object;
			object["name"] = picojson::value{"Object1"s};
			object["meshname"] = picojson::value{"TestMesh"s};
			object["position"] =
					picojson::value{ picojson::array{
							picojson::value{counter},
							picojson::value{0.0},
							picojson::value{1.0}
					}};

			picojson::value o{object};
			std::string objectString = o.serialize();
			connection->syncWriteBasicPayload(
					(uint32_t) objectString.size() + 1,
					(BasicPayloadType) Replay::Items::MeshObjectType,
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