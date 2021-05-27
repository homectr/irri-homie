#include "handlersPrg.h"
#include "settings.h"
#include "Program.h"

extern Program* programs[NUMBER_OF_PROGRAMS];
extern const String opts;
extern const unsigned char negativeOpts;
extern HomieNode* prg_node;

void onProgramStart(unsigned char progId){
    // update Homie property
    if (Homie.isConnected()) prg_node->setProperty("status").setRange(progId).send("ON");
    Homie.getLogger() << "Program " << progId << " is ON" << endl;
}

void onProgramStop(unsigned char progId){
    // update Homie property
    if (Homie.isConnected()) prg_node->setProperty("status").setRange(progId).send("OFF");
    Homie.getLogger() << "Program " << progId << " is OFF" << endl;
}

bool handleProgramStatus(const HomieRange& range, const String& value){
    if (!range.isRange) return false;  // if it's not a range
    if (range.index < 1 || range.index > NUMBER_OF_PROGRAMS) return false;  // if it's not a valid range

    int8_t i = opts.indexOf(value);
    if (0 > i) return false;  // if the value is not valid

    unsigned char progId = range.index - 1;
  
    bool on = (i < negativeOpts); // if one of the positive options
    if (on) programs[progId]->start();
    else programs[progId]->stop();

    return true;
}

bool handleProgramName(const HomieRange& range, const String& value){
    if (!range.isRange) return false;  // if it's not a range
    if (range.index < 1 || range.index > NUMBER_OF_PROGRAMS) return false;  // if it's not a valid range

    programs[range.index]->setName(value.c_str());
    if (Homie.isConnected()) prg_node->setProperty("name").setRange(range).send(value);
    Homie.getLogger() << "Program " << range.index << " name set to " << value << endl;

    return true;
}

bool handleProgramStartHour(const HomieRange& range, const String& value){
    if (!range.isRange) return false;  // if it's not a range
    if (range.index < 1 || range.index > NUMBER_OF_PROGRAMS) return false;  // if it's not a valid range

    int h = value.toInt();
    programs[range.index]->setStart(h, programs[range.index]->getStartMinute());
    if (Homie.isConnected()) prg_node->setProperty("starthour").setRange(range).send(String(h));
    Homie.getLogger() << "Program " << range.index << " start hour set to " << h << endl;

    return true;
}

bool handleProgramStartMin(const HomieRange& range, const String& value){
    if (!range.isRange) return false;  // if it's not a range
    if (range.index < 1 || range.index > NUMBER_OF_PROGRAMS) return false;  // if it's not a valid range

    int m = value.toInt();
    programs[range.index]->setStart(programs[range.index]->getStartHour(), m);
    if (Homie.isConnected()) prg_node->setProperty("startmin").setRange(range).send(String(m));
    Homie.getLogger() << "Program " << range.index << " start minute set to " << m << endl;

    return true;
}

bool handleProgramRunDays(const HomieRange& range, const String& value){
    if (!range.isRange) return false;  // if it's not a range
    if (range.index < 1 || range.index > NUMBER_OF_PROGRAMS) return false;  // if it's not a valid range

    if (!programs[range.index]->setRunDays(value.c_str())) return false;
    String rd = programs[range.index]->getRunDays();
    if (Homie.isConnected()) prg_node->setProperty("rundays").setRange(range).send(rd);
    Homie.getLogger() << "Program " << range.index << " run days set to " << rd << endl;

    return true;
}

bool handleProgramRunTimes(const HomieRange& range, const String& value){
    if (!range.isRange) return false;  // if it's not a range
    if (range.index < 1 || range.index > NUMBER_OF_PROGRAMS) return false;  // if it's not a valid range

    if (!programs[range.index]->setRunTimes(value.c_str())) return false;
    String rt = programs[range.index]->getRunDays();  
    if (Homie.isConnected()) prg_node->setProperty("rundays").setRange(range).send(rt);
    Homie.getLogger() << "Program " << range.index << " run days set to " << rt << endl;

    return true;
}