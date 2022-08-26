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

//
// Calibrate Probe offset menu.
//

#include "../../inc/MarlinConfigPre.h"

#if ENABLED(PROBE_OFFSET_WIZARD)

#include "menu_item.h"
#include "menu_addon.h"
#include "../../gcode/queue.h"
#include "../../module/motion.h"
//#include "../../module/temperature.h"
#include "../../module/planner.h"
#include "../../module/probe.h"
//#include "menu.h"

#include "../../feature/hotend_idle_callback.h"

#if HAS_LEVELING
  #include "../../feature/bedlevel/bedlevel.h"
#endif

// Global storage
float z_offset_backup, calculated_z_offset, z_offset_ref;

#if ENABLED(HAS_LEVELING)
  bool leveling_was_active;
#endif

inline void z_clearance_move() {
  do_z_clearance(
    #ifdef Z_AFTER_HOMING
      Z_AFTER_HOMING
    #elif defined(Z_HOMING_HEIGHT)
      Z_HOMING_HEIGHT
    #else
      10
    #endif
  );
}
inline void clear_temperature(){
  
    
    
    queue.inject_P(PSTR(CLEAR_PREHEAT_GCODE_BEVORE_MEASUREMENT)); //dont wait. Set bad temperatur 0
    //queue.inject_P(PSTR("M104 S0")); //dont wait set nozzle temp. to 0

}


void set_offset_and_end(const_float_t z) {
  probe.offset.z = z;
  SET_SOFT_ENDSTOP_LOOSE(false);
  TERN_(HAS_LEVELING, set_bed_leveling_enabled(leveling_was_active));
  
  
}

void _goto_manual_move_z(const_float_t scale) {
  ui.manual_move.menu_scale = scale;
  ui.goto_screen(lcd_move_z);
}

inline void cancel_z_probe_offset(){
  
  SERIAL_ECHOLNPGM("Cancel z Probe");
  set_offset_and_end(z_offset_backup);
  is_in_z_probe_offset_process =false;
    // If wizard-homing was done by probe with PROBE_OFFSET_WIZARD_START_Z
    #if HOMING_Z_WITH_PROBE && defined(PROBE_OFFSET_WIZARD_START_Z)
      set_axis_never_homed(Z_AXIS); // On cancel the Z position needs correction
      queue.inject_P(PSTR("G28Z"));
    #else // Otherwise do a Z clearance move like after Homing
      z_clearance_move();
      //clear_movement_and_Temperature();
    #endif
    
    clear_temperature();
    //ui.goto_previous_screen_no_defer();

}

void probe_offset_wizard_menu() {

  
    //queue.inject_P(PSTR("M190 S55")); //wait

  //TODO retract
  //thermalManager.setTargetHotend(210, 0);
  //ui.pause_show_message(PAUSE_MESSAGE_HEATING, PAUSE_MODE_SAME);
  
    //const int8_t target_extruder = 0;//get_target_extruder_from_command();
    //ui.pause_show_message(PAUSE_MESSAGE_CHANGING, PAUSE_MODE_PAUSE_PRINT, 0);

    /**const int beep_count = parser.intval('B', -1
    #ifdef FILAMENT_CHANGE_ALERT_BEEPS
      + 1 + FILAMENT_CHANGE_ALERT_BEEPS
    #endif
  );

    wait_for_confirmation(true, beep_count DXC_PASS);
  **/
  //
  
  START_MENU(); //Initialize screen
  //TODO home 

  
  temperature_timeout_call_Back= cancel_z_probe_offset;

  
  calculated_z_offset = probe.offset.z + current_position.z - z_offset_ref;

  //if (LCD_HEIGHT >= 4)
    //STATIC_ITEM(MSG_MOVE_NOZZLE_TO_BED, SS_CENTER|SS_INVERT); //Only for the menu

  //STATIC_ITEM_P(PSTR("Z="), SS_CENTER, ftostr42_52(current_position.z));
  STATIC_ITEM( MSG_ZPROBE_ZOFFSET_SET, SS_LEFT, ftostr42_52(calculated_z_offset));

  SUBMENU(MSG_MOVE_1MM,  []{ _goto_manual_move_z( 1);    });
  SUBMENU(MSG_MOVE_01MM, []{ _goto_manual_move_z( 0.1f); });

  if ((FINE_MANUAL_MOVE) > 0.0f && (FINE_MANUAL_MOVE) < 0.1f) {
    char tmp[20], numstr[10];
    // Determine digits needed right of decimal
    const uint8_t digs = !UNEAR_ZERO((FINE_MANUAL_MOVE) * 1000 - int((FINE_MANUAL_MOVE) * 1000)) ? 4 :
                         !UNEAR_ZERO((FINE_MANUAL_MOVE) *  100 - int((FINE_MANUAL_MOVE) *  100)) ? 3 : 2;
    sprintf_P(tmp, GET_TEXT(MSG_MOVE_N_MM), dtostrf(FINE_MANUAL_MOVE, 1, digs, numstr));
    #if DISABLED(HAS_GRAPHICAL_TFT)
      SUBMENU_P(NUL_STR, []{ _goto_manual_move_z(float(FINE_MANUAL_MOVE)); });
      MENU_ITEM_ADDON_START(0 + ENABLED(HAS_MARLINUI_HD44780));
      lcd_put_u8str(tmp);
      MENU_ITEM_ADDON_END();
    #else
      SUBMENU_P(tmp, []{ _goto_manual_move_z(float(FINE_MANUAL_MOVE)); });
    #endif
  }

  ACTION_ITEM(MSG_BUTTON_DONE, []{
    set_offset_and_end(calculated_z_offset);
    current_position.z = z_offset_ref;  // Set Z to z_offset_ref, as we can expect it is at probe height
    sync_plan_position();
    ui.store_settings(); //Save settings 
    z_clearance_move();                 // Raise Z as if it was homed
    
    clear_temperature();
    
    ui.goto_previous_screen_no_defer();
  });

  ACTION_ITEM(MSG_BUTTON_CANCEL, []{
    cancel_z_probe_offset();
    ui.goto_previous_screen_no_defer();
  });

  END_MENU();
}



void prepare_for_probe_offset_wizard() {
  #if defined(PROBE_OFFSET_WIZARD_XY_POS) || !HOMING_Z_WITH_PROBE
    if (ui.should_draw()) MenuItem_static::draw(1, GET_TEXT(MSG_PROBE_WIZARD_PROBING));

    if (ui.wait_for_move) return;

    #ifndef PROBE_OFFSET_WIZARD_XY_POS
      #define PROBE_OFFSET_WIZARD_XY_POS XY_CENTER
    #endif
    // Get X and Y from configuration, or use center
    constexpr xy_pos_t wizard_pos = PROBE_OFFSET_WIZARD_XY_POS;

    // Probe for Z reference
    ui.wait_for_move = true;
    z_offset_ref = probe.probe_at_point(wizard_pos, PROBE_PT_RAISE, 0, true);
    ui.wait_for_move = false;

    // Stow the probe, as the last call to probe.probe_at_point(...) left
    // the probe deployed if it was successful.
    probe.stow();
  #else
    if (ui.wait_for_move) return;
  #endif

  // Move Nozzle to Probing/Homing Position
  ui.wait_for_move = true;
  current_position += probe.offset_xy;
  line_to_current_position(MMM_TO_MMS(XY_PROBE_FEEDRATE));
  ui.synchronize(GET_TEXT(MSG_PROBE_WIZARD_MOVING));
  ui.wait_for_move = false;

  SET_SOFT_ENDSTOP_LOOSE(true); // Disable soft endstops for free Z movement

  // Go to Calibration Menu
  ui.goto_screen(probe_offset_wizard_menu);
  ui.defer_status_screen();
}

bool is_in_z_probe_offset_process = false;

void goto_probe_offset_wizard() {
  ui.defer_status_screen();
  set_all_unhomed();

  
  // Store probe.offset.z for Case: Cancel
  z_offset_backup = probe.offset.z;
  is_in_z_probe_offset_process = true;

  #ifdef PROBE_OFFSET_WIZARD_START_Z
    probe.offset.z = PROBE_OFFSET_WIZARD_START_Z;
  #endif

  // Store Bed-Leveling-State and disable
  #if HAS_LEVELING
    leveling_was_active = planner.leveling_active;
    set_bed_leveling_enabled(false);
  #endif
  /**
  queue.inject_P(PSTR("M140 S55"));//just set Bed temperatur
  
  queue.inject_P(PSTR("M104 S210")); //M109 waits and M104 dont wait. Only gcode queue 

  // Home all axes
  queue.inject_P(G28_STR);
**/

  
  queue.inject_P(PSTR(PREHEAT_GCODE_BEVORE_MEASUREMENT "\nG1 Z0")); //TODO: G10 Retract
  //queue.inject_P(PSTR("G28\nG90\nG1 Z0")); 



      ui.goto_screen([]{

        //ui.pause_show_message(PAUSE_MESSAGE_HEATING, PAUSE_MODE_SAME);
        _lcd_draw_heating_up_temperature(); //temperature like that
        //_lcd_draw_Heating_up_Temperature();
        if (all_axes_homed()) {
          z_offset_ref = 0;             // Set Z Value for Wizard Position to 0
          ui.goto_screen(prepare_for_probe_offset_wizard);
          ui.defer_status_screen();
        }
      });

  //_lcd_draw_Heating_up_Temperature();
/**
  ui.goto_screen([]{

    //_lcd_draw_homing(); //temperature like that
    
    ui.pause_show_message(PAUSE_MESSAGE_HEATING, PAUSE_MODE_SAME);
    //_lcd_draw_Heating_up_Temperature();
    //!thermalManager.isHeatingBed()
    if (!thermalManager.isHeatingBed()) {
      
      ui.goto_screen([]{

        _lcd_draw_homing(); //temperature like that
        if (all_axes_homed()) {
          z_offset_ref = 0;             // Set Z Value for Wizard Position to 0
          ui.goto_screen(prepare_for_probe_offset_wizard);
          ui.defer_status_screen();
        }
      });
  
    }
  });
  **/
  /**
 //temperature like that
  ui.goto_screen([]{
    _lcd_draw_homing();
    if (all_axes_homed()) {
      z_offset_ref = 0;             // Set Z Value for Wizard Position to 0
      ui.goto_screen(prepare_for_probe_offset_wizard);
      ui.defer_status_screen();
    }
  });
  **/

//clear_movement_and_Temperature();
}

#endif // PROBE_OFFSET_WIZARD
