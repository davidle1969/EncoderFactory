
#include <filesystem> // Required for std::filesystem
#include <iostream>
#include <cstdlib> // For system()

#include "Encoder_ffmpeg.h"
#include "utilities.h"
#include "logger.h"

namespace fs = std::filesystem;
using namespace std;

Encoder_ffmpeg::Encoder_ffmpeg()
{
    encode_data = NULL;

}

Encoder_ffmpeg::~Encoder_ffmpeg()
{

}



int Encoder_ffmpeg::encode_video(const string& input_file, const string& output_video, int32_t bitrate, int32_t audio_bitrate, int32_t multi_file)
{
	//Nvidia cannot decode av1 videos
	if ( multi_file == 0 ) 
    {
		if (encode_data->encoder == "nvenc" && get_decode_codec(input_file) == "av1") 
        {
			log (INFO, "encode_video: " + input_file + " NVIDIA does not support decoding av1");
			return 1;
		}
	}
	string command;	
    int result = 0;
    if ( encode_data->multi_pass == "1" )
    {
        command = encode_data->ffmpeg_path+"ffmpeg "+ add_error_logging() + add_decode_setting(input_file) + "-i \"" + input_file + "\" " + encode_data->encode_string + " " + add_bitrate(bitrate) + " " + add_audio_bitrate(audio_bitrate) + " -pass 1 -vsync cfr -f null /dev/null";
        log (INFO, "encode_video: Executing " + command);
        result = system (command.c_str());
        command = encode_data->ffmpeg_path+"ffmpeg "+ add_error_logging() + add_decode_setting(input_file) + "-i \"" + input_file + "\" " + encode_data->encode_string + " " + add_bitrate(bitrate) + " " + add_audio_bitrate(audio_bitrate) + " -pass 2 " + add_audio_encode() + " \"" + output_video + "\"";
        log (INFO, "encode_video: Executing " + command);
        result = system (command.c_str());
    }
    else
    {
        command = encode_data->ffmpeg_path+"ffmpeg "+ add_error_logging() + add_decode_setting(input_file) + add_concat(multi_file) + "-i \"" + input_file + "\" " + encode_data->encode_string + " " + add_bitrate(bitrate) + " " + add_audio_bitrate(audio_bitrate) + " \"" + output_video + "\"";
        log (INFO, "encode_video: Executing " + command);
        
        result = system (command.c_str());
    }

	int output = WEXITSTATUS(result);
	if (output != 0) {
		log (ERROR, "encode_video:  " + output_video + " encountered an ERROR with exit code " + std::to_string(output));
		return 1;
	}
	
	return 0;
}


void Encoder_ffmpeg::init_suffix()
{
	if( suffix_string.empty() )
	{
		suffix_string = encode_data->codec + ".mp4";
		if ( encode_data->encoder == "vaapi" )
			suffix_string = "va." + suffix_string;
		else if ( encode_data->encoder == "qsv" )
			suffix_string = "qsv." + suffix_string;
		else if ( encode_data->encoder == "nvenc" )
			suffix_string = "nvenc." + suffix_string;
		
		if ( encode_data->crf_string != "" )
			suffix_string = "crf" + encode_data->crf_string + "." + suffix_string;
		
		suffix_string = "ff." + suffix_string;

		log (INFO,  "init_suffix: " + suffix_string);
	}
}

string Encoder_ffmpeg::add_decode_setting(string input_file)
{

	string Decoder;
    Decoder = "-init_hw_device qsv -hwaccel qsv -hwaccel_output_format qsv ";

    if ( encode_data->encoder == "qsv" && encode_data->codec == "h264" )
    { 
        if ( get_decode_codec(input_file) == "av1" )
        {
            Decoder = "";
        }
    }
    else if (encode_data->encoder == "vaapi" )
    {
        Decoder="";
//			Decoder="-vaapi_device /dev/dri/renderD128 ";
    }
    else if ( encode_data->encoder == "nvenc" )
    {
        if ( get_decode_codec(input_file) == "av1" )
        {
            Decoder="";
        }
        else
        {
            Decoder="-hwaccel cuda -hwaccel_output_format cuda ";
        }
    }
    else
    {
        Decoder="";			
    }

	return Decoder;		
}

string Encoder_ffmpeg::add_concat(int multi_file) 
{
	if (encode_data->consolidate > 0 && multi_file > 0) 
	{
		return "-f concat -safe 0 ";
	}
    return string("");
}

string Encoder_ffmpeg::get_decode_codec(string  input_file)
{
	if (fileExists(input_file)) 
    {
        std::string command = "ffprobe -v error -select_streams v:0 -show_entries stream=codec_name -of default=noprint_wrappers=1:nokey=1 " + input_file;
        // Execute the command and redirect output to a temporary file
        // NOTE: This approach needs platform-specific handling for file redirection (e.g., "> output.txt" in most shells)
        return execute_command(command);
    }

    return string("");
}


string Encoder_ffmpeg::add_encode_setting()
{
    if (encode_data->encoder == "vaapi" )
        if ( !encode_data->scale.empty() )
            return "-vf 'format=nv12,hwupload,scale_vaapi=" + encode_data->scale + "' ";
        else
            return "-vf 'format=nv12,hwupload' ";

    return string("");
}

string Encoder_ffmpeg::add_encoder()
{
    if (encode_data->encoder == "qsv" || encode_data->encoder == "vaapi" )
    {
        if( encode_data->codec == "h264" ) return "-c:v h264_" + encode_data->encoder;
        if( encode_data->codec == "hevc" ) return "-c:v hevc_" + encode_data->encoder;
        if( encode_data->codec == "h265" ) return "-c:v hevc_" + encode_data->encoder;
        if( encode_data->codec == "av1" ) return "-c:v av1_" + encode_data->encoder;
        return "-c:v " + encode_data->codec;
    }
    else if (encode_data->encoder == "nvenc" )
    {
        if( encode_data->codec == "h264" ) return "-c:v h264_" + encode_data->encoder;
        if( encode_data->codec == "hevc" ) return "-c:v hevc_" + encode_data->encoder;
        if( encode_data->codec == "h265" ) return "-c:v hevc_" + encode_data->encoder;
        return "-c:v " + encode_data->codec;
    }
    else
        return "-c:v " + encode_data->codec;
}

string Encoder_ffmpeg::add_CRF()
{
	if ( !encode_data->crf_string.empty() )
    {
			if (encode_data->encoder == "qsv" || encode_data->encoder == "vaapi" )
				return "-global_quality:v " + encode_data->crf_string + " -extbrc 1 -look_ahead_depth 50 ";
			else
			{
					if  (encode_data->codec == "libx264" || encode_data->codec == "libx265" )
                            return "-crf:v " + encode_data->crf_string + " -extbrc 1 -look_ahead_depth 50";
					else
						return "-rc:v vbr -cq:v " + encode_data->crf_string + " -extbrc 1 -look_ahead_depth 50 ";
            }
    }
    return "";
}	

string Encoder_ffmpeg::add_maxrate()
{
	if (encode_data->maxrate != "" )
		return "-maxrate " + encode_data->maxrate + "k ";
	return "";
}

string Encoder_ffmpeg::add_bufsize()
{
	if (encode_data->bufsize != "" )
		return "-bufsize " + encode_data->bufsize + "k ";
	return "";
}

string Encoder_ffmpeg::add_preset()
{
	if (encode_data->preset != "" )
    {
		return "-preset " + encode_data->preset + " ";
	}
	return "";
}

string Encoder_ffmpeg::add_audio_encode()
{
    if ( !encode_data->audio_encode.empty() )
		return encode_data->audio_encode;
	else
		return "-c:a copy ";
	
    return "";
}

string Encoder_ffmpeg::add_scale()
{
	if (encode_data->scale != "")
   		return "-vf \"scale=" + encode_data->scale + "\" ";

    return "";

}

string Encoder_ffmpeg::add_multi_pass()
{
    return "";
}


string Encoder_ffmpeg::add_bitrate(const int32_t& bitrate)
{
	if ( bitrate != 0)
    {
		return "-b:v " + to_string(bitrate) + "k ";
    }
    return "";
}

int32_t Encoder_ffmpeg::get_bitrate(string input, int32_t file_size)
{
	int32_t bitrate = 0;
	if (encode_data->bitrate > 0)
    {
		string command = "(ffmpeg -i " + input + " 2>&1 | grep \"bitrate:\")";
        string output = execute_command(command); 
			
		command = "(echo \"$output\" | awk -F 'bitrate: ' '{print $2}' | awk '{print $1}')";
		string bitrate_awk = execute_command(command);

        if ( bitrate_awk != "" )
        {
            log( INFO, "get_bitrate:  bitrate_awk0 " + bitrate_awk);
            int32_t value = stoi(bitrate_awk);
            bitrate = value * 1024;

            log( INFO, "get_bitrate:  bitrate_awk " + to_string(bitrate));
        }   
        else	
        {
            //get the size in bits
            file_size = file_size * 8; 

            // Get video duration in seconds
            command = "$(ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 \"" + input + "\")";
            string duration = execute_command(command);
            
            log (INFO, duration  + " " + to_string(file_size));

            int32_t duration_value = stoi(duration);

 			if (duration_value != 0 )
            {
                //int32_t value = stoi(encode_data->src_bitrate);
                bitrate = file_size / duration_value;
            }
			else
				bitrate = encode_data->bitrate;
        }

        bitrate = bitrate/1024;
        
        if ( bitrate > 0 )
        {
            if ( bitrate > encode_data->bitrate )
                bitrate = encode_data->bitrate;
        }
        else
            bitrate = encode_data->bitrate;
    }
        
    return bitrate;				
}


string Encoder_ffmpeg::add_audio_bitrate(const int32_t& bitrate)
{
	if ( bitrate != 0)
    {
		return "-b:a " + to_string(bitrate) + "k ";
	}
	return "";
}


int32_t Encoder_ffmpeg::get_audio_bitrate(const string& input_file)
{
	int32_t bitrate = 0;
	if (encode_data->audio_bitrate > 0)
    {
		string command = "(ffprobe -v error -select_streams a:0 -show_entries stream=bit_rate -of default=noprint_wrappers=1:nokey=1 \"" + input_file + "\")";
		string output = execute_command(command); 
		
		log(INFO, "get_audio_bitrate:  output " + output);

		command = "(echo \"" + output + "\" | awk -F', ' '/Audio:/ {print $NF}' | awk '{print $(NF-1)}')";
		int32_t bitrate_awk = atoi(execute_command(command).c_str());
		
		log(INFO, "get_audio_bitrate:  bitrate_awk " + bitrate_awk);
		
		if (bitrate_awk > 0)
		{
			log(INFO, "get_audio_bitrate:  bitrate_awk0 " + to_string(bitrate_awk));
			bitrate = bitrate_awk * 1024;
			
			log( INFO, "get_audio_bitrate:  audio_bitrate " + to_string(bitrate));
		}
	}

    return bitrate;
}

bool Encoder_ffmpeg::check_addon(const string& file)
{
	if( encode_data->check_addon_string.empty() )
	{		
		string check = encode_data->codec;
		if (encode_data->encoder == "vaapi" )  check = "va." + check;
		else if (encode_data->encoder == "qsv" )  check = "qsv." + check;
		else if (encode_data->encoder == "nvenc" )  check = "nvenc." + check;

		encode_data->check_addon_string = check;
		log( INFO, "check_addon_string:  " + encode_data->check_addon_string);
	}

	//log( INFO, "check_addon_string:  " + file + " " +encode_data->check_addon_string);


	if (file.length() >= encode_data->check_addon_string.length()) {
        if(file.substr(file.length() - encode_data->check_addon_string.length()) == encode_data->check_addon_string ) {
	//		log( INFO, "check_addon:  returning true");
			return true;
		}
    }

//    if(file.find(encode_data->check_addon_string) != string::npos)
//        return true;
	//log( INFO, "check_addon:  returning false");
    
    return false;
}

string Encoder_ffmpeg::add_error_logging()
{
	string error = "";
	
	if (encode_data->error_logging > 0 ) 
        error="-xerror -loglevel info ";
	
    return error;
}






