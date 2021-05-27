#include "handlersPrg.h"
#include "settings.h"
#include "Program.h"

extern Program* programs[NUMBER_OF_PROGRAMS];
extern const String opts;
extern const unsigned char negativeOpts;
extern HomieNode* prg_node[NUMBER_OF_PROGRAMS];

void onProgramStart(unsigned char progId){
    if (progId >= NUMBER_OF_PROGRAMS) return;
    // update Homie property
    if (progId < NUMBER_OF_PROGRAMS && Homie.isConnected()) prg_node[progId]->setProperty("status").send("ON");
    Homie.getLogger() << "Program " << progId << " is ON" << endl;
}

void onProgramStop(unsigned char progId){
    if (progId >= NUMBER_OF_PROGRAMS) return;
    // update Homie property
    if (Homie.isConnected()) prg_node[progId]->setProperty("status").send("OFF");
    Homie.getLogger() << "Program " << progId << " is OFF" << endl;
}

bool handleProgramStatus(unsigned char progId, const String& value){
    if (progId >= NUMBER_OF_PROGRAMS) return false;

    int8_t i = opts.indexOf(value);
    if (0 > i) return false;  // if the value is not valid
  
    bool on = (i < negativeOpts); // if one of the positive options
    if (on) programs[progId]->start();
    else programs[progId]->stop();

    return true;
}

bool handleProgramName(unsigned char progId, const String& value){
    if (progId >= NUMBER_OF_PROGRAMS) return false;  // if it's not a valid range

    programs[progId]->setName(value.c_str());
    if (Homie.isConnected()) prg_node[progId]->setProperty("name").send(value);
    Homie.getLogger() << "Program " << progId << " name set to " << value << endl;

    return true;
}

bool handleProgramStartHour(unsigned char progId, const String& value){
    if (progId >= NUMBER_OF_PROGRAMS) return false;  // if it's not a valid range

    int h = value.toInt();
    programs[progId]->setStart(h, programs[progId]->getStartMinute());
    if (Homie.isConnected()) prg_node[progId]->setProperty("starthour").send(String(h));
    Homie.getLogger() << "Program " << progId << " start hour set to " << h << endl;

    return true;
}

bool handleProgramStartMin(unsigned char progId, const String& value){
    if (progId >= NUMBER_OF_PROGRAMS) return false;  // if it's not a valid range

    int m = value.toInt();
    programs[progId]->setStart(programs[progId]->getStartHour(), m);
    if (Homie.isConnected()) prg_node[progId]->setProperty("startmin").send(String(m));
    Homie.getLogger() << "Program " << progId << " start minute set to " << m << endl;

    return true;
}

bool handleProgramRunDays(unsigned char progId, unsigned char day, bool value){
    if (progId >= NUMBER_OF_PROGRAMS || day > 6) return false;  // if it's not a valid range

    if (!programs[progId]->setRunDay(day, value)) return false;
    String name = "day"+String(day);
    String v = programs[progId]->getRunDay(day) ? "ON":"OFF";
    if (Homie.isConnected()) prg_node[progId]->setProperty(name.c_str()).send(v);
    Homie.getLogger() << "Program " << progId << " run day " << day << " set to " << v << endl;

    return true;
}

bool handleProgramRunTimes(unsigned char progId, unsigned char valve, unsigned char runtime){
    if (progId >= NUMBER_OF_PROGRAMS) return false;  // if it's not a valid range

    if (!programs[progId]->setRunTime(valve, runtime)) return false;
    String name = "valve"+String(valve);
    unsigned char rt = programs[progId]->getRunTime(valve);  
    if (Homie.isConnected()) prg_node[progId]->setProperty(name.c_str()).send(String(rt));
    Homie.getLogger() << "Program " << progId << " valve " << valve << " run time set to " << rt << endl;

    return true;
}