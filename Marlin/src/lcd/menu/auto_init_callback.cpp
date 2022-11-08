

#include "auto_init_callback.h"



#include "../../feature/bed_temperature.h"
#include "../../feature/bedlevel/bedlevel.h"
#include "menu_item.h"




void (*auto_init_call_back_next_method)()=0;



void start_auto_init_process(){
        //METHOD:

     

     
    //ui.return_to_status();



    SERIAL_ECHO_MSG("start autotune");
    //LCD_MESSAGEPGM("init Process. Please Wait");
    //ui.draw_status_message()
    ui.set_status_P(GET_TEXT(MSG_AUTO_INITIALIZE_PLEASE_WAIT));

    //reset settings;
    auto_init_call_back_next_method = autotune_bed_temperature;//autotune_bed_temperature;


    execute_next_Auto_init_stepp();



    //TODO Menu das: nach einander

    //0. restore defaults

    //1. Bed PID
    //2. Nozzle PID
    //3. Z Probe wizzard
    //4. Ecken anfahren
    //5. Bett Vermessen 
    //durchführt.
}



void autotune_bed_temperature(){
    
    SERIAL_ECHO_MSG("start bed Temp");
    auto_init_call_back_next_method = autotune_nozzle_temperature;
    
    
    if(!bed_temperature_DISABLED){
        /**char cmd[20];
        sprintf_P(cmd, PSTR("M303 U1 E%i S%i"), H_BED, PREHEAT_1_TEMP_BED); 
        queue.inject(cmd);
        **/
       
        thermalManager.PID_autotune(50, H_BED, 5, true);
        
        execute_next_Auto_init_stepp();
        //init_process_z_probe_wizzard();
    }
    else{
        execute_next_Auto_init_stepp();

    }

}

void autotune_nozzle_temperature(){
    
    
    SERIAL_ECHO_MSG("start Nozztle Temp");
    auto_init_call_back_next_method = init_process_z_probe_wizzard;
    /**
    char cmd[20];
    //sprintf_P(cmd, PSTR("M303 U1 E%i S%i"), hid, tune_temp);
    sprintf_P(cmd, PSTR("M303 U1 E%i S%i"), H_E0 , PREHEAT_1_TEMP_HOTEND); 

    

    queue.inject(cmd);
**/

  thermalManager.PID_autotune(215, H_E0, 5, true);
  SERIAL_ECHO_MSG("done pid");
  //ui.reset_status();
  
  execute_next_Auto_init_stepp();
  //autotune_bed_temperature();


}







void init_process_z_probe_wizzard(){
    
    SERIAL_ECHO_MSG("start Z wizzard");
    auto_init_call_back_next_method = init_process_calibrate_corners;

    ui.goto_screen(goto_probe_offset_wizard);
    
    
    SERIAL_ECHO_MSG("z probe wizzarid ui routine was called");
    
    //TODO
}


void init_process_calibrate_corners(){
    
    SERIAL_ECHO_MSG("Level Corner");
    
    //sdf


    
    auto_init_call_back_next_method = init_process_calibrate_middle_edges;
    //TODO
       ui.goto_screen( _lcd_level_bed_corners);
    
        //execute_next_Auto_init_stepp();
}

void init_process_calibrate_middle_edges(){
    
    
    SERIAL_ECHO_MSG("Level Middle");
    

    auto_init_call_back_next_method =init_process_bed_leveling ;
    //TODO
        ui.goto_screen(_lcd_level_bed_center);
    
        //execute_next_Auto_init_stepp();
}

void init_process_bed_leveling(){
    
    SERIAL_ECHO_MSG("bett automatisch vermessen");
    auto_init_call_back_next_method = init_process_success;
    //TODO
    
    reset_bed_level(); 


    //ui.goto_screen(_lcd_level_bed_plane);
    
        execute_next_Auto_init_stepp();
}

void init_process_success(){
    
    SERIAL_ECHO_MSG("Success");
    
    auto_init_call_back_next_method = 0;
    
    //TODO
    
        //execute_next_Auto_init_stepp();

    ui.store_settings();

        
    ui.set_status_P(GET_TEXT(MSG_AUTO_INITIALIZE_SUCCESSFULL));
    
    ui.buzz(100,400);
    ui.buzz(100,500);
    ui.buzz(100,400);
    ui.buzz(100,500);
    ui.buzz(150,800);
}











void execute_next_Auto_init_stepp(){
    /*
    ui.buzz(100,200);
    ui.buzz(100,500);
    ui.buzz(100,200);
    ui.buzz(100,500);
    ui.buzz(100,200);
    ui.buzz(100,500);
    ui.buzz(100,200);
    ui.buzz(100,500);
    ui.buzz(150,800);
    */
    


    
    if(is_in_init_process()){
      auto_init_call_back_next_method();
      //auto_init_call_back_next_method=0;
      
        SERIAL_ECHO_MSG("Init Routine finished");
    }
    
    SERIAL_ECHO_MSG("called next");
}

bool is_in_init_process(){
    return auto_init_call_back_next_method!=0;
}