#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#define INFO 1
#define WARNING 2
#define ERROR 3

//int openlogfile();
//int closelogfile();
int log(int nlevel, const char* sMessage);
int log(int nlevel, std::string sMessage);
int log(int nlevel, const char* sMessage, std::string sValue);
int log(int nlevel, const char* sMessage, const char* sValue);
int log(int nlevel, const char* sMessage, int value);

std::string get_current_timestamp();


#endif // LOGGER_H