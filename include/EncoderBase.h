#ifndef ENCODERBASE_H
#define ENCODERBASE_H

#pragma once

#include "configuration.h"
#include <string>


class EncoderBase
{
public:
    EncoderBase();
    virtual ~EncoderBase();

    void set_encode_data(encode_struct& _encode_data);
    

    virtual int process();
    virtual int process_directory(const string& item);
    virtual int process_file(const string& item, const string& _sourcepath);
 
    virtual int process_outputFile(const string& outputFile, const string& outputName, const string& sourcePath, int32_t src_size = 0);
    virtual int process_into_single_file(const string& sourcePath);
    virtual int process_consolidated(const string& _sourcePath);

    virtual void init_suffix() {}
    virtual void init_decode();
    virtual void init_encode();

    virtual int encode_video(const string& input_file, const string& output_video, int32_t bitrate=0, int32_t audio_bitrate=0, int32_t multi_file=0) { return 0; }
    virtual string add_decode_setting(string  input_file) { return ""; }
    virtual string get_decode_codec(string  input_file) { return ""; }
    virtual string add_concat(int multi_file) { return ""; }

    virtual string add_encode_setting() { return ""; }
    virtual string add_encoder() { return ""; }
    virtual string add_CRF() { return ""; }
    virtual string add_maxrate() { return ""; }
    virtual string add_bufsize() { return ""; }
    virtual string add_preset() { return ""; }
    virtual string add_audio_encode() { return ""; }
    virtual string add_scale() { return ""; }
    virtual string add_multi_pass() { return ""; }
    virtual string add_audio_bitrate(const int32_t& bitrate) { return ""; }
    virtual string add_bitrate(const int32_t& bitrate) { return ""; }

    virtual int32_t get_bitrate(string input_file, int32_t file_size) { return 0; }
    virtual int32_t get_audio_bitrate(const string& input_file) { return 0; }

    virtual bool check_addon(const string& file) {return false;}
    virtual string add_error_logging() { return ""; }

protected:
    struct encode_struct* encode_data;
    string suffix_string;
};

#endif