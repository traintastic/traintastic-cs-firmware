/**
 * This file is part of the Traintastic CS RP2040 firmware,
 * see <https://github.com/traintastic/traintastic-cs-rp2040>.
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

#ifndef UTILS_BYTE_HPP
#define UTILS_BYTE_HPP

#include <cstdint>

constexpr uint8_t high8(const uint16_t value)
{
  return static_cast<uint8_t>(value >> 8);
}

constexpr uint8_t low8(const uint16_t value)
{
  return static_cast<uint8_t>(value & 0xFF);
}

constexpr uint16_t to16(const uint8_t valueLow, const uint8_t valueHigh)
{
  return (static_cast<uint16_t>(valueHigh) << 8) | valueLow;
}

#endif
