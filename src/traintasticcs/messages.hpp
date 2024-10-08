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

#ifndef TRAINTASTICCS_MESSAGES_HPP
#define TRAINTASTICCS_MESSAGES_HPP

#include <cstddef>
#include <cstdint>
#include "../utils/byte.hpp"
#include "types.hpp"
#include "throttle/channel.hpp"

namespace TraintasticCS {

struct Message;
using Checksum = std::byte;

inline Checksum calcChecksum(const Message& message);
inline bool isChecksumValid(const Message& message);

#define FROM_CS 0x80
enum class Command : uint8_t
{
  // Traintastic -> Traintastic CS
  Reset = 0x00,
  Ping = 0x01,
  GetInfo = 0x02,
  InitXpressNet = 0x03,
  InitS88 = 0x04,

  // Traintatic CS -> Traintastic
  ResetOk = FROM_CS | Reset,
  Pong = FROM_CS | Ping,
  Info = FROM_CS | GetInfo,
  InitXpressNetOk = FROM_CS | InitXpressNet,
  InitS88Ok = FROM_CS | InitS88,
  InputStateChanged = FROM_CS | 0x20,
  ThrottleSetSpeedDirection = FROM_CS | 0x30,
  ThrottleSetFunctions = FROM_CS | 0x31,
  Error = FROM_CS | 0x7F
};
#undef FROM_CS

enum class ErrorCode : uint8_t
{
  // don't use zero, reserved for no error
  Unknown = 1,
  InvalidCommand = 2,
  InvalidCommandPayload = 3,
  AlreadyInitialized = 4,
};

struct Message
{
  Command command;
  uint8_t length;

  constexpr Message(Command cmd, uint8_t len)
    : command{cmd}
    , length{len}
  {
  }

  constexpr uint16_t size() const
  {
    return sizeof(Message) + length + 1;
  }
};
static_assert(sizeof(Message) == 2);

struct MessageNoData : Message
{
  Checksum checksum;

  constexpr MessageNoData(Command cmd)
    : Message(cmd, sizeof(MessageNoData) - sizeof(Message) - sizeof(checksum))
    , checksum{static_cast<Checksum>(cmd)}
  {
  }
};
static_assert(sizeof(MessageNoData) == 3);

struct Reset : MessageNoData
{
  constexpr Reset()
    : MessageNoData(Command::Reset)
  {
  }
};

struct ResetOk : MessageNoData
{
  constexpr ResetOk()
    : MessageNoData(Command::ResetOk)
  {
  }
};

struct Ping : MessageNoData
{
  constexpr Ping()
    : MessageNoData(Command::Ping)
  {
  }
};

struct Pong : MessageNoData
{
  constexpr Pong()
    : MessageNoData(Command::Pong)
  {
  }
};

struct GetInfo : MessageNoData
{
  constexpr GetInfo()
    : MessageNoData(Command::GetInfo)
  {
  }
};

enum class Board : uint8_t
{
  TraintasticCS = 1,
};

struct Info : Message
{
  Board board;
  uint8_t versionMajor;
  uint8_t versionMinor;
  uint8_t versionPatch;
  Checksum checksum;

  constexpr Info(Board board_, uint8_t major, uint8_t minor, uint8_t patch)
    : Message(Command::Info, sizeof(Info) - sizeof(Message) - sizeof(checksum))
    , board{board_}
    , versionMajor{major}
    , versionMinor{minor}
    , versionPatch{patch}
    , checksum{static_cast<Checksum>(static_cast<uint8_t>(command) ^ length ^ static_cast<uint8_t>(board) ^ versionMajor ^ versionMinor ^ versionPatch)}
  {
  }
};

struct InitXpressNet : MessageNoData
{
  constexpr InitXpressNet()
    : MessageNoData(Command::InitXpressNet)
  {
  }
};

struct InitXpressNetOk : MessageNoData
{
  constexpr InitXpressNetOk()
    : MessageNoData(Command::InitXpressNetOk)
  {
  }
};

struct InitS88 : Message
{
  uint8_t moduleCount;
  uint8_t clockFrequency;
  Checksum checksum;

  constexpr InitS88(uint8_t moduleCount_, uint8_t clockFrequency_)
    : Message(Command::InitS88, sizeof(InitS88) - sizeof(Message) - sizeof(checksum))
    , moduleCount{moduleCount_}
    , clockFrequency{clockFrequency_}
    , checksum{static_cast<Checksum>(static_cast<uint8_t>(command) ^ length ^ moduleCount ^ clockFrequency)}
  {
  }
};

struct InitS88Ok : MessageNoData
{
  constexpr InitS88Ok()
    : MessageNoData(Command::InitS88Ok)
  {
  }
};

struct InputStateChanged : Message
{
  InputChannel channel;
  uint8_t addressH;
  uint8_t addressL;
  InputState state;
  Checksum checksum;

  constexpr InputStateChanged(InputChannel channel_, uint16_t address_, InputState state_)
    : Message(Command::InputStateChanged, sizeof(InputStateChanged) - sizeof(Message) - sizeof(checksum))
    , channel{channel_}
    , addressH{high8(address_)}
    , addressL{low8(address_)}
    , state{state_}
    , checksum{static_cast<Checksum>(static_cast<uint8_t>(command) ^ length ^ static_cast<uint8_t>(channel) ^ addressH ^ addressL ^ static_cast<uint8_t>(state))}
  {
  }

  uint16_t address() const
  {
    return to16(addressL, addressH);
  }
};

struct ThrottleMessage : Message
{
  Throttle::Channel channel;
  uint8_t throttleIdH;
  uint8_t throttleIdL;
  uint8_t addressH;
  uint8_t addressL;

  ThrottleMessage(Command cmd, uint8_t len, Throttle::Channel channel_, uint16_t throttleId_, uint16_t address_)
    : Message(cmd, len)
    , channel{channel_}
  {
    setThrottleId(throttleId_);
    setAddress(address_);
  }

  uint16_t throttleId() const
  {
    return to16(throttleIdL, throttleIdH);
  }

  void setThrottleId(uint16_t value)
  {
    throttleIdL = low8(value);
    throttleIdH = high8(value);
  }

  uint16_t address() const
  {
    return to16(addressL, addressH);
  }

  void setAddress(uint16_t value)
  {
    addressL = low8(value);
    addressH = high8(value);
  }
};

struct ThrottleSetSpeedDirection : ThrottleMessage
{
  uint8_t direction : 1;
  uint8_t eStop : 1;
  uint8_t setSpeedStep : 1;
  uint8_t setDirection : 1;
  uint8_t : 4;
  uint8_t speedStep;
  uint8_t speedSteps;
  Checksum checksum;

  ThrottleSetSpeedDirection(Throttle::Channel channel_, uint16_t throttleId_, uint16_t address_)
    : ThrottleMessage(Command::ThrottleSetSpeedDirection, sizeof(ThrottleSetSpeedDirection) - sizeof(Message) - sizeof(checksum), channel_, throttleId_, address_)
  {
    direction = 0;
    eStop = 0;
    setSpeedStep = 0;
    setDirection = 0;
    speedStep = 0;
    speedSteps = 0;
    checksum = calcChecksum(*this);
  }
};
static_assert(sizeof(ThrottleSetSpeedDirection) == 11);

struct ThrottleSetFunctions : ThrottleMessage
{
  uint8_t functions[1];

  uint8_t functionCount() const
  {
    return length - (sizeof(ThrottleMessage) - sizeof(Message));
  }

  std::pair<uint8_t, bool> function(uint8_t index) const
  {
    return {functions[index] & 0x7F, (functions[index] & 0x80) != 0};
  }

  void setFunction(uint8_t index, uint8_t number, bool value)
  {
    functions[index] = (number & 0x7F) | (value ? 0x80 : 0x00);
  }
};

struct Error : Message
{
  Command request;
  ErrorCode code;
  Checksum checksum;

  constexpr Error(Command request_, ErrorCode code_)
    : Message(Command::Error, sizeof(Error) - sizeof(Message) - sizeof(checksum))
    , request{request_}
    , code{code_}
    , checksum{static_cast<Checksum>(static_cast<uint8_t>(command) ^ length ^ static_cast<uint8_t>(request) ^ static_cast<uint8_t>(code))}
  {
  }
};
static_assert(sizeof(Error) == 5);

inline Checksum calcChecksum(const Message& message)
{
  uint8_t checksum = static_cast<uint8_t>(message.command) ^ message.length;
  const uint8_t* data = reinterpret_cast<const uint8_t*>(&message) + sizeof(Message);
  for(uint8_t i = 0; i < message.length; ++i)
  {
    checksum ^= data[i];
  }
  return static_cast<Checksum>(checksum);
}

inline bool isChecksumValid(const Message& message)
{
  return calcChecksum(message) == *(reinterpret_cast<const Checksum*>(&message) + message.length + 2);
}

inline void updateChecksum(Message& message)
{
  *(reinterpret_cast<Checksum*>(&message) + message.length + 2) = calcChecksum(message);
}

}

#endif
