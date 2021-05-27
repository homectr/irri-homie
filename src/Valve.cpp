#include <Arduino.h>
#include "Valve.h"

#define NODEBUG_PRINT
#include "debug_print.h"

Valve::Valve(unsigned char id){
    this->id = id;
}

void Valve::close(){
    status = 0;
    DEBUG_PRINT("Closing valve %d\n", id);
    if (onClose) onClose(id);
}

void Valve::open(unsigned int minutes, unsigned char intensity){
    DEBUG_PRINT("Requested opening valve %d for %d minutes @%d intensity \n", id, minutes, intensity);
    float t = minutes * (intensity/100);
    if (t == 0) { 
        close();
        return;
    }
    unsigned int min = t > 120 ? 120 : t;
    runtime =  min * 60000;
    openedAt = millis();
    status = 1;
    DEBUG_PRINT("Opening valve %d for %d minutes\n", id, min);
    if (onOpen) onOpen(id);
}

void Valve::loop(){
    DEBUG_PRINT("Loop:valve id=%d status=%d\n",id, status);
    if (status == 0) return;
    if (millis() - openedAt > runtime) close();
}