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

/// logger.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CENTRIFUGEPP__LOGGER__H
#define __CENTRIFUGEPP__LOGGER__H


#include <iostream>
#include <chrono>
#include <thread>


namespace as {

#define AS_LOG( a_stream, a_categoryName, a_m )                                                                        \
	a_stream << std::chrono::duration_cast<std::chrono::microseconds>(                                                 \
					std::chrono::system_clock::now().time_since_epoch() )                                              \
					.count()                                                                                           \
			 << " - " << std::this_thread::get_id() << " [" << a_categoryName << "] " << __FILE__ << AS_T( ", " )      \
			 << __func__ << AS_T( ':' ) << __LINE__ << ":: " << a_m

#if 0
#define AS_LOG_TRACE( m )
#define AS_LOG_DEBUG( m )
#else
#define AS_LOG_DEBUG( a_m ) AS_LOG( std::cout, "DBG", a_m )
#define AS_LOG_TRACE( a_m ) AS_LOG( std::cout, "TRC", a_m )
#endif

#define AS_LOG_INFO( a_m ) AS_LOG( std::cout, "INF", a_m )
#define AS_LOG_WARN( a_m ) AS_LOG( std::cerr, "WRN", a_m )
#define AS_LOG_ERROR( a_m ) AS_LOG( std::cerr, "ERR", a_m )

#define AS_LOG_TRACE_LINE( a_m ) AS_LOG_TRACE( a_m << std::endl )
#define AS_LOG_DEBUG_LINE( a_m ) AS_LOG_DEBUG( a_m << std::endl )
#define AS_LOG_INFO_LINE( a_m ) AS_LOG_INFO( a_m << std::endl )
#define AS_LOG_WARN_LINE( a_m ) AS_LOG_WARN( a_m << std::endl )
#define AS_LOG_ERROR_LINE( a_m ) AS_LOG_ERROR( a_m << std::endl )

} // namespace as


#endif
