

#include "hotend_idle_callback.h"


void (*temperature_timeout_call_Back)()=0;
bool critical_section_that_prevents_temperature_timeout = false;