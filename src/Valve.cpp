#include <Arduino.h>
#include "Valve.h"

#define NODEBUG_PRINT
#include "debug_print.h"

Valve::Valve(unsigned char id){
    this->id = id;
}

void Valve::close(){
    status = 0;
    DEBUG_PRINT("Closing valve %d\n",this->id);
    if (onClose) onClose(id);

}

void Valve::open(unsigned int minutes){
    if (minutes == 0) { 
        close();
        return;
    }
    unsigned int min = minutes > 120 ? 120 : minutes;
    runtime =  min * 60000;
    openedAt = millis();
    status = 1;
    DEBUG_PRINT("Opening valve %d for %d minutes\n",this->id, min);
    if (onOpen) onOpen(id);
}

void Valve::loop(){
    if (status == 0) return;
    if (millis() - this->openedAt > this->runtime) this->close();
}