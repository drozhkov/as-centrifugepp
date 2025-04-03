#
cmake_minimum_required (VERSION 3.12)


#
project ("centrifugepp-demo")


#
##
set(LIBS
	PRIVATE centrifugepp
)

##
set(LIBS
	${LIBS}
	PRIVATE Boost::beast
	PRIVATE OpenSSL::SSL
	PRIVATE OpenSSL::Crypto
	PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite
)


#
add_executable(${PROJECT_NAME} 
	centrifugepp-demo.cpp
)


#
target_link_libraries(${PROJECT_NAME} ${LIBS})


#
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)


#
install(TARGETS ${PROJECT_NAME} DESTINATION ./bin)
