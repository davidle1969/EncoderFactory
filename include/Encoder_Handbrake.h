#ifndef ENCODER_HANDBRAKE_H
#define ENCODER_HANDBRAKE_H
#pragma once

#include "EncoderBase.h"
#include "configuration.h"
#include <string>



class Encoder_Handbrake : public EncoderBase
{
public:
    Encoder_Handbrake();
    virtual ~Encoder_Handbrake();

    virtual void init_suffix();
//    virtual void init_decode();
//    virtual void init_encode();

 //   virtual int process();
 //   virtual int process_directory(const string& item);
 //   virtual int process_file(const string& item, const string& _sourcepath);
 
    virtual int encode_video(const string& input_file, const string& output_video, int32_t bitrate=0, int32_t audio_bitrate=0, int32_t multi_file=0);
 //   virtual int process_outputFile(const string& outputFile, const string& outputName, const string& sourcePath, int32_t src_size = 0);
 //   virtual int process_into_single_file(const string& sourcePath);
 //   virtual int process_consolidated(const string& _sourcePath);

    virtual string add_decode_setting(string  input_file);
    virtual string get_decode_codec(string  input_file);
    virtual string add_concat(int multi_file);

    virtual string add_encode_setting();
    virtual string add_encoder();
    virtual string add_CRF();
    virtual string add_maxrate();
    virtual string add_bufsize();
    virtual string add_preset();
    virtual string add_audio_encode();
    virtual string add_scale();
    virtual string add_multi_pass();
    virtual string add_audio_bitrate(const int32_t& bitrate);
    virtual string add_bitrate(const int32_t& bitrate);

    virtual int32_t get_bitrate(string input_file, int32_t file_size);
    virtual int32_t get_audio_bitrate(const string& input_file);

    virtual bool check_addon(const string& file);
    virtual string add_error_logging();


private:

};

#endif