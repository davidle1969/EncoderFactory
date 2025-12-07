#ifndef ENCODERFACTORY_H
#define ENCODERFACTORY_H

#include "EncoderBase.h"
#include "configuration.h"
#include "logger.h"
#include <string>
using namespace std;

class EncoderFactory {
public:
    EncoderFactory();
    ~EncoderFactory();

    int process(const char* config_file);
    EncoderBase* getEncoder();

/*    int process();
    int process_directory(const string& item);
    int process_file(const string& item, const string& _sourcepath);
 
    int encode_video(const string& input_file, const string& output_video, int64_t bitrate=0, int64_t audio_bitrate=0, int64_t multi_file=0);
    int process_outputFile(const string& outputFile, const string& outputName, const string& sourcePath, int64_t src_size = 0);

    int process_into_single_file(const string& sourcePath);
    int process_consolidated(const string& _sourcePath);

 
    void init_suffix();
    void init_decode();
    void init_encode();

    string add_decode_setting(string  input_file);
    string get_decode_codec(string  input_file);
    string add_concat(int multi_file);

    string add_encode_setting();
    string add_encoder();
    string add_CRF();
    string add_maxrate();
    string add_bufsize();
    string add_preset();
    string add_audio_encode();
    string add_scale();
    string add_multi_pass();
    string add_audio_bitrate(const int64_t& bitrate);
    string add_bitrate(const int64_t& bitrate);

    int64_t get_bitrate(string input_file, int64_t file_size);
    int64_t get_audio_bitrate(const string& input_file);

    bool check_addon(const string& file);
    string add_error_logging();
*/
    
 //   string get_addon_string() const { return addon_string; }



protected:
    Configuration config;
    struct encode_struct encode_data;
    string suffix_string;

};
#endif // ENCODERFACTORY_H