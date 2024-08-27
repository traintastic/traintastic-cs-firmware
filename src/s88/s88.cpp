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
#include "s88.pio.h"
#include <algorithm>
#include <pico/stdlib.h>
#include <hardware/clocks.h>
#include <hardware/pio.h>
#include "../config.hpp"
#include "../traintasticcs/input.hpp"
#include "../utils/time.hpp"

namespace S88 {

static bool g_enabled = false;
static uint16_t g_inputCount;
static absolute_time_t g_nextScan;
static uint g_fifoRead;
static uint g_inputIndex;

#define CLOCK_FREQUENCY 10000

void init()
{
  //gpio_init(S88_PIN_POWER);
  //gpio_set_dir(S88_PIN_POWER, GPIO_OUT);

  // setup pio:
  pio_gpio_init(S88_PIO, S88_PIN_DATA);
  pio_gpio_init(S88_PIO, S88_PIN_CLOCK);
  pio_gpio_init(S88_PIO, S88_PIN_LOAD);
  pio_gpio_init(S88_PIO, S88_PIN_RESET);

  pio_sm_set_consecutive_pindirs(S88_PIO, S88_SM, S88_PIN_CLOCK, 3, true);

  uint offset = pio_add_program(S88_PIO, &s88_program);
  pio_sm_config c = s88_program_get_default_config(offset);

  sm_config_set_set_pins(&c, S88_PIN_CLOCK, 3);
  sm_config_set_sideset_pins(&c, S88_PIN_CLOCK);
  sm_config_set_in_pins(&c, S88_PIN_DATA);

  sm_config_set_out_shift(&c, true, true, 32); // right shift, autopush

  float div = (float)clock_get_hz(clk_sys) / (4 * CLOCK_FREQUENCY);
  sm_config_set_clkdiv(&c, div);

  pio_sm_init(S88_PIO, S88_SM, offset, &c);
}

bool enabled()
{
  return g_enabled;
}

void enable(uint8_t moduleCount)
{
  //gpio_put(S88_PIN_POWER, 1);

  pio_sm_set_enabled(S88_PIO, S88_SM, false);

  pio_sm_clear_fifos(S88_PIO, S88_SM);

  pio_sm_restart(S88_PIO, S88_SM);

  pio_sm_set_enabled(S88_PIO, S88_SM, true);

  g_inputCount = moduleCount * 8;
  g_enabled = true;
  g_nextScan = make_timeout_time_ms(1000);
  g_fifoRead = 0;
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

  constexpr uint wordSize = 32;

  while(g_fifoRead != 0 && !pio_sm_is_rx_fifo_empty(S88_PIO, S88_SM))
  {
    auto value = pio_sm_get(S88_PIO, S88_SM);
    uint bitsToRead = std::min(g_inputCount - g_inputIndex, wordSize);

    if(bitsToRead < wordSize)
    {
      // values are shifted in form the right, align them left
      value >>= wordSize - bitsToRead;
    }

    for(;bitsToRead != 0; --bitsToRead)
    {
      TraintasticCS::Input::updateState(
        TraintasticCS::InputChannel::S88,
        1 + g_inputIndex,
        (value & 1) ? TraintasticCS::InputState::High : TraintasticCS::InputState::Low);
      value >>= 1;
      g_inputIndex++;
    }

    g_fifoRead--;
  }

  if(g_fifoRead == 0) // trigger next scan
  {
    pio_sm_put(S88_PIO, S88_SM, g_inputCount - 2);
    g_fifoRead = 1 + (g_inputCount / wordSize); // round up, at multiple of wordSize there is a dummy push
    g_inputIndex = 0;
  }
}

}
