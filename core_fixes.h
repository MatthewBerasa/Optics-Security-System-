#ifndef CORE_FIXES_H
#define CORE_FIXES_H

// Save the original swap macro if it exists
#ifdef swap
    #define ORIGINAL_SWAP swap
    #undef swap
#endif

// Include all problematic headers here
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Restore the original swap macro if needed
#ifdef ORIGINAL_SWAP
    #define swap ORIGINAL_SWAP
    #undef ORIGINAL_SWAP
#endif

#endif