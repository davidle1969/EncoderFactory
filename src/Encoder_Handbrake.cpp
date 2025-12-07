#include <filesystem> // Required for std::filesystem
#include <iostream>
#include <cstdlib> // For system()

#include "Encoder_Handbrake.h"
#include "utilities.h"
#include "logger.h"

namespace fs = std::filesystem;
using namespace std;


Encoder_Handbrake::Encoder_Handbrake()
{
	encode_data = NULL;
}

Encoder_Handbrake::~Encoder_Handbrake()
{

}


bool Encoder_Handbrake::encode_video(const string& input_file, const string& output_video, int64_t bitrate, int64_t audio_bitrate, int64_t multi_file)
{
	//Nvidia cannot decode av1 videos
	if ( multi_file == 0 ) 
    {
		if (encode_data->encoder == "nvenc" && get_decode_codec(input_file) == "av1") 
        {
			log (INFO, "encode_video: " + input_file + " NVIDIA does not support decoding av1");
			return false;
		}
	}
	string command;	
    int result = 0;
	command = encode_data->handbrake_path+"HandBrakeCLI -i \"" + input_file + "\" -o \"" + output_video + "\" " + add_decode_setting(input_file) + encode_data->encode_string + " " + add_bitrate(bitrate) + add_audio_bitrate(audio_bitrate) + add_multi_pass();
	log (INFO, "encode_video: Executing " + command);
	result = system (command.c_str());
	int output = WEXITSTATUS(result);
	if (output != 0) {
		log (ERROR, "encode_video:  " + output_video + " encountered an ERROR with exit code " + std::to_string(output));
		return false;
	}
	
	return true;
}

void Encoder_Handbrake::init_suffix()
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
		
		suffix_string = "hb." + suffix_string;

		log (INFO,  "init_suffix: " + suffix_string);
	}
}

string Encoder_Handbrake::add_decode_setting(string input_file)
{
	string Decoder;
if (encode_data->encoder == "qsv" )
	{
		Decoder="--enable-hw-decoding qsv ";
	}

	return Decoder;		
}

string Encoder_Handbrake::add_concat(int multi_file) 
{   
    //handbrake does not have a way to concat files like ffmpeg
    return string("");
}

string Encoder_Handbrake::get_decode_codec(string  input_file)
{
	if (fileExists(input_file)) 
    {
        std::string command = "HandBrakeCLI -i " + input_file + " --scan";
        // Execute the command and redirect output to a temporary file
        // NOTE: This approach needs platform-specific handling for file redirection (e.g., "> output.txt" in most shells)
        string str = execute_command(command);
        str = extractValue(str, "Encoding: ");
        log (INFO,  "get_decode_codec: " + str);

        return str;
    }

    return string("");
}


string Encoder_Handbrake::add_encode_setting()
{
    return string("");
}

string Encoder_Handbrake::add_encoder()
{
    if (encode_data->encoder == "qsv" )
    {
        if( encode_data->codec == "h264" ) return "-e qsv_h264";
        if( encode_data->codec == "hevc" ) return "-e qsv_h265";
        if( encode_data->codec == "h265" ) return "-e qsv_h265";
        if( encode_data->codec == "av1" ) return "-e qsv_av1";
        return "-e qsv_h264";
    }
    else
        return "-e " + encode_data->codec;
    return "";
}

string Encoder_Handbrake::add_CRF()
{
	if ( encode_data->crf_string != "" )
		return "-q:v "+ encode_data->crf_string + " ";

    return "";
}	

string Encoder_Handbrake::add_maxrate()
{
	if ( !encode_data->maxrate.empty() )
	    return "--maxBitrate " + encode_data->maxrate + " ";
	return "";
}

string Encoder_Handbrake::add_bufsize()
{
	if (!encode_data->bufsize.empty() )
		return "--vbv-bufsize " + encode_data->bufsize + " ";
	return "";
}

string Encoder_Handbrake::add_preset()
{
	if (!encode_data->preset.empty() )
    {
		return "--preset \"" + encode_data->preset + "\" "; 
	}
	return "";
}

string Encoder_Handbrake::add_audio_encode()
{
    if (!encode_data->audio_encode.empty())
		return encode_data->audio_encode;

	return "-E copy –audio-copy-mask ac3,dts,dtshd –audio-fallback ffac3 ";
}

string Encoder_Handbrake::add_scale()
{
	if (encode_data->scale != "")
   		return "-vf \"scale=" + encode_data->scale + "\" ";

    return "";

}

string Encoder_Handbrake::add_multi_pass()
{
    if (!encode_data->multi_pass.empty())
		return "--multi-pass ";
    return "";
}


string Encoder_Handbrake::add_bitrate(const int64_t& bitrate)
{
	if ( bitrate != 0)
    {
		return "--vb " + to_string(bitrate) + "k ";
    }
    return "";
}

int64_t Encoder_Handbrake::get_bitrate(string input, int64_t file_size, int64_t conf_bitrate)
{
	int64_t bitrate = 0;
	if (conf_bitrate > 0)
    {
		//string command = "(ffmpeg -i " + input + " 2>&1 | grep \"bitrate:\")";
        string command = "HandBrakeCLI -i " + input + " --scan";

        string output = execute_command(command);
        output = extractValue(output, "Average Bitrate: ");
        log (INFO,  "get_bitrate: " + output); 
			
		//command = "(echo \"$output\" | awk -F 'bitrate: ' '{print $2}' | awk '{print $1}')";
		string bitrate_awk = extractValue(output, "Average Bitrate: ");
        if ( bitrate_awk != "" )
        {
            log( INFO, "get_bitrate:  bitrate_awk0 " + bitrate_awk);
            int64_t value = stoi(bitrate_awk);
            bitrate = value * 1024;

            log( INFO, "get_bitrate:  bitrate_awk " + to_string(bitrate));
        }   
        else	
        {
            //get the size in bits
            file_size = file_size * 8; 

            // Get video duration in seconds
        //    command = "$(ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 \"" + input + "\")";
            string duration = extractValue(output, "Duration: ");//execute_command(command);
            
            log (INFO, duration  + " " + to_string(file_size));

            int64_t duration_value = convertTimeToSeconds(duration);

 			if (duration_value != 0 )
            {
                //int64_t value = stoi(encode_data->src_bitrate);
                bitrate = file_size / duration_value;
            }
			else
				bitrate = conf_bitrate;
        }

        bitrate = bitrate/1024;
        
        if ( bitrate > 0 )
        {
            if ( bitrate > conf_bitrate )
                bitrate = conf_bitrate;
        }
        else
            bitrate = conf_bitrate;
    }
        
    return bitrate;				
}


string Encoder_Handbrake::add_audio_bitrate(const int64_t& bitrate)
{
	return "";
}


int64_t Encoder_Handbrake::get_audio_bitrate(const string& input_file)
{
	int64_t bitrate = 0;
	if (encode_data->audio_bitrate > 0)
    {

		string command = "(ffprobe -v error -select_streams a:0 -show_entries stream=bit_rate -of default=noprint_wrappers=1:nokey=1 \"" + input_file + "\")";
		string output = execute_command(command); 
		
		log(INFO, "get_audio_bitrate:  output " + output);

		command = "(echo \"" + output + "\" | awk -F', ' '/Audio:/ {print $NF}' | awk '{print $(NF-1)}')";
		int64_t bitrate_awk = atoi(execute_command(command).c_str());
		
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

/*bool Encoder_Handbrake::check_addon(const string& file)
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
*/
//HandBrakeCLI --input input_video.mp4 --info | grep "resolution" | awk -F '[x]' '{print $1 "\n" $2}'

string Encoder_Handbrake::add_error_logging()
{
	string error = "";
	
	if (encode_data->error_logging > 0 ) 
        error="-xerror -loglevel info ";
	
    return error;
}





