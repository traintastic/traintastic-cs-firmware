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

#include "input.hpp"
#include <algorithm>
#include <array>
#include "messages.hpp"
#include "../s88/s88.hpp"

namespace TraintasticCS { void send(const Message& message); } // FIXME

namespace TraintasticCS::Input {

static std::array<InputState, 8 * S88::moduleCountMax> g_s88;

struct InputStates
{
  InputState* data;
  size_t size = 0;
};

static InputStates getStates(InputChannel channel)
{
  switch(channel)
  {
    case InputChannel::S88:
      return {g_s88.data(), g_s88.size()};
  }
  return {nullptr, 0};
}

void enable()
{
  std::fill(g_s88.begin(), g_s88.end(), InputState::Unknown);
}

bool getState(InputChannel channel, uint16_t address, InputState& state)
{
  const auto states = getStates(channel);

  if(address >= 1 && address <= states.size) /*[[likely]]*/
  {
    state = states.data[address - 1];
    return true;
  }

  return false;
}

void updateState(InputChannel channel, uint16_t address, InputState state)
{
  auto states = getStates(channel);

  if(address >= 1 && address <= states.size && states.data[address - 1] != state)
  {
    states.data[address - 1] = state;
    send(InputStateChanged(channel, address, state));
  }
}

}
