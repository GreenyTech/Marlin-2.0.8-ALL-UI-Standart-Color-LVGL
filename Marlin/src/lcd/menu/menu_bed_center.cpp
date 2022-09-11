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
// Level Bed Corners menu
//

#include "../../inc/MarlinConfigPre.h"

#if BOTH(HAS_LCD_MENU, LEVEL_BED_CORNERS)

#include "menu_item.h"
#include "../../module/motion.h"
#include "../../module/planner.h"

#if HAS_LEVELING
  #include "../../feature/bedlevel/bedlevel.h"
#endif

#ifndef LEVEL_CORNERS_Z_HOP
  #define LEVEL_CORNERS_Z_HOP 4.0
#endif
#ifndef LEVEL_CORNERS_HEIGHT
  #define LEVEL_CORNERS_HEIGHT 0.0
#endif

#if ENABLED(LEVEL_CORNERS_USE_PROBE)
  #include "../../module/probe.h"
  #include "../../module/endstops.h"
  #if ENABLED(BLTOUCH)
    #include "../../feature/bltouch.h"
  #endif
  #ifndef LEVEL_CORNERS_PROBE_TOLERANCE
    #define LEVEL_CORNERS_PROBE_TOLERANCE 0.2
  #endif
  float last_z;
  int good_points;
  bool corner_probing_done, wait_for_probe;

  #if HAS_MARLINUI_U8GLIB
    #include "../dogm/marlinui_DOGM.h"
  #endif
  #define GOOD_POINTS_TO_STR(N) ui8tostr2(N)
  #define LAST_Z_TO_STR(N) ftostr53_63(N) //ftostr42_52(N)

#endif

static_assert(LEVEL_CORNERS_Z_HOP >= 0, "LEVEL_CORNERS_Z_HOP must be >= 0. Please update your configuration.");

#if HAS_LEVELING
  static bool leveling_was_active = false;
#endif

#ifndef LEVEL_CORNERS_LEVELING_ORDER
  #define LEVEL_CORNERS_LEVELING_ORDER { LF, RF, LB, RB }; // Default
  //#define LEVEL_CORNERS_LEVELING_ORDER { LF, LB, RF  }  // 3 hard-coded points
  //#define LEVEL_CORNERS_LEVELING_ORDER { LF, RF }       // 3-Point tramming - Rear
  //#define LEVEL_CORNERS_LEVELING_ORDER { LF, LB }       // 3-Point tramming - Right
  //#define LEVEL_CORNERS_LEVELING_ORDER { RF, RB }       // 3-Point tramming - Left
  //#define LEVEL_CORNERS_LEVELING_ORDER { LB, RB }       // 3-Point tramming - Front
#endif

#define LF 1
#define RF 2
#define RB 3
#define LB 4
#define CF 5
#define CB 6
#define CC 7
//constexpr int center_center_index = TERN(LEVEL_CENTER_TOO, available_center_points - 1, -1);
constexpr float inset_lfrb[4] = LEVEL_CORNERS_INSET_LFRB;
constexpr xy_pos_t lf { (X_MIN_BED) + inset_lfrb[0], (Y_MIN_BED) + inset_lfrb[1] },
                   rb { (X_MAX_BED) - inset_lfrb[2], (Y_MAX_BED) - inset_lfrb[3] };

static int8_t bed_corner;







constexpr int lcoCenter[] = LEVEL_CENTERS_LEVELING_ORDER;
//constexpr bool level_corners_3_points = COUNT(lcoCenter) == 2;
//static_assert(level_corners_3_points || COUNT(lcoCenter) == 4, "LEVEL_CORNERS_LEVELING_ORDER must have exactly 2 or 4 corners.");

constexpr int lcoCenterdiff = abs(lcoCenter[0] - lcoCenter[1]);
//static_assert(COUNT(lcoCenter) == 4 || lcoCenterdiff == 1 || lcoCenterdiff == 3, "The first two LEVEL_CORNERS_LEVELING_ORDER corners must be on the same edge.");

//constexpr int nr_edge_points = level_corners_3_points ? 3 : 4;
constexpr int available_center_points = sizeof(lcoCenter)/sizeof(lcoCenter[0]);//nr_edge_points + ENABLED(LEVEL_CENTER_TOO)+2;


/**
 * Select next corner coordinates
 */
static void _lcd_level_bed_center_get_next_position() {

 
    // Four-Corner Bed Tramming with optional center
    
      current_position = lf;                       // Left front
        switch (lcoCenter[bed_corner]) {
          case CF: current_position.x = X_CENTER; break; //Middle Front
          case RF: current_position.x = rb.x; break; // Right Front
          case RB: current_position   = rb;   break; // Right Back
          case CB: current_position.x = X_CENTER;  //middle Back
                   current_position.y = rb.y; break;
          case LB: current_position.y = rb.y; break; // Left Back
          case CC: current_position.set(X_CENTER, Y_CENTER); break;
        }
  
}


  static void _lcd_goto_next_center() {
    line_to_z(LEVEL_CORNERS_Z_HOP);

    // Select next corner coordinates
    _lcd_level_bed_center_get_next_position();

    line_to_current_position(manual_feedrate_mm_s.x);
    line_to_z(LEVEL_CORNERS_HEIGHT);
    if (++bed_corner >= available_center_points) bed_corner = 0;
  }


static void _lcd_level_bed_center_homing() {
  //todo
  //_lcd_draw_homing();
  _lcd_draw_heating_up_temperature();
  if (!all_axes_homed()) return;
  ui.buzz(200,500); //TODO anderer Ton?
  #if ENABLED(LEVEL_CORNERS_USE_PROBE)
    _lcd_test_corners();
    if (corner_probing_done) ui.goto_previous_screen_no_defer();
    TERN_(HAS_LEVELING, set_bed_leveling_enabled(leveling_was_active));
    endstops.enable_z_probe(false);
  #else
    bed_corner = 0;
    ui.goto_screen([]{
      MenuItem_confirm::select_screen(
          GET_TEXT(MSG_BUTTON_NEXT), GET_TEXT(MSG_BUTTON_DONE)
        , _lcd_goto_next_center
        , []{
          //TODO Gabriel 
          //ui.chirp();
            queue.inject_P(PSTR(CLEAR_PREHEAT_GCODE_BEVORE_MEASUREMENT)); //todo check if this is working
            line_to_z(LEVEL_CORNERS_Z_HOP); // Raise Z off the bed when done
            TERN_(HAS_LEVELING, set_bed_leveling_enabled(leveling_was_active));
            ui.goto_previous_screen_no_defer();
          }
        , GET_TEXT(TERN(LEVEL_CENTER_TOO, MSG_LEVEL_BED_NEXT_POINT, MSG_NEXT_CORNER))
        , (const char*)nullptr, PSTR("?")
      );
    });
    ui.set_selection(true);
    _lcd_goto_next_center();
  #endif
}

void _lcd_level_bed_center() {
//TODO insert Leveling

  ui.defer_status_screen();

  /**

  if (!all_axes_trusted()) {
    set_all_unhomed();
    queue.inject_P(G28_STR);
  }
  **/
  //queue.inject_P(PSTR("M190 S55\nM109 S210\nG92 E0\nG1 E F500\nG1 E-6\nG28\nG1 Z0")); //TODO: G10 Retract
    set_all_unhomed();
  queue.inject_P(PSTR(PREHEAT_GCODE_BEVORE_MEASUREMENT)); //TODO: G10 Retract

 

  // Disable leveling so the planner won't mess with us
  #if HAS_LEVELING
    leveling_was_active = planner.leveling_active;
    set_bed_leveling_enabled(false);
  #endif

  ui.goto_screen(_lcd_level_bed_center_homing);
}


void _lcd_level_bed_plane() {

  //ui.defer_status_screen();
  
  ui.return_to_status();
  ui.set_status_P(GET_TEXT(MSG_LEVEL_BED_PLEASE_WAIT),-1);
  
  //ui.status_printf_P(0, PSTR(S_FMT " %i/%i"), GET_TEXT(MSG_PROBING_MESH), int(pt_index), int(abl.abl_points));

  if (!all_axes_trusted()) {
    set_all_unhomed();
    queue.inject_P(G28_STR);
  }

  // Disable leveling so the planner won't mess with us
  
  queue.inject_P(PSTR("G29N"));
  
}

#endif // HAS_LCD_MENU && LEVEL_BED_CORNERS
