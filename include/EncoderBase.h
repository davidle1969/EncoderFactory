#ifndef ENCODERBASE_H
#define ENCODERBASE_H

#pragma once

#include "configuration.h"
#include <string>

struct resolution
{
    int width;
	int height;
};

struct video_detail
{
    /* data */
    std::string filename;
    std::string codec;
    std::string extension;

    int64_t file_size;
    int64_t bitrate;
    int64_t audio_bitrate;

    struct resolution res;
};

struct encode_detail
{
    /* data */
    std::string output_video;
    std::string input_file;
    std::string codec;
//    std::string extension;

    int64_t src_size;
    int64_t src_file_count;

    int64_t bitrate;
    int64_t audio_bitrate;

    bool bMultifile;  
    bool bRescale;
    struct resolution res;
};

class EncoderBase
{
public:
    EncoderBase();
    virtual ~EncoderBase();

    void set_encode_data(encode_struct& _encode_data);
    

    virtual int process();
    virtual int process_consolidated();

    virtual int process_directory(const string& item);
    virtual int process_file(const string& item, const string& _sourcepath);
 
    virtual int process_outputFile(const string& outputFile, const string& sourcePath, int64_t src_size = 0);
    virtual int process_into_single_file(const string& sourcePath);
    virtual int process_consolidated(const string& _sourcePath);

    virtual void init_suffix() {}
    virtual void init_decode();
    virtual void init_encode();

    virtual bool encode_video(const struct encode_detail& encoder) { return 0; }
 //   virtual bool encode_video(const string& input_file, const string& output_video, int64_t bitrate=0, int64_t audio_bitrate=0, int64_t multi_file=0) { return 0; }
    
    virtual string add_decode_setting(string  input_file) { return ""; }
    virtual string get_decode_codec(string  input_file) { return ""; }
    virtual string add_concat(int multi_file) { return ""; }
    virtual string add_concat() { return ""; }


    virtual string add_encode_setting() { return ""; }
    virtual string add_encoder() { return ""; }
    virtual string add_CRF() { return ""; }
    virtual string add_maxrate() { return ""; }
    virtual string add_bufsize() { return ""; }
    virtual string add_preset() { return ""; }
    virtual string add_audio_encode() { return ""; }
    virtual string add_scale() { return ""; }
    virtual string add_scale(const struct encode_detail& encoder) { return ""; }
    virtual string add_multi_pass() { return ""; }
    virtual string add_audio_bitrate(const int64_t& bitrate) { return ""; }
    virtual string add_bitrate(const int64_t& bitrate) { return ""; }

    virtual int64_t get_bitrate(string input_file, int64_t file_size, int64_t conf_bitrate) { return 0; }
    virtual int64_t get_bitrate(string input, int64_t file_size) { return 0; }

    virtual int64_t get_audio_bitrate(const string& input_file, int64_t conf_audio_bitrate) { return 0; }
    virtual int64_t get_audio_bitrate(const string& input_file) { return 0; }

    virtual string get_convert_string();

    void get_video_detail(struct video_detail& data, const string& fileName);

    virtual struct resolution get_resolution(const string& input_file);

    virtual bool has_been_converted(const string& file) {return false;}
    virtual string add_error_logging() { return ""; }

    void dump(struct video_detail& data, int type);
    void dump(struct video_detail& data);

    void dump(const string&  filename, int type);
    void dump_directory(const string&  path, int type = 0);

    

protected:
    struct encode_struct* encode_data;
    string suffix_string;
};

#endif