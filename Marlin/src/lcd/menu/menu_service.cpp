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
// Info Menu
//

#include "../../inc/MarlinConfigPre.h"

#if BOTH(HAS_LCD_MENU, LCD_INFO_MENU)

#include "menu_item.h"



#define VALUE_ITEM(MSG, VALUE, STYL)    do{ char msg[21]; strcpy_P(msg, PSTR(": ")); strcpy(msg + 2, VALUE); STATIC_ITEM(MSG, STYL, msg); }while(0)
#define VALUE_ITEM_P(MSG, PVALUE, STYL) do{ char msg[21]; strcpy_P(msg, PSTR(": ")); strcpy_P(msg + 2, PSTR(PVALUE)); STATIC_ITEM(MSG, STYL, msg); }while(0)

#if ENABLED(PRINTCOUNTER)

  #include "../../module/printcounter.h"

  //
  // About Printer > Printer Stats
  //
  void menu_info_stats_service() {
    if (ui.use_click()) return ui.go_back();

    printStatistics stats = print_job_timer.getStats();

    char buffer[21];

    START_SCREEN();                                                                         // 12345678901234567890
    VALUE_ITEM(MSG_INFO_PRINT_COUNT, i16tostr3left(stats.totalPrints), SS_LEFT);            // Print Count: 999
    VALUE_ITEM(MSG_INFO_COMPLETED_PRINTS, i16tostr3left(stats.finishedPrints), SS_LEFT);    // Completed  : 666

    STATIC_ITEM(MSG_INFO_PRINT_TIME, SS_LEFT);                                              // Total print Time:
    STATIC_ITEM_P(PSTR("> "), SS_LEFT, duration_t(stats.printTime).toString(buffer));       // > 99y 364d 23h 59m 59s

    STATIC_ITEM(MSG_INFO_PRINT_LONGEST, SS_LEFT);                                           // Longest job time:
    STATIC_ITEM_P(PSTR("> "), SS_LEFT, duration_t(stats.longestPrint).toString(buffer));    // > 99y 364d 23h 59m 59s

    STATIC_ITEM(MSG_INFO_PRINT_FILAMENT, SS_LEFT);                                          // Extruded total:
    sprintf_P(buffer, PSTR("%ld.%im")
      , long(stats.filamentUsed / 1000)
      , int16_t(stats.filamentUsed / 100) % 10
    );
    STATIC_ITEM_P(PSTR("> "), SS_LEFT, buffer);                                             // > 125m

    END_SCREEN();
  }


    void menu_info_stats_service_Routine() {

      if (ui.use_click()) return ui.go_back();

    printStatistics stats = print_job_timer.getStats();

    char buffer[21];

    START_SCREEN();                                                                         

    #if SERVICE_INTERVAL_1 > 0 || SERVICE_INTERVAL_2 > 0 || SERVICE_INTERVAL_3 > 0
      strcpy_P(buffer, GET_TEXT(MSG_SERVICE_IN));
    #endif

    #if SERVICE_INTERVAL_1 > 0
      STATIC_ITEM_P(PSTR(SERVICE_NAME_1 " "), SS_LEFT, buffer);                             // Service X in:
      STATIC_ITEM_P(PSTR("> "), SS_LEFT, duration_t(stats.nextService1).toString(buffer));  // > 7d 12h 11m 10s
    #endif

    #if SERVICE_INTERVAL_2 > 0
      STATIC_ITEM_P(PSTR(SERVICE_NAME_2 " "), SS_LEFT, buffer);
      STATIC_ITEM_P(PSTR("> "), SS_LEFT, duration_t(stats.nextService2).toString(buffer));
    #endif

    #if SERVICE_INTERVAL_3 > 0
      STATIC_ITEM_P(PSTR(SERVICE_NAME_3 " "), SS_LEFT, buffer);
      STATIC_ITEM_P(PSTR("> "), SS_LEFT, duration_t(stats.nextService3).toString(buffer));
    #endif

    END_SCREEN();
      }

#endif




//
// "About Printer" submenu
//
void menu_service() {
  START_MENU();
  BACK_ITEM(MSG_MAIN);

  #if ENABLED(PRINTCOUNTER)
    SUBMENU(MSG_INFO_STATS_MENU, menu_info_stats_service);               // Printer Stats >
  #endif


   SUBMENU(MSG_INFO_NEXT_SERVICE_MENU, menu_info_stats_service_Routine);               // Printer Next Serviceses >
  
  
    #if HAS_SERVICE_INTERVALS
    static auto _service_reset = [](const int index) {
      print_job_timer.resetServiceInterval(index);
      ui.completion_feedback();
      ui.reset_status();
      ui.return_to_status();
    };
    #if SERVICE_INTERVAL_1 > 0
      CONFIRM_ITEM_P(PSTR(SERVICE_NAME_RESET_1),
        MSG_BUTTON_RESET, MSG_BUTTON_CANCEL,
        []{ _service_reset(1); }, ui.goto_previous_screen,
        GET_TEXT(MSG_SERVICE_RESET), F(SERVICE_NAME_1), PSTR("?")
      );
    #endif
    #if SERVICE_INTERVAL_2 > 0
      CONFIRM_ITEM_P(PSTR(SERVICE_NAME_RESET_2),
        MSG_BUTTON_RESET, MSG_BUTTON_CANCEL,
        []{ _service_reset(2); }, ui.goto_previous_screen,
        GET_TEXT(MSG_SERVICE_RESET), F(SERVICE_NAME_2), PSTR("?")
      );
    #endif
    #if SERVICE_INTERVAL_3 > 0
      CONFIRM_ITEM_P(PSTR(SERVICE_NAME_3),
        MSG_BUTTON_RESET, MSG_BUTTON_CANCEL,
        []{ _service_reset(3); }, ui.goto_previous_screen,
        GET_TEXT(MSG_SERVICE_RESET), F(SERVICE_NAME_3), PSTR("?")
      );
    #endif
  #endif
  if(!printer_busy()){

  if(homing_needed()){
    GCODES_ITEM(MOVE_AXES_FOR_GRASPING_ROUTINE_MENU, PSTR("G28\nG0 X0 Y0 Z0 F4000\nG0 X350 Y333 Z365 F4000\nG0 X170 Y200 Z200 F4000"));
  }
  else{
    GCODES_ITEM(MOVE_AXES_FOR_GRASPING_ROUTINE_MENU, PSTR("G0 X0 Y0 Z0 F4000\nG0 X350 Y333 Z365 F4000\nG0 X170 Y200 Z200 F4000"));
  }
  
  }

  END_MENU();
}

#endif // HAS_LCD_MENU && LCD_INFO_MENU
