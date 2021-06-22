#include "handlersPrg.h"
#include "settings.h"
#include "utils.h"

extern TimeChangeRule* tcr;
extern HomieNode* prg_node[NUMBER_OF_PROGRAMS];

void onProgramStart(unsigned char progId){
    if (progId >= NUMBER_OF_PROGRAMS) return;
    // update Homie property
    if (progId < NUMBER_OF_PROGRAMS && Homie.isConnected()) prg_node[progId]->setProperty("status").send(boolStr(true));
    Homie.getLogger() << nowStr(tcr->abbrev) << " Program " << progId << " STARTED" << endl;
}

void onProgramStop(unsigned char progId){
    if (progId >= NUMBER_OF_PROGRAMS) return;
    // update Homie property
    if (Homie.isConnected()) prg_node[progId]->setProperty("status").send(boolStr(false));
    Homie.getLogger() << nowStr(tcr->abbrev) << " Program " << progId << " ENDED" << endl;
}
