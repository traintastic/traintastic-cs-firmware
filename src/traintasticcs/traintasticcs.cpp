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

#include "traintasticcs.hpp"

#include <cstring>
#include <pico/stdlib.h>
#include <hardware/uart.h>

#include "../config.hpp"
#include "messages.hpp"

static constexpr uint32_t baudrate = 115'200;
static uint8_t g_rxBuffer[2 + 255 + 1];
static uint8_t g_rxCount = 0;

namespace TraintasticCS
{

static void received();

void init()
{
  uart_init(TRAINTASTIC_CS_UART, baudrate);
  gpio_set_function(TRAINTASTIC_CS_PIN_TX, GPIO_FUNC_UART);
  gpio_set_function(TRAINTASTIC_CS_PIN_RX, GPIO_FUNC_UART);

  uart_getc(TRAINTASTIC_CS_UART); // FIXME: why do we receive 0xFF at startup ??
}

void process()
{
  while(uart_is_readable(TRAINTASTIC_CS_UART))
  {
    g_rxBuffer[g_rxCount] = uart_getc(TRAINTASTIC_CS_UART);
    g_rxCount++;

    while(g_rxCount >= 2 && g_rxCount == (2 + g_rxBuffer[1] + 1))
    {
      if(isChecksumValid(*reinterpret_cast<const Message*>(g_rxBuffer))) /*[[likely]]*/
      {
        received();
        g_rxCount = 0;
        break;
      }
      else // drop one byte
      {
        g_rxCount--;
        std::memmove(g_rxBuffer, g_rxBuffer + 1, g_rxCount);
      }
    }
  }
}

static void send(const Message& message)
{
  const auto* p = reinterpret_cast<const uint8_t*>(&message);
  const uint8_t* end = p + message.size();
  for(; p < end; ++p)
  {
    uart_putc_raw(TRAINTASTIC_CS_UART, *p);
  }
}

static void received()
{
  const auto& message = *reinterpret_cast<const Message*>(g_rxBuffer);

  switch(message.command)
  {
    case Command::Ping:
      send(Pong());
      break;

    case Command::GetInfo:
      send(Info(Board::TraintasticCS, 0, 1, 0));
      break;
  }
}

namespace Throttle
{
  void emergencyStop(Channel channel, uint16_t throttleId, uint16_t address)
  {
    // for now, just sent it to the host
    ThrottleSetSpeedDirection message(channel, throttleId, address);
    message.eStop = 1;
    message.checksum = calcChecksum(message);
    send(message);
  }

  void setSpeedAndDirection(Channel channel, uint16_t throttleId, uint16_t address, bool eStop, uint8_t speedStep, uint8_t speedSteps, Direction direction)
  {
    // for now, just sent it to the host
    ThrottleSetSpeedDirection message(channel, throttleId, address);
    message.eStop = eStop ? 1 : 0;
    message.speedStep = speedStep;
    message.speedSteps = speedSteps;
    message.setSpeedStep = 1;
    message.direction = direction == Direction::Forward ? 1 : 0;
    message.setDirection = 1;
    message.checksum = calcChecksum(message);
    send(message);
  }

  void setFunctions(Channel channel, uint16_t throttleId, uint16_t address, std::initializer_list<std::pair<uint8_t, bool>> values)
  {
    // for now, just sent it to the host
    uint8_t buffer[sizeof(ThrottleSetFunctions) + values.size()];
    auto* message = reinterpret_cast<ThrottleSetFunctions*>(buffer);
    message->command = Command::ThrottleSetFunctions;
    message->length = sizeof(ThrottleSetFunctions) + values.size() - sizeof(Message) - sizeof(Checksum);
    message->channel = channel;
    message->setThrottleId(throttleId);
    message->setAddress(address);
    uint8_t index = 0;
    for(auto& v : values)
    {
      message->setFunction(index++, v.first, v.second);
    }
    updateChecksum(*message);
    send(*message);
  }
}

}
