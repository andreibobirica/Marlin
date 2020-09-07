/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Created by Andrei Bobirica.
 * Copyright (c) 2020 Andrei Bobirica
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

/**
 * feature/relaymultie.h - Dual Extrusion with a Relay Board
 */

#include "../inc/MarlinConfigPre.h"

class RelayMultiE {
  public:
      static bool reverseRELAYMULTIE;
      static bool standbyNozzleRELAYMULTIE;
};

void select_extrusor(const uint8_t e);
void relaymultie_init();

extern RelayMultiE rme;

