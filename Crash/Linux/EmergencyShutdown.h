#ifndef EMERGENCY_SHUTDOWN_H
#define EMERGENCY_SHUTDOWN_H

#include "../crashDefines.h"


void RegisterEmergencyShutdownCallback( void (*pFunc)() );
void DoEmergencyShutdown();

#endif

