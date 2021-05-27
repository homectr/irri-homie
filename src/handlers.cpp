#include "handlers.h"

#define NODEBUG_PRINT
#include "debug_print.h"

bool updateHandler(const HomieNode &node, const HomieRange &range, const String &property, const String &value){
    DEBUG_PRINT("[updateHandler] node=%s pro=%s value=%s\n",node.getId(), property.c_str(), value.c_str());
    return true;
}