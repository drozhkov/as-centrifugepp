#include <iostream>
#include <string_view>

#include "centrifugepp/centrifugeClient.hpp"


int main( int argc, char * argv[] )
{
	if ( argc < 2 ) {
		std::clog << "usage: " << argv[0] << " <api-key>" << std::endl;
		return 1;
	}

	try {
		as::CentrifugeClient client(
			"wss://ws.lis-skins.com/connection/websocket",
			[&argv] { return argv[1]; },
			[]( auto & client ) { client.subscribe( "public:obtained-skins" ); },
			[]( auto & client, const std::string_view channelName, const std::string_view data ) {
				AS_LOG_INFO_LINE( data );
			} );

		client.run();
	}
	catch ( const std::exception & x ) {
		std::cerr << x.what() << std::endl;
	}

	return 0;
}
