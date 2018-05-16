/*
 * ProductDebug.cpp
 *
 *  Created on: 14.06.2016
 *      Author: nid
 */

#include <Arduino.h>
#include <Timer.h>
#include <SerialCommand.h>
#include <DbgCliNode.h>
#include <DbgCliTopic.h>
#include <DbgCliCommand.h>
#include <DbgTraceContext.h>
#include <DbgTracePort.h>
#include <DbgTraceLevel.h>
#include <DbgPrintConsole.h>
#include <DbgTraceOut.h>
#ifdef ESP8266
extern "C"
{
  #include "user_interface.h"
}
#else
#include <RamUtils.h>
#endif
#include <AppDebug.h>

//-----------------------------------------------------------------------------
// HW Serial1 available?
//-----------------------------------------------------------------------------
#if defined(__AVR_ATmega328P__)  // Arduino UNO, NANO have no HW Serial1
#else
  #define HW_Serial1_available
#endif

//-----------------------------------------------------------------------------
// Free Heap Logger
//-----------------------------------------------------------------------------
const unsigned long c_freeHeapLogIntervalMillis = 10000;

class FreeHeapLogTimerAdapter : public TimerAdapter
{
private:
  DbgTrace_Port* m_trPort;
public:
  FreeHeapLogTimerAdapter()
  : m_trPort(new DbgTrace_Port("heap", DbgTrace_Level::info))
  { }

  void timeExpired()
  {
    int freeHeap = 0;
#ifdef ESP8266
    freeHeap = system_get_free_heap_size();
#else
    freeHeap = RamUtils::getFreeRam();
#endif
    TR_PRINTF(m_trPort, DbgTrace_Level::debug, "%d bytes free", freeHeap);
  }
};

//-----------------------------------------------------------------------------

extern SerialCommand* sCmd;

const unsigned long int baudRate = 9600;

class DbgPrint_Console1 : public DbgPrint_Console
{
public:
  virtual void print(const char* str)
  {
#ifdef ARDUINO
  #if defined(HW_Serial1_available)
    Serial1.println(str);
  #endif
#else
    printf("%s\n", str);
#endif
  }
};

void setupDebugEnv()
{
  //-----------------------------------------------------------------------------
  // Serial Command Object for Debug CLI
  //-----------------------------------------------------------------------------
  Serial.begin(baudRate);
#if defined(HW_Serial1_available)
  Serial1.begin(baudRate);
#endif
  sCmd = new SerialCommand();
  DbgCli_Node::AssignRootNode(new DbgCli_Topic(0, "dbg", "Wiring Controller Debug CLI Root Node."));

  // Setup callbacks for SerialCommand commands
  if (0 != sCmd)
  {
    sCmd->addCommand("dbg", dbgCliExecute);
    sCmd->addCommand("hello", sayHello);        // Echos the string argument back
    sCmd->setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")
  }

  //---------------------------------------------------------------------------
  // Debug Trace
  //---------------------------------------------------------------------------
  new DbgTrace_Context(new DbgCli_Topic(DbgCli_Node::RootNode(), "tr", "Modify debug trace"));
  new DbgTrace_Out(DbgTrace_Context::getContext(), "trConOut", new DbgPrint_Console());
#if defined(HW_Serial1_available)
  new DbgTrace_Out(DbgTrace_Context::getContext(), "trConOut1", new DbgPrint_Console1());
#endif

  //-----------------------------------------------------------------------------
  // Free Heap Logger
  //-----------------------------------------------------------------------------
  new Timer(new FreeHeapLogTimerAdapter(), Timer::IS_RECURRING, c_freeHeapLogIntervalMillis);
}

void dbgCliExecute()
{
  if ((0 != sCmd) && (0 != DbgCli_Node::RootNode()))
  {
    const unsigned int firstArgToHandle = 1;
    const unsigned int maxArgCnt = 10;
    char* args[maxArgCnt];
    char* arg = const_cast<char*>("dbg");
    unsigned int arg_cnt = 0;
    while ((maxArgCnt > arg_cnt) && (0 != arg))
    {
      args[arg_cnt] = arg;
      arg = sCmd->next();
      arg_cnt++;
    }
    DbgCli_Node::RootNode()->execute(static_cast<unsigned int>(arg_cnt), const_cast<const char**>(args), firstArgToHandle);
  }
}

void sayHello()
{
  char *arg;
  if (0 != sCmd)
  {
    arg = sCmd->next();    // Get the next argument from the SerialCommand object buffer
  }
  else
  {
    arg = const_cast<char*>("");;
  }
  if (arg != NULL)         // As long as it exists, take it
  {
    Serial.print("Hello ");
    Serial.println(arg);
  }
  else
  {
    Serial.println("Hello, whoever you are");
  }
}

// This is the default handler, and gets called when no other command matches.
void unrecognized(const char *command)
{
  Serial.println("What?");
}
