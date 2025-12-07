#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <cstdint>
#include <filesystem> // Required for std::filesystem
namespace fs = std::filesystem;

std::string remove_dates_from_filename(const std::string& filename);
int touch_file(const std::string& filepath);
int count_files_in_directory(const std::string& directoryPath);
int append_to_file(const std::string& filepath, const std::string& content);


bool pathExists(const std::string& path);
bool pathExists(const std::filesystem::path filePath);
bool fileExists(const std::string& filename, bool checkSize = false);
bool fileExists(const std::filesystem::path& filePath, bool checkSize = false) ;

bool is_number(const std::string& s);
int64_t string_to_long(const std::string& sNumber );
int64_t get_filesize(const std::string& file);



std::string execute_command(const std::string& command);

int move_file(const std::string& src_file, const std::string& dest);
int delete_file(const std::string& filename);
std::string extractValue(const std::string& str, const std::string& key) ;
int64_t convertTimeToSeconds(const std::string& timeStr); 



#endif // UTILITIES_H