#include "handlers.h"
#include "Program.h"
#include "Valve.h"
#include "utils.h"
#include "config.h"

#define NODEBUG_PRINT
#include "debug_print.h"

extern TimeChangeRule *tcr;
extern Program *programs[NUMBER_OF_PROGRAMS];
extern Valve *valves[NUMBER_OF_VALVES];

bool updateHandler(const HomieNode &node, const HomieRange &range, const String &property, const String &value)
{
  bool updated = false;
  String newValue = value;
  DEBUG_PRINT("[updateHandler] node=%s pro=%s value=%s\n", node.getId(), property.c_str(), value.c_str());

  if (strcmp_P(node.getType(), PSTR("program")) == 0)
  {
    Program *program = findProgramById(node.getId());
    if (!program)
      return false;

    if (property.startsWith(F("day")))
    {
      unsigned char dayIdx = property.charAt(3) - '1';
      program->setRunDay(dayIdx, value == "true");
      newValue = boolStr(program->getRunDay(dayIdx));
      updated = true;
    }
    if (property.startsWith(F("rtvalve")))
    {
      unsigned char valveIdx = property.charAt(7) - '1';
      unsigned int rt = value.toInt();
      rt = rt > 120 ? 120 : rt; // max 2 hours
      program->setRunTimeMin(valveIdx, rt);
      newValue = String(program->getRunTimeMin(valveIdx));
      updated = true;
    }

    if (property == "startHour")
    {
      unsigned char h = value.toInt();
      program->setStart(h, program->getStartMinute());
      newValue = String(h);
      updated = true;
    }

    if (property == "startMin")
    {
      unsigned char m = value.toInt();
      program->setStart(program->getStartHour(), m);
      newValue = String(m);
      updated = true;
    }

    if (property == "status")
    {
      if (value == "true")
        program->start();
      else
        program->stop();
      newValue = boolStr(program->isRunning());
      updated = true;
    }
  }

  if (strcmp_P(node.getType(), PSTR("valve")) == 0)
  {
    Valve *valve = findValveById(node.getId());
    if (!valve)
      return false;

    if (property == "runtime")
    {
      unsigned int rt = value.toInt();
      rt = rt > 120 ? 120 : rt; // max 2 hours
      valve->setRunTimeMin(rt);
      newValue = valve->getRunTimeMin();
      updated = true;
    }

    if (property == "status")
    {
      if (value == "true")
        valve->open();
      else
        valve->close();
      newValue = boolStr(valve->isOpen());
      updated = true;
    }
  }

  if (updated)
  {
    if (Homie.isConnected())
      node.setProperty(property).send(newValue);
    Homie.getLogger() << nowStr(tcr->abbrev) << " Node=" << node.getId() << " property=" << property << " set to " << newValue << endl;
    saveConfig();
  }

  return updated;
}