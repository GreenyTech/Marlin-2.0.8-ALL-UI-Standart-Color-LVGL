

#pragma once

extern void (*auto_init_call_back_next_method)();
//temperature_timeout_call_Back

void start_auto_init_process();
void execute_next_Auto_init_stepp();




void autotune_bed_temperature();
void autotune_nozzle_temperature();
void init_process_z_probe_wizzard();
void init_process_calibrate_corners();
void init_process_calibrate_middle_edges();
void init_process_bed_leveling();
void init_process_success();

bool is_in_init_process();
