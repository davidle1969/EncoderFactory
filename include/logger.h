#ifndef LOGGER_H
#define LOGGER_H

#include <string>

enum LogLevel {
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4
};

extern int _logLevel;

// Define a macro for easier access
#define DEBUG LogLevel::DEBUG
#define INFO LogLevel::INFO
#define WARNING LogLevel::WARNING
#define ERROR LogLevel::ERROR

//#define INFO 1
//#define WARNING 2
//#define ERROR 3

//int openlogfile();
//int closelogfile();
int log(int nlevel, const char* sMessage);
int log(int nlevel, std::string sMessage);
int log(int nlevel, const char* sMessage, std::string sValue);
int log(int nlevel, const char* sMessage, const char* sValue);
int log(int nlevel, const char* sMessage, int value);

std::string get_current_timestamp();
void setLogLevel(int level);
int getLogLevel();



#endif // LOGGER_H