#pragma once

#include <Homie.h>
#include <TimeLib.h>

#include "Program.h"
#include "Valve.h"

struct ProgramListEntry {
    Program* prg;
    ProgramListEntry* next;
};

struct ValveListEntry {
    Valve* valve;
    ValveListEntry* next;
};

class System {

    private:
        ValveListEntry* valves;
        ProgramListEntry* programs;

        unsigned char intensity = 100;
        time_t disabledTill = 0;

    public:
        void loadConfig();

        void addValve(Valve* valve);
        Valve* getValve(unsigned char idx);
        void addProgram(Program* valve);
        Valve* getProgram(unsigned char idx);


};
