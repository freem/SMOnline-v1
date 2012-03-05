//Crash.cpp
//
//Ported away from Glenn Maynard's Rage.
//Self contained crash reporting.

// DO NOT USE stdio.h!  printf() calls malloc()!
//#include <stdio.h>

#include "crash.h"

#include <iostream> //For this specific implementation
using namespace std;

static void DoSave();

bool g_bAutoRestart = false;
