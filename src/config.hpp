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

#ifndef CONFIG_HPP
#define CONFIG_HPP

#define TRAINTASTIC_CS_PIN_RX 0
#define TRAINTASTIC_CS_PIN_TX 1
#define TRAINTASTIC_CS_UART uart0

//#define S88_PIN_POWER
#define S88_PIN_DATA 10
#define S88_PIN_CLOCK 11
#define S88_PIN_LOAD 12
#define S88_PIN_RESET 13
#define S88_PIO pio0
#define S88_SM 2

#define XPRESSNET_PIN_POWER PICO_DEFAULT_LED_PIN // LED for testing now
#define XPRESSNET_PIN_RX 14
#define XPRESSNET_PIN_TX 15
#define XPRESSNET_PIN_TX_EN 16
#define XPRESSNET_PIO pio0
#define XPRESSNET_SM_RX 0
#define XPRESSNET_SM_TX 1

#endif
