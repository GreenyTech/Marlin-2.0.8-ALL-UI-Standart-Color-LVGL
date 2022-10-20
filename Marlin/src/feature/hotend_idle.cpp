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
 * Hotend Idle Timeout
 * Prevent filament in the nozzle from charring and causing a critical jam.
 */

#include "../inc/MarlinConfig.h"

#if ENABLED(HOTEND_IDLE_TIMEOUT)

#include "hotend_idle.h"
#include "../gcode/gcode.h"

#include "../module/temperature.h"
#include "../module/motion.h"
#include "../module/planner.h"
#include "../lcd/marlinui.h"

#include "hotend_idle_callback.h"

extern HotendIdleProtection hotend_idle;

millis_t HotendIdleProtection::next_protect_ms = 0;

void HotendIdleProtection::check_hotends(const millis_t &ms) {
  bool do_prot = false;
  
  const bool busy = (TERN0(HAS_RESUME_CONTINUE, wait_for_user) || planner.has_blocks_queued());
  if(!busy){
    HOTEND_LOOP() {
      if (thermalManager.degHotend(e) >= (HOTEND_IDLE_MIN_TRIGGER)) {
        do_prot = true; break;
      }
    }

    //check Bed
    if (thermalManager.degBed() >= (BED_IDLE_MIN_TRIGGER)) {
      //todo hier könnte geprüft werden ob die Zieltemperatur gerade erreicht wurde
      
      //if(thermalManager.wait_for_hotend)
        do_prot = true;
    }


  } 
  // Check differenc to target temperature
  //thermalManager.degTargetBed


  if (bool(next_protect_ms) != do_prot)
    next_protect_ms = do_prot ? ms + hp_interval : 0;
}

void HotendIdleProtection::check_e_motion(const millis_t &ms) {
  static float old_e_position = 0;
  if (old_e_position != current_position.e) {
    old_e_position = current_position.e;          // Track filament motion
    if (next_protect_ms)                          // If some heater is on then...
      next_protect_ms = ms + hp_interval;         // ...delay the timeout till later
  }
}

void HotendIdleProtection::check_user_interaction_motion(const millis_t &ms) {
  
  extern int total_number_of_menu_clicks;
  static int old_total_number_of_menu_clicks;
  if (old_total_number_of_menu_clicks != total_number_of_menu_clicks) {
    old_total_number_of_menu_clicks = total_number_of_menu_clicks;          // Track filament motion
    SERIAL_ECHO_MSG("total number of userclickes inceased");
    if (next_protect_ms)                          // If some heater is on then...
      next_protect_ms = ms + hp_interval;         // ...delay the timeout till later
  }
}

#if ENABLED(HOTEND_IDLE_TIMEOUT_PREVENTION_BY_XYZ_MOVMENT)
void HotendIdleProtection::check_xyz_motion(const millis_t &ms) {
  static float old_x_position = 0;
  static float old_y_position = 0;
  static float old_z_position = 0;
  if (old_x_position != current_position.x) {
    old_x_position = current_position.x;          // Track filament motion
    if (next_protect_ms)                          // If some heater is on then...
      next_protect_ms = ms + hp_interval;         // ...delay the timeout till later
  }
  if (old_y_position != current_position.y) {
    old_y_position = current_position.y;          // Track filament motion
    if (next_protect_ms)                          // If some heater is on then...
      next_protect_ms = ms + hp_interval;         // ...delay the timeout till later
  }
  if (old_z_position != current_position.z) {
    old_z_position = current_position.z;          // Track filament motion
    if (next_protect_ms)                          // If some heater is on then...
      next_protect_ms = ms + hp_interval;         // ...delay the timeout till later
  }
}
#endif


void HotendIdleProtection::check_critical_section(const millis_t &ms) {
  if (critical_section_that_prevents_temperature_timeout) {
    if (next_protect_ms)                          // If some heater is on then...
      next_protect_ms = ms + hp_interval;         // ...delay the timeout till later
  }
}




void HotendIdleProtection::check() {
  const millis_t ms = millis();                   // Shared millis

  check_hotends(ms);                              // Any hotends need protection?



  check_e_motion(ms);                             // Motion will protect them

  check_user_interaction_motion(ms);

#if ENABLED(HOTEND_IDLE_TIMEOUT_PREVENTION_BY_XYZ_MOVMENT)
  check_xyz_motion(ms);
#endif

  check_critical_section(ms);






  // Hot and not moving for too long...
  if (next_protect_ms && ELAPSED(ms, next_protect_ms))
    timed_out();
}



  
// Lower (but don't raise) hotend / bed temperatures
void HotendIdleProtection::timed_out() {
  next_protect_ms = 0;
  SERIAL_ECHOLNPGM("reset timeout");
  //next_protect_ms = 0;
  SERIAL_ECHOLNPGM("Hotend Idle Timeout");
  //ui.return_to_status();

  //TODO do not timeout

  if(temperature_timeout_call_Back!=0){
      temperature_timeout_call_Back();
      temperature_timeout_call_Back=0;
    }
    printingIsActive()?SERIAL_ECHOLNPAIR("is Printing"):SERIAL_ECHOLNPAIR("printingIsActive -> false");
    printingIsPaused()?SERIAL_ECHOLNPAIR("printing is paused"):SERIAL_ECHOLNPAIR("printingIsPaused -> is false");
    //printer_busy()?SERIAL_ECHOLNPAIR("printer_busy"):SERIAL_ECHOLNPAIR("printer_busy -> is false");

    ui.clear_lcd();
    ui.status_screen();


  LCD_MESSAGEPGM(MSG_HOTEND_IDLE_TIMEOUT);
  HOTEND_LOOP() {
    if ((HOTEND_IDLE_NOZZLE_TARGET) < thermalManager.degTargetHotend(e))
      thermalManager.setTargetHotend(HOTEND_IDLE_NOZZLE_TARGET, e);
  }
  #if HAS_HEATED_BED
    if ((HOTEND_IDLE_BED_TARGET) < thermalManager.degTargetBed())
      thermalManager.setTargetBed(HOTEND_IDLE_BED_TARGET);
  #endif
  
}

#endif // HOTEND_IDLE_TIMEOUT
