set( NET_SRC
		tcpconnection.h
		tcpsimpleserver.cpp
		tcpsimpleserver.h
		tcpresolver.cpp
		tcpresolver.h
		details/context.h
		details/context.cpp
		details/tcpconnectionimpl.h
		details/tcpsimpleserverimpl.cpp
		details/tcpsimpleserverimpl.h
		details/tcpresolverimpl.h basicpayload.h details/tcpconnectionimpl.cpp)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES} )

add_library( net STATIC ${NET_SRC} )
target_link_libraries(net asio)