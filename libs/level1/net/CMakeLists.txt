set( NET_SRC
		tcpconnection.cpp
		tcpconnection.h tcpsimpleserver.cpp tcpsimpleserver.h context.h context.cpp)

add_definitions(${wyrd_DEFINITIONS})
include_directories( ${wyrd_INCLUDES} )

add_library( net STATIC ${NET_SRC} )
target_link_libraries(net asio)