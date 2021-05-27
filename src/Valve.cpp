#include <Arduino.h>
#include "Valve.h"

//#define NODEBUG_PRINT
#include "debug_print.h"

Valve::Valve(unsigned char id){
    this->id = id;
}

void Valve::close(){
    status = 0;
    DEBUG_PRINT("Closing valve %d\n", id);
    if (onClose) onClose(id);
}

void Valve::open(unsigned int seconds, unsigned char intensity){
    DEBUG_PRINT("Requested opening valve %d for %d seconds @%d intensity \n", id, seconds, intensity);
    unsigned long t = (seconds * intensity)/100;
    if (t == 0) return;
    
    runtime =  (t > 7200 ? 7200 : t) * 1000;
    openedAt = millis();
    status = 1;
    DEBUG_PRINT("Opening valve %d for %d seconds\n", id, runtime/1000);

    if (onOpen) onOpen(id);
}

void Valve::loop(){
    DEBUG_PRINT("Loop:valve id=%d status=%d\n",id, status);
    if (status == 0) return;
    if (millis() - openedAt > runtime) close();
}