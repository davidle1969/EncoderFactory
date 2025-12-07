#include "utilities.h"

#include <regex>
#include <format>
#include <fstream>
#include <filesystem> // Required for std::filesystem
#include <cstdio> // Required for remove()

#include "logger.h"

using namespace std;
namespace fs = std::filesystem;

string remove_dates_from_filename(const string& filename)
{
    // This function removes date patterns like " (2023)" from the filename
    std::regex date_pattern(R"( \(\d{4}\))");
    return std::regex_replace(filename, date_pattern, "");
}

int touch_file(const string& filepath)
{
    ofstream file(filepath);
    if (!file.is_open())
    {
        log(DEBUG, "Failed to touch file: " + filepath);
        return -1;
    }
    file.close();
    return 0;
}

int append_to_file(const string& filepath, const string& content)
{
    ofstream file(filepath, ios::app);
    if (!file.is_open())
    {
        log(DEBUG, "Failed to append to file: " + filepath);
        return -1;
    }
    file << content;
    file.close();
    return 0;
}


bool pathExists(const string& path) 
{
    fs::path filePath(path);
    return pathExists(filePath);
}

bool pathExists(const fs::path filePath) 
{

    // 1. Check if the file exists
    if (!fs::exists(filePath)) 
    {
//        log(DEBUG, "path does not exist: " + filePath.string());
        return false;
    }

    // 2. Check if it's a regular file (not a directory, etc.)
    if (!fs::is_directory(filePath)) 
    {
//        log(DEBUG, "Path is not a regular directory: " + filePath.string());
        return false;
    }
    return true; // Path exists and is a directory
}


bool fileExists(const std::string& filename, bool checkSize) 
{
    fs::path filePath(filename);

    return fileExists(filePath, checkSize);
}

bool fileExists(const fs::path& filePath, bool checkSize) 
{
     // 1. Check if the file exists
    if (!fs::exists(filePath)) 
    {
//        log(DEBUG, "File does not exist: " + filePath.string());
        return false;
    }

    // 2. Check if it's a regular file (not a directory, etc.)
    if (!fs::is_regular_file(filePath)) 
    {
//        log(DEBUG, "File is not a regular file: " + filePath.string());
        return false;
    }

    // 3. Check if the file is empty
    if (checkSize && fs::file_size(filePath) == 0) 
    {
//        log(DEBUG, "File exists but is empty: " + filePath.string());
        return false;
    }

    return true; // File exists and is not empty
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

int64_t string_to_long(const std::string& sNumber )
{
//    log(DEBUG, "string_to_long: " + sNumber);
//    char* p;

    return strtol(sNumber.c_str(), NULL, 10);


/*    if( is_number(sNumber))
        return stol(sNumber);

    return 0;
*/
}


int64_t get_filesize(const string& _file)
{
	int64_t size=0;
    fs::path file(_file);
	if (fs::exists(file) && fs::is_regular_file(file)) {
        size = fs::file_size(file);
        //log(DEBUG, "get_filesize:  " + _file + " " + to_string(size));
    }
	return size;
}




int count_files_in_directory(const string& directoryPath)
{
    if( pathExists(directoryPath) == false)
        return false;
    
    int count = 0;

    try {
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                count++;
            }
        }
    } catch (const fs::filesystem_error& e) {
        log(ERROR, "count_files_in_directory: " + string(e.what()));
        return -1;
    }

    return count;
}


string execute_command(const string& command) 
{
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        // Handle error
        return "";
    }
    char buffer[128];
    string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL) {
            result += buffer;
        }
    }

//    log(DEBUG, "execute_command: " + command + " Result: " + result);
    pclose(pipe);
    return result;
}

string extractValue(const std::string& str, const std::string& key) 
{
    size_t start_pos = str.find(key);

    if (start_pos == std::string::npos) {
        // If the prefix is not found, return an empty string or handle as necessary
        return ""; 
    }

    start_pos += key.length(); // Move to the start of the codec string
    size_t end_pos = str.find_first_of(" \n\t", start_pos); // Find end position (space, newline, or tab)

    // If no space is found, use the entire remaining string
    if (end_pos == std::string::npos) {
        end_pos = str.length(); // use the length of the string
    }

    // Extract the substring which is the codec
    return str.substr(start_pos, end_pos - start_pos);
}


int64_t convertTimeToSeconds(const std::string& timeStr) 
{
    std::istringstream ss(timeStr);
    int hours, minutes, seconds;

    // Split the input string based on ':' and parse into hours, minutes, seconds
    char delimiter;
    ss >> hours >> delimiter >> minutes >> delimiter >> seconds;

    // Calculate total seconds
    return hours * 3600 + minutes * 60 + seconds;
}



int move_file(const string& src_file, const string& dest_path) 
{
    try {
        // Construct the destination file path by appending the source file name
        fs::path source = src_file;
        fs::path destination = fs::path(dest_path) / source.filename();

        // Move the file
        fs::rename(source, destination);
        return 0;  // Indicates success
    } catch (const fs::filesystem_error& e) {
        log(ERROR, "move_file: " + string(e.what()));
        return -1; // Indicates failure
    }
}

int delete_file(const string& filename)
{
    if( fileExists(filename, false) ) 
        return remove(filename.c_str());

    return -1;    
}

