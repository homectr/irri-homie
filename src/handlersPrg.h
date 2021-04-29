#pragma once

#include <Arduino.h>
#include <Homie.h>

void onProgramStart(unsigned char progId);
void onProgramStop(unsigned char progId);

bool handleProgramStatus(const HomieRange& range, const String& value);
bool handleProgramName(const HomieRange& range, const String& value);
bool handleProgramStartHour(const HomieRange& range, const String& value);
bool handleProgramStartMin(const HomieRange& range, const String& value);
bool handleProgramRunDays(const HomieRange& range, const String& value);
bool handleProgramRunTimes(const HomieRange& range, const String& value);

