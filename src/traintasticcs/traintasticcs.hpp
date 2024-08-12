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

#ifndef TRAINTASTICCS_TRAINTASTICCS_HPP
#define TRAINTASTICCS_TRAINTASTICCS_HPP

#include <cstdint>
#include <initializer_list>
#include <utility>

#include "direction.hpp"
#include "throttle/channel.hpp"

namespace TraintasticCS
{

void init();
void process();

namespace Throttle
{
  void emergencyStop(Channel channel, uint16_t throttleId, uint16_t address);
  void setSpeedAndDirection(Channel channel, uint16_t throttleId, uint16_t address, bool eStop, uint8_t speedStep, uint8_t speedSteps, Direction direction);
  void setFunctions(Channel channel, uint16_t throttleId, uint16_t address, std::initializer_list<std::pair<uint8_t, bool>> values);

  inline void setFunction(Channel channel, uint16_t throttleId, uint16_t address, uint8_t number, bool value)
  {
    setFunctions(channel, throttleId, address, {{number, value}});
  }
}

}

#endif
