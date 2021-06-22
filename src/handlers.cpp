#include "handlers.h"
#include "Program.h"
#include "Valve.h"
#include "utils.h"

//#define NODEBUG_PRINT
#include "debug_print.h"

extern TimeChangeRule* tcr;
extern Program* programs[NUMBER_OF_PROGRAMS];
extern Valve* valves[NUMBER_OF_VALVES];

bool updateHandler(const HomieNode &node, const HomieRange &range, const String &property, const String &value){
    bool updated = false;
    DEBUG_PRINT("[updateHandler] node=%s pro=%s value=%s\n",node.getId(), property.c_str(), value.c_str());

    if (strcmp_P(node.getType(),PSTR("program")) == 0){
        unsigned char prgIdx = *(node.getId()+3)-'1';  // convert program # to number : assuming prg#
        if (property.startsWith(F("day"))){
            unsigned char dayIdx = property.charAt(3)-'1';
            programs[prgIdx]->setRunDay(dayIdx,value=="true");
            updated = true;
        }
        if (property.startsWith(F("rtvalve"))){
            unsigned char valveIdx = property.charAt(7)-'1';
            unsigned int rt = value.toInt();
            programs[prgIdx]->setRunTime(valveIdx, rt);
            updated = true;
        }

        if (property == "startHour"){
            programs[prgIdx]->setStart(value.toInt(),programs[prgIdx]->getStartMinute());
            updated = true;
        }

        if (property == "startMin"){
            programs[prgIdx]->setStart(programs[prgIdx]->getStartHour(), value.toInt());
            updated = true;
        }

        if (property == "status"){
            if (value == "true") programs[prgIdx]->start();
            else programs[prgIdx]->stop();
            updated = true;
        }


    }

    if (updated) {
        if (Homie.isConnected()) node.setProperty(property).send(value);
        Homie.getLogger() << nowStr(tcr->abbrev) << " Node=" << node.getId() << " property=" << property << " set to " << value << endl;
        // TODO
        // save to local config
    }

    return updated;
}