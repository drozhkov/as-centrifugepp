#
cmake_minimum_required (VERSION 3.12)


#
##
include_directories("src")

##
set(LIBS
	PRIVATE Boost::beast
	PRIVATE OpenSSL::SSL
	PRIVATE OpenSSL::Crypto
)


#
add_library(${PROJECT_NAME} 
	src/protocol/client.pb.cc
	src/wsClient.cpp
	src/centrifugeClient.cpp
)


#
set_target_properties(${PROJECT_NAME} PROPERTIES 
	CXX_STANDARD 20
)


#
target_link_libraries(${PROJECT_NAME} ${LIBS})
