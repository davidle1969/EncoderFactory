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
        log(ERROR, "Failed to touch file: " + filepath);
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
        log(ERROR, "Failed to append to file: " + filepath);
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
        log(ERROR, "path does not exist: " + filePath.string());
        return false;
    }

    // 2. Check if it's a regular file (not a directory, etc.)
    if (!fs::is_directory(filePath)) 
    {
        log(ERROR, "Path is not a regular directory: " + filePath.string());
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
        log(ERROR, "File does not exist: " + filePath.string());
        return false;
    }

    // 2. Check if it's a regular file (not a directory, etc.)
    if (!fs::is_regular_file(filePath)) 
    {
        log(ERROR, "File is not a regular file: " + filePath.string());
        return false;
    }

    // 3. Check if the file is empty
    if (checkSize && fs::file_size(filePath) == 0) 
    {
        log(ERROR, "File exists but is empty: " + filePath.string());
        return false;
    }

    return true; // File exists and is not empty
}



int32_t get_filesize(const string& _file)
{
	int32_t size=0;
    fs::path file(_file);
	if (fs::exists(file) && fs::is_regular_file(file)) {
        size = fs::file_size(file);
    }
	return size;
}




int count_files_in_directory(const string& directoryPath)
{
    
    int count = 0;

    try {
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (fs::is_regular_file(entry.status())) {
                count++;
            }
        }
    } catch (const fs::filesystem_error& e) {
        log(ERROR, "Filesystem error: " + string(e.what()));
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

    log(INFO, "execute_command: " + command + " Result: " + result);
    pclose(pipe);
    return result;
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
        return -1; // Indicates failure
    }
}

int delete_file(const string& filename)
{
    if( fileExists(filename, false) ) 
        return remove(filename.c_str());

    return -1;
    
}

