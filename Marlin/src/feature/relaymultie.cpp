/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
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

/**
 * feature/powerloss.cpp - Resume an SD print after power-loss
 */
#include "../inc/MarlinConfigPre.h"

#if ENABLED(RELAYMULTIE)

    #ifndef RELAYMULTIE_PIN
        #define RELAYMULTIE_PIN PD13
    #endif

    #include "../module/stepper.h"
    #include "relaymultie.h"
    //Revert the state of the Relay High 5 V, Low 0 volt OR High 0 V, Low 5 V
    bool RelayMultiE::reverseRELAYMULTIE = false;
    //Put the automatic Temperature and Ventilation system OFF or ON, use a standby nozzle 
    bool RelayMultiE::standbyNozzleRELAYMULTIE = false;

    //Select the desired Extrusor
    void select_extrusor(const uint8_t e) {
        //Choose the state
        bool state = rme.reverseRELAYMULTIE;
        planner.synchronize();
        disable_e_steppers();
        //Action Relay
        const pin_t pin = GET_PIN_MAP_PIN(RELAYMULTIE_PIN);
        if (pin_is_protected(pin)) return protected_pin_err();//stop if pin protected
        safe_delay(100);//Wait for precaution

        pinMode(pin, OUTPUT);//set pin to output
        SERIAL_ECHOLNPAIR("Asked for E: ", e);
        SERIAL_ECHOLNPAIR("Relay State was: ", state);

        if(e == (uint8_t)1){
            extDigitalWrite(pin, !state);
            SERIAL_ECHOLNPGM("E1 activated");
        }

        if(e == (uint8_t)0){
            extDigitalWrite(pin, state);
            SERIAL_ECHOLNPGM("E0 activated");
        }
        //safe delay to active the relay
        safe_delay(500);
    }

    //startup init
    void relaymultie_init(){
        //RELAY State
        bool state = RelayMultiE::reverseRELAYMULTIE;
        planner.synchronize();
        disable_e_steppers();
        const pin_t pin = GET_PIN_MAP_PIN(RELAYMULTIE_PIN);//PIN relay Digitale
        if (pin_is_protected(pin)) return protected_pin_err();//stop if pin protected
        safe_delay(200);//Wait 
        pinMode(pin, OUTPUT);//set pin to output
        extDigitalWrite(pin, state);
        SERIAL_ECHOLNPAIR("Relay Reversed Logic checked: ", state);
    }

#endif // RELAYMULTIE
