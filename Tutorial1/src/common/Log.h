#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>

/**
* Define-uri pt. mesaje de log;
* Functiile de logging functioneaza 
* numai in modul debug; au aceasi sintaxa ca printf()
*/
#ifdef _DEBUG
#define LogMessage		printf
#define LogError		printf
#define LogWarning		printf
#define LogFatalError	printf
#else
#define LogMessage
#define LogError
#define LogWarning
#define LogFatalError
#endif

#endif /*LOG_H_*/
