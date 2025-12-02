#ifndef ENCODER_FFMPEG_H
#define ENCODER_FFMPEG_H
#pragma once


#include "configuration.h"
#include <string>


class Encoder_ffmpeg
{
public:
    Encoder_ffmpeg();
    ~Encoder_ffmpeg();

    void set_encode_data(encode_struct& _encode_data);
    int encode_video(const string& input_file, const string& output_video, int32_t bitrate, int32_t audio_bitrate, int32_t multi_file);
    
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
    string add_audio_bitrate(const int32_t& bitrate);
    string add_bitrate(const int32_t& bitrate);

    int32_t get_bitrate(string input_file, int32_t file_size);
    int32_t get_audio_bitrate(const string& input_file);

    bool check_addon(const string& file);
    string add_error_logging();


private:
    struct encode_struct* encode_data;
    string suffix_string;

};

#endif