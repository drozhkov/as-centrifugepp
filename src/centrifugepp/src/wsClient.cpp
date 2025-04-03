/*
 *	Copyright (c) 2025 Denis Rozhkov <denis@rozhkoff.com>
 *	This file is part of as-centrifugepp.
 *
 *	as-centrifugepp is free software: you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or (at your
 *	option) any later version.
 *
 *	as-centrifugepp is distributed in the hope that it will be
 *	useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *	Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License along with
 *	as-centrifugepp. If not, see <https://www.gnu.org/licenses/>.
 */

// SPDX-FileCopyrightText: 2025 Denis Rozhkov <denis@rozhkoff.com>
// SPDX-License-Identifier: GPL-3.0-or-later

/// wsClient.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include "centrifugepp/core.hpp"
#include "centrifugepp/logger.hpp"

#include "centrifugepp/wsClient.hpp"


namespace as {

	void WsClientBase::run()
	{
		m_stream.next_layer().set_verify_mode( boost::asio::ssl::verify_none );

		boost::asio::ip::tcp::resolver resolver( m_io );
		std::string portS = std::to_string( m_url.Port() );
		resolver.async_resolve( m_url.Hostname(),
			portS,
			std::bind( &WsClientBase::OnResolve, this, std::placeholders::_1, std::placeholders::_2 ) );

		refreshLastActivityTs();
		m_isWatchdogActive.test_and_set();

		m_watchdogThread = std::thread( [this] {
			while ( m_isWatchdogActive.test_and_set() ) {
				std::this_thread::sleep_for( std::chrono::milliseconds( m_watchdogTimeoutMs / 2 ) );

				if ( NowTs() - m_lastActivityTs > m_watchdogTimeoutMs ) {
					m_io.stop();

					return;
				}
			}
		} );

		m_io.run();
	}


	void WsClientBase::readAsync()
	{
		m_stream.async_read(
			m_buffer, std::bind( &WsClientBase::OnReadComplete, this, std::placeholders::_1, std::placeholders::_2 ) );
	}


	bool WsClientBase::write( const void * data, size_t size )
	{
		std::lock_guard<std::mutex> lock( m_streamWriteSync );

		boost::system::error_code ec;
		m_stream.write( boost::asio::buffer( data, size ), ec );

		return static_cast<bool>( ec );
	}


	void WsClientBase::writeAsync( const void * data, size_t size )
	{
		std::lock_guard<std::mutex> lock( m_streamWriteSync );

		m_stream.async_write( boost::asio::buffer( data, size ),
			std::bind( &WsClientBase::OnWriteComplete, this, std::placeholders::_1, std::placeholders::_2 ) );
	}


	void WsClientBase::pingAsync( const void * data, size_t size )
	{
		std::lock_guard<std::mutex> lock( m_streamPingSync );

		m_stream.async_ping( { static_cast<const char *>( data ), size },
			std::bind( &WsClientBase::OnPingComplete, this, std::placeholders::_1 ) );
	}

} // namespace as
