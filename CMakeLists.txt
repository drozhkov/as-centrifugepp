#
cmake_minimum_required(VERSION 3.16)


#
project("centrifugepp")


#
if (MSVC)
	add_compile_options(-D_WIN32_WINNT=0x0601)
endif()


#
##
include_directories("include")

##
find_package(boost_beast CONFIG REQUIRED)

##
find_package(OpenSSL REQUIRED)

##
find_package(Protobuf CONFIG REQUIRED)


#
##
add_subdirectory("src/centrifugepp")

##
add_subdirectory("src/centrifugepp-demo")
