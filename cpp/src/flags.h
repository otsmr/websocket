/*
 * Copyright (c) 2022, Tobias <git@tsmr.eu>
 * 
 */

#pragma once

// -- compile options
#define COMPILE_FOR_FUZZING 1
#define ARTIFICIAL_BUGS     1 // to be sure that the fuzzer can find some bugs :^)

#define NOFORK  (COMPILE_FOR_FUZZING)

#define DEBUG_LEVEL 7
// --



enum DebugLevel {
    Emergency,      // emerg
    Alerts,         // alert
    Critical,       // crit
    Errors,         // err
    Warnings,       // warn
    Notification,   // notice
    Information,    // info
    Debug,          // debug 
};

#define USEFORK (!NOFORK)

#if COMPILE_FOR_FUZZING
extern char * g_fuzzing_input_file;
#endif