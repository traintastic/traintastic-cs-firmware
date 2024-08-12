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

#include <pico/stdlib.h>
#include <pico/binary_info.h>

#include "config.hpp"
#include "traintasticcs/traintasticcs.hpp"
#include "xpressnet/xpressnet.hpp"

int main()
{
  // Binary info:
  bi_decl(bi_1pin_with_name(TRAINTASTIC_CS_PIN_RX, "Traintastic CS Rx"));
  bi_decl(bi_1pin_with_name(TRAINTASTIC_CS_PIN_TX, "Traintastic CS Tx"));
  bi_decl(bi_1pin_with_name(XPRESSNET_PIN_RX, "XpressNet Rx"));
  bi_decl(bi_1pin_with_name(XPRESSNET_PIN_TX, "XpressNet Tx"));
  bi_decl(bi_1pin_with_name(XPRESSNET_PIN_TX_EN, "XpressNet Tx enable"));

  TraintasticCS::init();
  XpressNet::init();

  for(;;)
  {
    TraintasticCS::process();
    XpressNet::process();
    sleep_ms(5);
  }
}

