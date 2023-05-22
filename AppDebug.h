/*
 * AppDebug.h
 *
 *  Created on: 24.03.2017
 *      Author: nid
 */

#ifndef LIB_APP_DEBUG_APPDEBUG_H_
#define LIB_APP_DEBUG_APPDEBUG_H_

class SerialCommand;

class AppDebug
{
public:
  AppDebug(SerialCommand* sCmd);
  virtual ~AppDebug();

  void setupDebugEnv();

private:
  static void dbgCliExecute();
  static void sayHello();
  static void unrecognized(const char *command);

private:
  static SerialCommand* m_sCmd;
};

#endif /* LIB_APP_DEBUG_APPDEBUG_H_ */
