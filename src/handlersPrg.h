#pragma once

#include <Arduino.h>
#include <Homie.h>
#include "Program.h"

void onProgramStart(Program* program);
void onProgramStop(Program* program);

bool handleProgramStatus(unsigned char progId, const String& value);
bool handleProgramName(unsigned char progId, const String& value);
bool handleProgramStartHour(unsigned char progId, const String& value);
bool handleProgramStartMin(unsigned char progId, const String& value);
bool handleProgramRunDays(unsigned char progId, unsigned char day, bool value);
bool handleProgramRunTimes(unsigned char progId, unsigned char valve, unsigned char runtime);

