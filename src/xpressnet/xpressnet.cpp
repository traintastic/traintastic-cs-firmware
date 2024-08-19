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

#include "xpressnet.hpp"
#include "xpressnet.pio.h"

#include <pico/stdlib.h> // sleep_us

#include "../config.hpp"
#include "../traintasticcs/traintasticcs.hpp"
#include "../utils/bit.hpp"
#include "../utils/endian.hpp"

namespace XpressNet {

static bool g_enabled = false;
static uint8_t g_address;
static uint8_t g_rxBuffer[32];
static uint8_t g_rxBufferCount;

static void received();

void init()
{
  gpio_init(XPRESSNET_PIN_TX_EN);
  gpio_set_dir(XPRESSNET_PIN_TX_EN, GPIO_OUT);

  gpio_init(XPRESSNET_PIN_POWER);
  gpio_set_dir(XPRESSNET_PIN_POWER, GPIO_OUT);

  xpressnet_rx_program_init(XPRESSNET_PIO, XPRESSNET_SM_RX, XPRESSNET_PIN_RX);
  xpressnet_tx_program_init(XPRESSNET_PIO, XPRESSNET_SM_TX, XPRESSNET_PIN_TX, XPRESSNET_PIN_TX_EN);
}

bool enabled()
{
  return g_enabled;
}

void enable()
{
  g_address = 0;
  g_rxBufferCount = 0;

  pio_sm_set_enabled(XPRESSNET_PIO, XPRESSNET_SM_RX, false);
  pio_sm_set_enabled(XPRESSNET_PIO, XPRESSNET_SM_TX, false);

  pio_sm_clear_fifos(XPRESSNET_PIO, XPRESSNET_SM_RX);
  pio_sm_clear_fifos(XPRESSNET_PIO, XPRESSNET_SM_TX);

  pio_sm_restart(XPRESSNET_PIO, XPRESSNET_SM_RX);
  pio_sm_restart(XPRESSNET_PIO, XPRESSNET_SM_TX);

  pio_sm_set_enabled(XPRESSNET_PIO, XPRESSNET_SM_RX, true);
  pio_sm_set_enabled(XPRESSNET_PIO, XPRESSNET_SM_TX, true);

  gpio_put(XPRESSNET_PIN_POWER, 1);

  g_enabled = true;
}

void disable()
{
  if(!enabled())
  {
    return;
  }

  gpio_put(XPRESSNET_PIN_POWER, 0);

  pio_sm_set_enabled(XPRESSNET_PIO, XPRESSNET_SM_RX, false);
  pio_sm_set_enabled(XPRESSNET_PIO, XPRESSNET_SM_TX, false);

  g_enabled = false;
}

void sendCallByte(uint8_t value)
{
  uint8_t bits = 0;
  for(uint8_t m = 1; m < 0x80; m <<= 1)
  {
    if(value & m)
    {
      bits++;
    }
  }
  if(bits & 1) // odd
  {
    value |= 0x80;
  }

  gpio_put(XPRESSNET_PIN_TX_EN, 1);
  pio_sm_put(XPRESSNET_PIO, XPRESSNET_SM_TX, 0x0100u | value);

  sleep_us(190); // FIXME
  gpio_put(XPRESSNET_PIN_TX_EN, 0); // FIXME
}

void sendNormalInquiry(uint8_t address)
{
  sendCallByte(0x40 | address);
}

void send(uint8_t callByte, const uint8_t* message)
{
  uint8_t bits = 0;
  for(uint8_t m = 1; m < 0x80; m <<= 1)
  {
    if(callByte & m)
    {
      bits++;
    }
  }
  if(bits & 1) // odd
  {
    callByte |= 0x80;
  }

  const uint8_t dataLength = message[0] & 0x0F;
  gpio_put(XPRESSNET_PIN_TX_EN, 1);
  pio_sm_put(XPRESSNET_PIO, XPRESSNET_SM_TX, 0x0100u | callByte);
  uint8_t checksum = 0;
  for(uint8_t i = 0; i <= dataLength; ++i)
  {
    pio_sm_put(XPRESSNET_PIO, XPRESSNET_SM_TX, message[i]);
    checksum ^= message[i];
  }
  pio_sm_put(XPRESSNET_PIO, XPRESSNET_SM_TX, checksum);

  sleep_us(190 * (3 + dataLength)); // FIXME
  gpio_put(XPRESSNET_PIN_TX_EN, 0); // FIXME
}

void process()
{
  if(!g_enabled)
  {
    return;
  }

  while(!pio_sm_is_rx_fifo_empty(XPRESSNET_PIO, XPRESSNET_SM_RX))
  {
    uint16_t value = pio_sm_get(XPRESSNET_PIO, XPRESSNET_SM_RX) >> (32 - 9);
    if((value & 0x100) == 0) // data byte
    {
      g_rxBuffer[g_rxBufferCount] = static_cast<uint8_t>(value);
      g_rxBufferCount++;
    }
    else
    {
      g_rxBufferCount = 0; // reset buffer, should never happen
    }

    const uint8_t length = 2 + (g_rxBuffer[0] & 0x0F);
    if(g_rxBufferCount >= length)
    {
      // calc chcksum:
      uint8_t checksum = g_rxBuffer[0];
      for(uint8_t i = 1; i < length - 1; i++)
        checksum ^= g_rxBuffer[i];

      if(checksum == g_rxBuffer[length - 1])
      {
        received();
        g_rxBufferCount -= length;
      }
      else
      {
        g_rxBufferCount = 0; // reset buffer, checksum invalid
      }
    }
  }

  if(pio_sm_is_tx_fifo_empty(XPRESSNET_PIO, XPRESSNET_SM_TX))
  {
    if(++g_address > 31)
    {
      g_address = 1;
    }
    sendNormalInquiry(g_address);
  }
}

static void received()
{
  const uint8_t* message = g_rxBuffer;
  switch(message[0])
  {
    case 0x21:
      switch(message[1])
      {
        case 0x80: // Stop operations request
        {
          static constexpr uint8_t msg[2] = {0x61, 0x00};
          send(0x60, msg);
          send(0x60, msg);
          send(0x60, msg);
          break;
        }
        case 0x81: // Resume operations request
        {
          static constexpr uint8_t msg[2] = {0x61, 0x01};
          send(0x60, msg);
          send(0x60, msg);
          send(0x60, msg);
          break;
        }
        case 0x24: // Command station status request
        {
          static constexpr uint8_t msg[3] = {0x62, 0x22, 0x00};
          send(0x60 | g_address, msg);
          break;
        }
        case 0x21: // Command station software-version request
        {
          static constexpr uint8_t msg[4] = {0x63, 0x21, 0x30, 0x10};
          send(0x60 | g_address, msg);
          break;
        }
      }
      break;

    case 0x80: // Stop all locomotives request (emergency stop)
    {
      static constexpr uint8_t msg[2] = {0x81, 0x00};
      send(0x60, msg);
      send(0x60, msg);
      send(0x60, msg);
      break;
    }
    case 0xE4:
    {
      switch(message[1])
      {
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        {
          bool eStop = false;
          uint8_t speedStep = 0;
          uint8_t speedSteps = 0;

          switch(message[1])
          {
            case 0x10: // 14 steps
              TraintasticCS::Throttle::setFunction(
                TraintasticCS::Throttle::Channel::XpressNet,
                g_address,
                be16(message + 2),
                0,
                bit<7>(message[4])
              );
              speedSteps = 14;
              speedStep = message[4] & 0x0F;
              if(speedStep == 1)
              {
                eStop = true;
                speedStep = 0;
              }
              else if(speedStep > 1)
              {
                speedStep--;
              }
              break;

            case 0x11:
              speedSteps = 27;
              speedStep = ((message[4] & 0x0F) << 1) | ((message[4] & 0x10) >> 4);
              if(speedStep >= 4 && speedStep < 31)
              {
                speedStep -= 3;
              }
              else if(speedStep != 0) // all unused values are eStop too.
              {
                eStop = true;
                speedStep = 0;
              }
              break;

            case 0x12:
              speedSteps = 28;
              speedStep = ((message[4] & 0x0F) << 1) | ((message[4] & 0x10) >> 4);
              if(speedStep >= 4)
              {
                speedStep -= 3;
              }
              else if(speedStep != 0) // all unused values are eStop too.
              {
                eStop = true;
                speedStep = 0;
              }
              break;

            case 0x13:
              speedSteps = 126;
              speedStep = message[4] & 0x7F;
              if(speedStep == 1)
              {
                eStop = true;
                speedStep = 0;
              }
              else if(speedStep > 1)
              {
                speedStep--;
              }
              break;
          }

          TraintasticCS::Throttle::setSpeedAndDirection(
            TraintasticCS::Throttle::Channel::XpressNet,
            g_address,
            be16(message + 2),
            eStop,
            speedStep,
            speedSteps,
            bit<7>(message[4]) ? TraintasticCS::Direction::Forward : TraintasticCS::Direction::Reverse
          );
          break;
        }
        case 0x20: //  Function instruction group 1
          TraintasticCS::Throttle::setFunctions(
            TraintasticCS::Throttle::Channel::XpressNet,
            g_address,
            be16(message + 2),
            {
              {0, bit<4>(message[4])},
              {1, bit<0>(message[4])},
              {2, bit<1>(message[4])},
              {3, bit<2>(message[4])},
              {4, bit<3>(message[4])},
            }
          );
          break;

        case 0x21: // Function instruction group 2
          TraintasticCS::Throttle::setFunctions(
            TraintasticCS::Throttle::Channel::XpressNet,
            g_address,
            be16(message + 2),
            {
              {5, bit<0>(message[4])},
              {6, bit<1>(message[4])},
              {7, bit<2>(message[4])},
              {8, bit<3>(message[4])},
            }
          );
          break;

        case 0x22: // Function instruction group 3
          TraintasticCS::Throttle::setFunctions(
            TraintasticCS::Throttle::Channel::XpressNet,
            g_address,
            be16(message + 2),
            {
              {9, bit<0>(message[4])},
              {10, bit<1>(message[4])},
              {11, bit<2>(message[4])},
              {12, bit<3>(message[4])},
            }
          );
          break;

        case 0xF3: // Roco Multimaus F13-F20
          TraintasticCS::Throttle::setFunctions(
            TraintasticCS::Throttle::Channel::XpressNet,
            g_address,
            be16(g_rxBuffer + 2),
            {
              {13, bit<0>(message[4])},
              {14, bit<1>(message[4])},
              {15, bit<2>(message[4])},
              {16, bit<3>(message[4])},
              {17, bit<4>(message[4])},
              {18, bit<5>(message[4])},
              {19, bit<6>(message[4])},
              {20, bit<7>(message[4])},
            }
          );
          break;
      }
    }
  }
}

}
