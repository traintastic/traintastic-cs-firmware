/**
 * This file is part of the Traintastic CS firmware,
 * see <https://github.com/traintastic/traintastic-cs-firmware>.
 *
 * Copyright (C) 2024 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UTILS_TIME_HPP
#define UTILS_TIME_HPP

#include <pico/types.h>

#ifndef NDEBUG
inline bool operator>=(const absolute_time_t lhs, const absolute_time_t rhs)
{
  return to_us_since_boot(lhs) >= to_us_since_boot(rhs);
}

inline bool operator<(const absolute_time_t lhs, const absolute_time_t rhs)
{
  return to_us_since_boot(lhs) < to_us_since_boot(rhs);
}
#endif

#endif
