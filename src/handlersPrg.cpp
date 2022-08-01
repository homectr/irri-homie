#include "handlersPrg.h"
#include "settings.h"
#include "utils.h"

extern TimeChangeRule *tcr;

void onProgramStart(Program *prg)
{
  if (!prg)
    return;
  // update Homie property
  if (Homie.isConnected())
    prg->getHomie()->setProperty("status").send(boolStr(true));
  Homie.getLogger() << nowStr(tcr->abbrev) << " Program " << prg->getIdStr() << " STARTED" << endl;
}

void onProgramStop(Program *prg)
{
  if (!prg)
    return;
  // update Homie property
  if (Homie.isConnected())
    prg->getHomie()->setProperty("status").send(boolStr(false));
  Homie.getLogger() << nowStr(tcr->abbrev) << " Program " << prg->getIdStr() << " ENDED" << endl;
}
