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
// Main Menu
//

#include "../../inc/MarlinConfigPre.h"

#if HAS_LCD_MENU

#include "menu_item.h"
#include "../../module/temperature.h"
#include "../../gcode/queue.h"
#include "../../module/printcounter.h"
#include "../../module/stepper.h"
#include "../../sd/cardreader.h"

#if HAS_GAMES && DISABLED(LCD_INFO_MENU)
  #include "game/game.h"
#endif

#if EITHER(SDSUPPORT, HOST_PROMPT_SUPPORT) || defined(ACTION_ON_CANCEL)
  #define MACHINE_CAN_STOP 1
#endif
#if ANY(SDSUPPORT, HOST_PROMPT_SUPPORT, PARK_HEAD_ON_PAUSE) || defined(ACTION_ON_PAUSE)
  #define MACHINE_CAN_PAUSE 1
#endif

#if ENABLED(MMU2_MENUS)
  #include "../../lcd/menu/menu_mmu2.h"
#endif

#if ENABLED(PASSWORD_FEATURE)
  #include "../../feature/password/password.h"
#endif

#if ENABLED(HOST_START_MENU_ITEM) && defined(ACTION_ON_START)
  #include "../../feature/host_actions.h"
#endif

#if ENABLED(GCODE_REPEAT_MARKERS)
  #include "../../feature/repeat.h"
#endif

void menu_tune();
void menu_cancelobject();
void menu_motion();
void menu_temperature();
void menu_configuration();

#if HAS_POWER_MONITOR
  void menu_power_monitor();
#endif

#if ENABLED(MIXING_EXTRUDER)
  void menu_mixer();
#endif

#if ENABLED(ADVANCED_PAUSE_FEATURE)
  void _menu_temp_filament_op(const PauseMode, const int8_t);
  void menu_change_filament();
#endif

#if ENABLED(LCD_INFO_MENU)
  void menu_info();
#endif

#if ENABLED(SERVICE_ROUTINE)
void menu_service();
#endif

#if EITHER(LED_CONTROL_MENU, CASE_LIGHT_MENU)
  void menu_led();
#endif

#if HAS_CUTTER
  void menu_spindle_laser();
#endif

#if ENABLED(PREHEAT_SHORTCUT_MENU_ITEM)
  void menu_preheat_only();
#endif

#if HAS_MULTI_LANGUAGE
  void menu_language();
#endif







///preheat pla / Chagne filament
static PauseMode _change_filament_mode; // = PAUSE_MODE_PAUSE_PRINT
static int8_t _change_filament_extruder; // = 0

inline PGM_P _change_filament_command() {
  switch (_change_filament_mode) {
    case PAUSE_MODE_LOAD_FILAMENT:    return PSTR("M701 T%d");
    case PAUSE_MODE_UNLOAD_FILAMENT:  return _change_filament_extruder >= 0
                                           ? PSTR("M702 T%d") : PSTR("M702 ;%d");
    case PAUSE_MODE_CHANGE_FILAMENT:
    case PAUSE_MODE_PAUSE_PRINT:
    default: break;
  }
  return PSTR("M600 B0 T%d");
}

// Initiate Filament Load/Unload/Change at the specified temperature
static void _change_filament_with_temp(const uint16_t celsius) {
  char cmd[11];
  sprintf_P(cmd, _change_filament_command(), _change_filament_extruder);
  thermalManager.setTargetHotend(celsius, _change_filament_extruder);
  queue.inject(cmd);
}

static void _change_filament_with_preset() {
  //_change_filament_with_temp(ui.material_preset[MenuItemBase::itemIndex].hotend_temp);
  //todo ask if realy wanted
        MenuItem_confirm::select_screen(
          GET_TEXT(MSG_BUTTON_CHANGE), GET_TEXT(MSG_BACK),
          []{_change_filament_with_temp(ui.material_preset[MenuItemBase::itemIndex].hotend_temp);},
           ui.goto_previous_screen,
          GET_TEXT(MSG_CHANGE_FILAMENT), (const char *)nullptr, PSTR("?")
        );


        
}




//printer infos:
void menu_new_info_printer() {
    if (ui.use_click()) return ui.go_back();
    START_SCREEN();
    //STATIC_ITEM(MSG_MARLIN, SS_DEFAULT|SS_INVERT);              // Marlin
    STATIC_ITEM_P(PSTR(MACHINE_NAME));                          // My3DPrinter
    STATIC_ITEM_P(PSTR(SHORT_BUILD_VERSION));                   // x.x.x-Branch
    STATIC_ITEM_P(PSTR(STRING_DISTRIBUTION_DATE));              // YYYY-MM-DD HH:MM
    STATIC_ITEM_P(PSTR(WEBSITE_URL));                           // www.my3dprinter.com
    STATIC_ITEM_P(PSTR(BOARD_INFO_NAME), SS_DEFAULT|SS_INVERT);      // MyPrinterController
    /**
    PSTRING_ITEM(MSG_INFO_EXTRUDERS, STRINGIFY(EXTRUDERS), SS_CENTER); // Extruders: 2
    #if HAS_LEVELING
      STATIC_ITEM(
        TERN_(AUTO_BED_LEVELING_3POINT, MSG_3POINT_LEVELING)      // 3-Point Leveling
        TERN_(AUTO_BED_LEVELING_LINEAR, MSG_LINEAR_LEVELING)      // Linear Leveling
        TERN_(AUTO_BED_LEVELING_BILINEAR, MSG_BILINEAR_LEVELING)  // Bi-linear Leveling
        TERN_(AUTO_BED_LEVELING_UBL, MSG_UBL_LEVELING)            // Unified Bed Leveling
        TERN_(MESH_BED_LEVELING, MSG_MESH_LEVELING)               // Mesh Leveling
      );
    #endif

    **/
    END_SCREEN();
  }





















#if ENABLED(CUSTOM_MENU_MAIN)

  void _lcd_custom_menu_main_gcode(PGM_P const cmd) {
    queue.inject_P(cmd);
    TERN_(CUSTOM_MENU_MAIN_SCRIPT_AUDIBLE_FEEDBACK, ui.completion_feedback());
    TERN_(CUSTOM_MENU_MAIN_SCRIPT_RETURN, ui.return_to_status());
  }

  void custom_menus_main() {
    START_MENU();
    BACK_ITEM(MSG_MAIN);

    #define HAS_CUSTOM_ITEM_MAIN(N) (defined(MAIN_MENU_ITEM_##N##_DESC) && defined(MAIN_MENU_ITEM_##N##_GCODE))

    #define CUSTOM_TEST_MAIN(N) do{ \
      constexpr char c = MAIN_MENU_ITEM_##N##_GCODE[strlen(MAIN_MENU_ITEM_##N##_GCODE) - 1]; \
      static_assert(c != '\n' && c != '\r', "MAIN_MENU_ITEM_" STRINGIFY(N) "_GCODE cannot have a newline at the end. Please remove it."); \
    }while(0)

    #ifdef MAIN_MENU_ITEM_SCRIPT_DONE
      #define _DONE_SCRIPT "\n" MAIN_MENU_ITEM_SCRIPT_DONE
    #else
      #define _DONE_SCRIPT ""
    #endif
    #define GCODE_LAMBDA_MAIN(N) []{ _lcd_custom_menu_main_gcode(PSTR(MAIN_MENU_ITEM_##N##_GCODE _DONE_SCRIPT)); }
    #define _CUSTOM_ITEM_MAIN(N) ACTION_ITEM_P(PSTR(MAIN_MENU_ITEM_##N##_DESC), GCODE_LAMBDA_MAIN(N));
    #define _CUSTOM_ITEM_MAIN_CONFIRM(N)             \
      SUBMENU_P(PSTR(MAIN_MENU_ITEM_##N##_DESC), []{ \
          MenuItem_confirm::confirm_screen(          \
            GCODE_LAMBDA_MAIN(N),                    \
            ui.goto_previous_screen,                 \
            PSTR(MAIN_MENU_ITEM_##N##_DESC "?")      \
          );                                         \
        })

    #define CUSTOM_ITEM_MAIN(N) do{ if (ENABLED(MAIN_MENU_ITEM_##N##_CONFIRM)) _CUSTOM_ITEM_MAIN_CONFIRM(N); else _CUSTOM_ITEM_MAIN(N); }while(0)

    #if HAS_CUSTOM_ITEM_MAIN(1)
      CUSTOM_TEST_MAIN(1);
      CUSTOM_ITEM_MAIN(1);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(2)
      CUSTOM_TEST_MAIN(2);
      CUSTOM_ITEM_MAIN(2);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(3)
      CUSTOM_TEST_MAIN(3);
      CUSTOM_ITEM_MAIN(3);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(4)
      CUSTOM_TEST_MAIN(4);
      CUSTOM_ITEM_MAIN(4);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(5)
      CUSTOM_TEST_MAIN(5);
      CUSTOM_ITEM_MAIN(5);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(6)
      CUSTOM_TEST_MAIN(6);
      CUSTOM_ITEM_MAIN(6);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(7)
      CUSTOM_TEST_MAIN(7);
      CUSTOM_ITEM_MAIN(7);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(8)
      CUSTOM_TEST_MAIN(8);
      CUSTOM_ITEM_MAIN(8);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(9)
      CUSTOM_TEST_MAIN(9);
      CUSTOM_ITEM_MAIN(9);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(10)
      CUSTOM_TEST_MAIN(10);
      CUSTOM_ITEM_MAIN(10);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(11)
      CUSTOM_TEST_MAIN(11);
      CUSTOM_ITEM_MAIN(11);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(12)
      CUSTOM_TEST_MAIN(12);
      CUSTOM_ITEM_MAIN(12);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(13)
      CUSTOM_TEST_MAIN(13);
      CUSTOM_ITEM_MAIN(13);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(14)
      CUSTOM_TEST_MAIN(14);
      CUSTOM_ITEM_MAIN(14);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(15)
      CUSTOM_TEST_MAIN(15);
      CUSTOM_ITEM_MAIN(15);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(16)
      CUSTOM_TEST_MAIN(16);
      CUSTOM_ITEM_MAIN(16);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(17)
      CUSTOM_TEST_MAIN(17);
      CUSTOM_ITEM_MAIN(17);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(18)
      CUSTOM_TEST_MAIN(18);
      CUSTOM_ITEM_MAIN(18);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(19)
      CUSTOM_TEST_MAIN(19);
      CUSTOM_ITEM_MAIN(19);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(20)
      CUSTOM_TEST_MAIN(20);
      CUSTOM_ITEM_MAIN(20);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(21)
      CUSTOM_TEST_MAIN(21);
      CUSTOM_ITEM_MAIN(21);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(22)
      CUSTOM_TEST_MAIN(22);
      CUSTOM_ITEM_MAIN(22);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(23)
      CUSTOM_TEST_MAIN(23);
      CUSTOM_ITEM_MAIN(23);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(24)
      CUSTOM_TEST_MAIN(24);
      CUSTOM_ITEM_MAIN(24);
    #endif
    #if HAS_CUSTOM_ITEM_MAIN(25)
      CUSTOM_TEST_MAIN(25);
      CUSTOM_ITEM_MAIN(25);
    #endif
    END_MENU();
  }

#endif // CUSTOM_MENU_MAIN

void menu_main() {
  const bool busy = printingIsActive()
    #if ENABLED(SDSUPPORT)
      , card_detected = card.isMounted()
      , card_open = card_detected && card.isFileOpen()
    #endif
  ;

  START_MENU();
  BACK_ITEM(MSG_INFO_SCREEN);

  if (busy) {
    #if MACHINE_CAN_PAUSE
      ACTION_ITEM(MSG_PAUSE_PRINT, ui.pause_print);
    #endif
    #if MACHINE_CAN_STOP
      SUBMENU(MSG_STOP_PRINT, []{
        MenuItem_confirm::select_screen(
          GET_TEXT(MSG_BUTTON_STOP), GET_TEXT(MSG_BACK),
          ui.abort_print, ui.goto_previous_screen,
          GET_TEXT(MSG_STOP_PRINT), (const char *)nullptr, PSTR("?")
        );
      });
    #endif

    #if ENABLED(GCODE_REPEAT_MARKERS)
      if (repeat.is_active())
        ACTION_ITEM(MSG_END_LOOPS, repeat.cancel);
    #endif

    //SUBMENU(MSG_TUNE, menu_tune);

    #if ENABLED(CANCEL_OBJECTS) && DISABLED(SLIM_LCD_MENUS)
      SUBMENU(MSG_CANCEL_OBJECT, []{ editable.int8 = -1; ui.goto_screen(menu_cancelobject); });
    #endif
  }
  else {

    #if !HAS_ENCODER_WHEEL && ENABLED(SDSUPPORT)

      // *** IF THIS SECTION IS CHANGED, REPRODUCE BELOW ***

      //
      // Run Auto Files
      //
      #if ENABLED(MENU_ADDAUTOSTART)
        ACTION_ITEM(MSG_RUN_AUTO_FILES, card.autofile_begin);
      #endif

      if (card_detected) {
        if (!card_open) {
          SUBMENU(MSG_MEDIA_MENU, MEDIA_MENU_GATEWAY);
          #if PIN_EXISTS(SD_DETECT)
            GCODES_ITEM(MSG_CHANGE_MEDIA, PSTR("M21"));
          #else
            GCODES_ITEM(MSG_RELEASE_MEDIA, PSTR("M22"));
          #endif
        }
      }
      else {
        #if PIN_EXISTS(SD_DETECT)
          ACTION_ITEM(MSG_NO_MEDIA, nullptr);
        #else
          GCODES_ITEM(MSG_ATTACH_MEDIA, PSTR("M21"));
        #endif
      }

    #endif // !HAS_ENCODER_WHEEL && SDSUPPORT

    if (TERN0(MACHINE_CAN_PAUSE, printingIsPaused()))
      ACTION_ITEM(MSG_RESUME_PRINT, ui.resume_print);

    #if ENABLED(HOST_START_MENU_ITEM) && defined(ACTION_ON_START)
      ACTION_ITEM(MSG_HOST_START_PRINT, host_action_start);
    #endif

    #if ENABLED(PREHEAT_SHORTCUT_MENU_ITEM)
      SUBMENU(MSG_PREHEAT_CUSTOM, menu_preheat_only);
    #endif

    SUBMENU(MSG_MOTION, menu_motion);
  }
  
  

  #if HAS_CUTTER
    SUBMENU(MSG_CUTTER(MENU), STICKY_SCREEN(menu_spindle_laser));
  #endif

  #if HAS_TEMPERATURE
    SUBMENU(MSG_TEMPERATURE, menu_temperature);
  #endif

  #if HAS_POWER_MONITOR
    SUBMENU(MSG_POWER_MONITOR, menu_power_monitor);
  #endif

  #if ENABLED(MIXING_EXTRUDER)
    SUBMENU(MSG_MIXER, menu_mixer);
  #endif

  #if ENABLED(MMU2_MENUS)
    if (!busy) SUBMENU(MSG_MMU2_MENU, menu_mmu2);
  #endif

  SUBMENU(MSG_CONFIGURATION, menu_configuration);

  #if ENABLED(CUSTOM_MENU_MAIN)
    if (TERN1(CUSTOM_MENU_MAIN_ONLY_IDLE, !busy)) {
      #ifdef CUSTOM_MENU_MAIN_TITLE
        SUBMENU_P(PSTR(CUSTOM_MENU_MAIN_TITLE), custom_menus_main);
      #else
        SUBMENU(MSG_CUSTOM_COMMANDS, custom_menus_main);
      #endif
    }
  #endif

/*
  #if ENABLED(ADVANCED_PAUSE_FEATURE)
    #if E_STEPPERS == 1 && DISABLED(FILAMENT_LOAD_UNLOAD_GCODES)
      if (thermalManager.targetHotEnoughToExtrude(active_extruder))
        GCODES_ITEM(MSG_FILAMENTCHANGE, PSTR("M600 B0"));
      else
      //todo
        //SUBMENU(MSG_FILAMENTCHANGE, []{ _menu_temp_filament_op(PAUSE_MODE_CHANGE_FILAMENT, 0); }); // We only print PLA -> change to MSG_Filamentchange PLA
        
        ACTION_ITEM_N_S(0, "", MSG_FILAMENTCHANGE, _change_filament_with_preset);
    #else
      SUBMENU(MSG_FILAMENTCHANGE, menu_change_filament);
    #endif
  #endif
  */

 
  //ACTION_ITEM_N_S(0, "", MSG_FILAMENTCHANGE, _change_filament_with_preset);

  SUBMENU(MSG_FILAMENTCHANGE,_change_filament_with_preset);

#if ENABLED(SERVICE_ROUTINE)
  SUBMENU(MSG_Service_Routine_MENU, menu_service); //add menu information -> replace
#endif

  SUBMENU(MSG_INFO_PRINTER_MENU, menu_new_info_printer);           // Printer Info >
    
/**
  #if ENABLED(LCD_INFO_MENU)
    SUBMENU(MSG_INFO_MENU, menu_info); 
  #endif

**/
  #if EITHER(LED_CONTROL_MENU, CASE_LIGHT_MENU)
    SUBMENU(MSG_LEDS, menu_led);
  #endif

  //
  // Switch power on/off
  //
  #if ENABLED(PSU_CONTROL)
    if (powersupply_on)
      GCODES_ITEM(MSG_SWITCH_PS_OFF, PSTR("M81"));
    else
      GCODES_ITEM(MSG_SWITCH_PS_ON, PSTR("M80"));
  #endif

  #if BOTH(HAS_ENCODER_WHEEL, SDSUPPORT)

    if (!busy) {

      // *** IF THIS SECTION IS CHANGED, REPRODUCE ABOVE ***

      //
      // Autostart
      //
      #if ENABLED(MENU_ADDAUTOSTART)
        ACTION_ITEM(MSG_RUN_AUTO_FILES, card.autofile_begin);
      #endif

      if (card_detected) {
        if (!card_open) {
          #if PIN_EXISTS(SD_DETECT)
            GCODES_ITEM(MSG_CHANGE_MEDIA, PSTR("M21"));
          #else
          /** Disable attach micro sd card Release Media attach media**/
            GCODES_ITEM(MSG_RELEASE_MEDIA, PSTR("M22"));
          #endif
          //SUBMENU(MSG_MEDIA_MENU, MEDIA_MENU_GATEWAY);
        }
      }
      else {
        #if PIN_EXISTS(SD_DETECT)
          ACTION_ITEM(MSG_NO_MEDIA, nullptr);
        #else
          GCODES_ITEM(MSG_ATTACH_MEDIA, PSTR("M21"));
        #endif
      }


    }


  #endif // HAS_ENCODER_WHEEL && SDSUPPORT




  #if HAS_GAMES && DISABLED(LCD_INFO_MENU)
    #if ENABLED(GAMES_EASTER_EGG)
      SKIP_ITEM();
      SKIP_ITEM();
      SKIP_ITEM();
    #endif
    // Game sub-menu or the individual game
    {
      SUBMENU(
        #if HAS_GAME_MENU
          MSG_GAMES, menu_game
        #elif ENABLED(MARLIN_BRICKOUT)
          MSG_BRICKOUT, brickout.enter_game
        #elif ENABLED(MARLIN_INVADERS)
          MSG_INVADERS, invaders.enter_game
        #elif ENABLED(MARLIN_SNAKE)
          MSG_SNAKE, snake.enter_game
        #elif ENABLED(MARLIN_MAZE)
          MSG_MAZE, maze.enter_game
        #endif
      );
    }
  #endif

  #if HAS_MULTI_LANGUAGE
    SUBMENU(LANGUAGE, menu_language);
  #endif

  END_MENU();
}

#endif // HAS_LCD_MENU
