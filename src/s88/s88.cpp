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

#include "s88.hpp"
#include <pico/stdlib.h>
#include "../config.hpp"
#include "../traintasticcs/input.hpp"
#include "../utils/time.hpp"

#define CLOCK_TICK 100 // us

namespace S88 {

static bool g_enabled = false;
static uint16_t g_inputCount;
static absolute_time_t g_nextScan;

void init()
{
  //gpio_init(S88_PIN_POWER);
  //gpio_set_dir(S88_PIN_POWER, GPIO_OUT);

  gpio_init(S88_PIN_RESET);
  gpio_set_dir(S88_PIN_RESET, GPIO_OUT);

  gpio_init(S88_PIN_LOAD);
  gpio_set_dir(S88_PIN_LOAD, GPIO_OUT);

  gpio_init(S88_PIN_DATA);
  gpio_set_dir(S88_PIN_DATA, GPIO_IN);

  gpio_init(S88_PIN_CLOCK);
  gpio_set_dir(S88_PIN_CLOCK, GPIO_OUT);
  gpio_put(S88_PIN_CLOCK, 1);
}

bool enabled()
{
  return g_enabled;
}

void enable(uint8_t moduleCount)
{
  //gpio_put(S88_PIN_POWER, 1);

  g_inputCount = moduleCount * 8;
  g_enabled = true;
  g_nextScan = make_timeout_time_ms(1000);
}

void disable()
{
  //gpio_put(S88_PIN_POWER, 0);

  g_enabled = false;
}

void process()
{
  if(!g_enabled)
  {
    return;
  }

  if(get_absolute_time() < g_nextScan)
  {
    return;
  }

  // bit bang test, verify if we can read it

  gpio_put(S88_PIN_CLOCK, 0);
  sleep_us(CLOCK_TICK * 2);
  gpio_put(S88_PIN_LOAD, 1);
  sleep_us(CLOCK_TICK);
  gpio_put(S88_PIN_CLOCK, 1);
  sleep_us(CLOCK_TICK);
  gpio_put(S88_PIN_CLOCK, 0);
  sleep_us(CLOCK_TICK);
  gpio_put(S88_PIN_RESET, 1);
  sleep_us(CLOCK_TICK);
  gpio_put(S88_PIN_RESET, 0);
  sleep_us(CLOCK_TICK);
  gpio_put(S88_PIN_LOAD, 0);

  TraintasticCS::Input::updateState(
    TraintasticCS::InputChannel::S88,
    1,
    gpio_get(S88_PIN_DATA) ? TraintasticCS::InputState::High : TraintasticCS::InputState::Low);

  sleep_us(CLOCK_TICK);

  for(uint8_t address = 2; address <= g_inputCount; ++address)
  {
    gpio_put(S88_PIN_CLOCK, 1);
    sleep_us(CLOCK_TICK);

    TraintasticCS::Input::updateState(
      TraintasticCS::InputChannel::S88,
      address,
      gpio_get(S88_PIN_DATA) ? TraintasticCS::InputState::High : TraintasticCS::InputState::Low);

    gpio_put(S88_PIN_CLOCK, 0);
    sleep_us(CLOCK_TICK);
  }

  gpio_put(S88_PIN_CLOCK, 1);

  g_nextScan = make_timeout_time_ms(10);
}

}
