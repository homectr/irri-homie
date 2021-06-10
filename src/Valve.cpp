#include <Arduino.h>
#include "Valve.h"

#define NODEBUG_PRINT
#include "debug_print.h"

Valve::Valve(unsigned char id, unsigned char inverse){
    this->id = id;

    char is[20];
    snprintf(is,20,"valve%d",id);
    idStr = strdup(is);

    snprintf(is,20,"Valve %d",id);
    name = strdup(is);

    this->inverse = inverse;
    status = 0;
}

void Valve::close(){
    status = 0;
    DEBUG_PRINT("Closing valve %d\n", id);
    if (onClose) onClose(id, inverse);
}

void Valve::open(unsigned int seconds, unsigned char intensity){
    DEBUG_PRINT("Requested opening valve %d for %d seconds @%d intensity \n", id, seconds, intensity);
    unsigned long t = (seconds * intensity)/100;
    if (t == 0) return;
    
    runtime =  (t > 7200 ? 7200 : t) * 1000;
    openedAt = millis();
    status = 1;
    DEBUG_PRINT("Opening valve %d for %d seconds\n", id, runtime/1000);

    if (onOpen) onOpen(id, inverse);
}

void Valve::loop(){
    DEBUG_PRINT("Loop:valve id=%d status=%d\n",id, status);
    if (!isOpen()) return;
    if (millis() - openedAt > runtime) close();
}