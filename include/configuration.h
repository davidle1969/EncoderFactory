#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <iostream>
#include <string> // Required for std::string
#include <vector>
#include <yaml-cpp/yaml.h>

using namespace std;
//using std::string;
//namespace fs = std::filesystem;

struct encode_struct
{
    /* data */
    int consolidate;
	int error_logging;
	
	int64_t bitrate;
    int64_t audio_bitrate;
	int64_t max_size;	
	int64_t trash_file_size;

    std::string encode_string;
	std::string decode_string;
    std::string addon_string;
	std::string config_file;

    std::string trash_path;
    std::string source_path;
	std::string dest_path;
	std::string staging_path;
    std::string handbrake_path;
    std::string ffmpeg_path;

    std::string encoder;
	std::string codec;
	std::string preset;
	std::string encoder_type;
//    std::string addon;
    std::string scale;
    std::string extension;

    std::string crf_string;
    std::string maxrate;
    std::string bufsize;
    std::string multi_pass;
    
	std::string audio_encode;
 //   std::string src_bitrate;
//    std::string check_addon_string;
    std::string converted_string;

    std::vector<std::string> extensions_vector;
};


class Configuration {
public:

    Configuration();
 //   Configuration();
    ~Configuration();

    int initialize();

    int validate_config();


    bool process(const char* configFile);

    std::string dump(const char* value, int type = 0);
    
    std::string get_value(const char* value, int type = 0);
    std::string get_string(const char* value);
    int32_t get_int(const char* value);
    int32_t load_extension_vector(const std::string& extensions);
    void dump_extension_vector();


 /*
    string endcode_string;
	string decode_string;
	string trash_path;
	int trash_file_size;

	string audio_encode;
	string config_file;
	string source_path;
	string dest_path;
	string staging_path;
    string handbrake_path;
	string ffmpeg_path;
	vector<string> extensions_array;
	string encoder;
	string codec;
	string preset;
	string encoder_type;
    string addon;
    string scale;

    string crf_string;
    string maxrate;
    string bufsize;
    string multi_pass;
    string audio_bitrate;
    string src_bitrate;
//	int CRF;
	int consolidate;
	int error_logging;
	
	int bitrate;
//	int maxrate;
	
	int max_size;	
//	int src_bitrate;
*/
    void set_encode_data(encode_struct& _encode_data); 
protected:
    encode_struct* encode_data;
    YAML::Node config;

//    string addon_string;
//    string decode_string;
//    string endcode_string;
};

#endif // CONFIGURATION_H