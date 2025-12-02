#include <stdio.h>
#include <iostream> 
#include <ctime>
#include <string>
#include <regex>
#include <format>
#include <fstream>
#include <filesystem> // Required for std::filesystem

#include "logger.h"

//#define INFO 1
//#define WARNING 2
//#define ERROR 3

using namespace std;

//static ofstream outfile;
/*
int openlogfile()
{
    static ofstream outfile("./log/error.log", ios::app);
    if (!outfile.is_open())
    {
        cout << "Failed to open log file." << endl;
        return -1;
    }
    return 0;
}

int closelogfile()
{
    static ofstream outfile;
    if (outfile.is_open())
    {
        outfile.close();
        return 0;
    }
    return -1;
}
*/
int log(int nlevel, string sMessage)
{
    return log(nlevel, sMessage.c_str());
}

int log(int nlevel, const char* sMessage, string sValue)
{
    return log(nlevel, sMessage, sValue.c_str());
}


int log(int nlevel, const char* sMessage, const char* sValue)
{
    char buffer[1024];
    snprintf(buffer, 1024, "%s %s\n", sMessage, sValue);
    return log(nlevel, buffer);
}

int log(int nlevel, const char* sMessage, int value)
{
    char buffer[1024];
    snprintf(buffer, 1024, "%s %d\n", sMessage, value);
    return log(nlevel, buffer);
}


int log(int nlevel, const char* sMessage)
{
 	string level_string = "UNKNOWN";

    // Map numeric log level to string
    switch (nlevel)
    {
        case INFO:
            level_string="INFO";
            break;
        case WARNING:
            level_string="WARNING";
            break;
        case ERROR:
            level_string="ERROR";
            break;
        default:
            level_string="UNKNOWN";
            break;
    }   	
	char buffer[1024];
    string currenttime = get_current_timestamp();
    // Handle multiline messages by using printf
    snprintf(buffer, 1024, "[%s] [%s]: %s\n", currenttime.c_str(), level_string.c_str(), sMessage);

    cout<< buffer;
//    openlogfile();
    

    if (nlevel == ERROR)
    {
        static ofstream outfile("./log/error.log", ios::app);        
        if (outfile.is_open() == false)
        {
            cerr << "Log file is not open!" << endl;
            return -1;
        }

        outfile << buffer << std::endl;
    }
    return 0;

}

string get_current_timestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
    return std::string(buffer);

}


