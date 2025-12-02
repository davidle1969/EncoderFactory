// /home/davidle/projects/c/EncoderFactory.cpp
#include <filesystem> // Required for std::filesystem
#include <iostream>
#include <cstdlib> // For system()

#include "EncoderFactory.h"
#include "utilities.h"
//#include "./library/logger.h"

namespace fs = std::filesystem;
using namespace std;


EncoderFactory::EncoderFactory()// : config(encode_data)
{
	
}

EncoderFactory::~EncoderFactory()
{}


int EncoderFactory::process(const char* config_file)
{
//    openlogfile();
	log(INFO, "START");
	config.set_encode_data(encode_data);	
    config.process(config_file);
 	if (config.validate_config() < 0 )
    {
        log(ERROR, "Invalid EncoderFactory. Exiting.");
        //closelogfile();
        return -1;
    }

	if (encode_data.consolidate == 0 )
		process();
	else
		process_consolidated(encode_data.source_path);
	
	log (INFO, "FINISH");
  //  closelogfile();
    return 0;
}


int EncoderFactory::process()
{
	init_suffix();
//	init_encode();
	init_decode();

	// Check if the directory exists
    if (pathExists(encode_data.source_path))
    {
        log (INFO, "Processing source path: " + encode_data.source_path);
        return process_directory(encode_data.source_path);
    }
    else if (fileExists(encode_data.source_path))
    {
        log (INFO, "Processing source file: " + encode_data.source_path);
        return process_file(encode_data.source_path, fs::absolute(encode_data.source_path).string());
    }
    else
    {
        log (INFO, "Error: '" + encode_data.source_path + "' is not a valid source directory.");
        return -1;
    }	

    return 0;
}


int EncoderFactory::process_directory(const string& sourcepath) 
{
    log (INFO, "process_directory " + sourcepath);

    fs::path _sourcepath = sourcepath;
	
	//Check if the directory exists
    if (!pathExists(_sourcepath)) 
    {
        log (INFO, "Error: '" + _sourcepath.string() + "' is not a valid directory.");
        return -1;
    }

 
    // Create a directory_iterator to traverse the directory
    // This will iterate through the immediate children of the directory
    for (const auto& entry : std::filesystem::directory_iterator(_sourcepath)) 
    {
        // 'entry' is a std::filesystem::directory_entry object
        // You can access various properties of the entry:
        if (fileExists(entry.path())) 
            process_file (entry.path().string(), _sourcepath.string());	         
        else if (pathExists(entry.path())) 
            process_directory (entry.path().string());
		else
			log (WARNING, "process_directory: " + entry.path().string() + " is neither a file nor a directory");
        
    }
    return 0;
}

int EncoderFactory::process_file(const string& _item, const string& _sourcepath)
{
	fs::path sourcepath = _sourcepath;
    fs::path item = _item;

    if (fileExists(item)) 
    {	
		string filename = item.filename().string();
		string filename_without_extension = item.stem().string();
		
//		string file_codec = get_decode_codec(item.string());
		
//		log "$INFO" "process_file:  $item CODEC $(get_decode_codec "$item")"
		
		bool bFound = check_addon(filename_without_extension);

		if( bFound == false )
        {
//			log (INFO, "process_file: The filename " + filename_without_extension + " Encoding needed");

			string output_video = encode_data.staging_path+filename_without_extension+"."+suffix_string;			
			if ( !fileExists(output_video, true) ) {
				string file_extension = item.extension().string();
				
				int32_t src_size = get_filesize(item.string());
				int32_t bitrate = encode_data.bitrate;
					
				bFound = false;
				
/*                for element in "${extensions_array[@]}"; do
					if [[ "$element" == "$file_extension" ]]; then
						found=1
						break
					fi
				done
*/
				if (src_size > encode_data.max_size || bFound) 
                {
				    bitrate = get_bitrate(item.string(), src_size);

					//encode_data.audio_bitrate = "";
					int32_t audio_bitrate = get_audio_bitrate(item.string());	
					
					if (encode_video(item.string(), output_video, bitrate, audio_bitrate)) 
                    {
						process_outputFile(output_video, filename_without_extension + "." + encode_data.addon_string, _sourcepath);	
					} 
                    else if (fileExists(output_video)) 
                    {
						move_file(output_video, encode_data.trash_path);
					}	
				}
			} 
            else 
            {
				log (INFO, "process_file:  " + output_video + " ALREADY exist");
			}
		}
		else
		{
			log (INFO, "process_file: The filename " + filename_without_extension + " contains check string");
		}
    }
	else
	{
    	log (INFO, "process_file: " + _item + " is neither a file nor a directory");
	}
	return 0;
}

int EncoderFactory::encode_video(const string& input_file, const string& output_video, int32_t bitrate, int32_t audio_bitrate, int32_t multi_file)
{
	//Nvidia cannot decode av1 videos
	if ( multi_file == 0 ) 
    {
		if (encode_data.encoder == "nvenc" && get_decode_codec(input_file) == "av1") 
        {
			log (INFO, "encode_video: " + input_file + " NVIDIA does not support decoding av1");
			return 1;
		}
	}
	string command;	
    int result = 0;
	if (encode_data.encoder_type == "ffmpeg") 
    {
		if ( encode_data.multi_pass == "1" )
        {
			command = encode_data.ffmpeg_path+"ffmpeg "+ add_error_logging() + add_decode_setting(input_file) + "-i \"" + input_file + "\" " + encode_data.encode_string + " " + add_bitrate(bitrate) + " " + add_audio_bitrate(audio_bitrate) + " -pass 1 -vsync cfr -f null /dev/null";
			log (INFO, "encode_video: Executing " + command);
			result = system (command.c_str());
			command = encode_data.ffmpeg_path+"ffmpeg "+ add_error_logging() + add_decode_setting(input_file) + "-i \"" + input_file + "\" " + encode_data.encode_string + " " + add_bitrate(bitrate) + " " + add_audio_bitrate(audio_bitrate) + " -pass 2 " + add_audio_encode() + " \"" + output_video + "\"";
			log (INFO, "encode_video: Executing " + command);
			result = system (command.c_str());
        }
		else
		{
            command = encode_data.ffmpeg_path+"ffmpeg "+ add_error_logging() + add_decode_setting(input_file) + add_concat(multi_file) + "-i \"" + input_file + "\" " + encode_data.encode_string + " " + add_bitrate(bitrate) + " " + add_audio_bitrate(audio_bitrate) + " \"" + output_video + "\"";
			log (INFO, "encode_video: Executing " + command);
			
			result = system (command.c_str());
			
        }
    }
	else
    {
		command = encode_data.handbrake_path+"HandBrakeCLI -i \"" + input_file + "\" -o \"" + output_video + "\" " + add_decode_setting(input_file) + " " + encode_data.encode_string + " " + add_bitrate(bitrate) + " " + add_audio_bitrate(audio_bitrate) + " " + add_multi_pass();
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


int EncoderFactory::process_outputFile(const string& outputFile, const string& outputName, const string& sourcePath, int32_t src_size)
{
    int outSize = get_filesize(outputFile);
	
	if (outSize == 0) 
 		delete_file(outputFile);
	else
    {					
		if ( pathExists(encode_data.dest_path)) 
			move_file(outputFile, encode_data.dest_path);
		else
		{
            if (encode_data.trash_file_size == 0 )
			{
				if (outSize < src_size) {
					move_file(outputFile, encode_data.source_path);
					log (ERROR, encode_data.source_path + "/" + outputName);
				} 
                else 
                {
					move_file(outputFile, encode_data.trash_path);
				}
			} 
            else if (outSize > encode_data.trash_file_size) 
            {
				move_file(outputFile, encode_data.trash_path);
			} 
            else 
            {
				move_file(outputFile, encode_data.source_path);
				log (ERROR, encode_data.source_path + "/" + outputName);
            }
        }
    }
    return 0;
}


int EncoderFactory::process_into_single_file(const string& _sourcePath)
{
	// log (INFO, "process_into_single_file " + sourcePath);

//    fs::path path = _sourcePath;
 
	if (!pathExists(_sourcePath)) 
    {
        log (WARNING, "process_into_single_file Error: '" + _sourcePath + "' is not a valid directory.");
        return 1;
    }

    string current_dir_name = fs::path(_sourcePath).filename().string();
	string current_dir_name_wo_dates = remove_dates_from_filename(current_dir_name);
	
	log (INFO, "process_into_single_file: " + current_dir_name_wo_dates);
	
	string output_file= encode_data.staging_path + current_dir_name + ".txt";
//	input_file="$current_dir_name*"	
	string output_video = encode_data.staging_path + current_dir_name + "." + suffix_string;
	int32_t bitrate = encode_data.bitrate;
    
	if (fileExists(output_video, true))
	{ 
		//check to see if it's not empty
		log (INFO, "process_into_single_file:  " + output_video + " Already exist in DEST");
		return 1;
	}
	
	//Create or clear the output file
    touch_file(output_file);

	int32_t file_count= count_files_in_directory(_sourcePath);
	int32_t src_size=0;
	
	// Loop through each file in the directory
	for (const auto& itr : fs::directory_iterator(_sourcePath)) 
    {
        std::filesystem::path item = itr.path();
		// Check if it's a file or directory
        if (fileExists(item)) 
        {
			
			//check to see if it's a single file
			if (file_count == 1) 
            {
				process_file(item.filename().string(), _sourcePath);
				delete_file(output_file);
				return 1;
			}
			
			src_size= src_size + get_filesize(item.filename().string());
		
			string filename=item.filename().string();
			string filename_without_extension=filename.substr(0, filename.find_last_of('.'));
			//check to see if new file has NOT already been created and it matches the input wildcard 
			if (check_addon(filename_without_extension)) 
            {
				if (filename_without_extension.find(current_dir_name_wo_dates) == 0) 
                    append_to_file(output_file, "file '" + item.filename().string() + "'\n");
					//printf "file '%s'\n" "$item" >> "$output_file"
				else
					log(WARNING, "process_into_single_file: error - WILDCARD " + filename_without_extension + " DOES NOT matches " + current_dir_name_wo_dates);
				
            } 
			else
				log (WARNING, "process_into_single_file: info - CHECK " + filename_without_extension + " does matches check string");
            
        }
    }
	
	if (fileExists(output_file, true)) 
    {
		//check to see if it's not empty
		if (encode_video(output_file, output_video, bitrate, 1)) 
        {
			process_outputFile(output_video, current_dir_name + "." + encode_data.addon_string, _sourcePath, src_size);	
		}
        else
        {
			if (fileExists(output_video)) 
            {
				move_file(output_video, encode_data.trash_path);
			}
		}
    }
    else
    {
		delete_file(output_file);
	}
    return 0;
}


int EncoderFactory::process_consolidated(const string& _sourcePath)
{
    log(INFO, "process_consolidated " + _sourcePath);
	
	//check to see if current directory has files in it
	if (count_files_in_directory(_sourcePath) > 0) {
		//log "$INFO" "There are files in the directory."
		process_into_single_file(_sourcePath);
		return 1;
	}
				
		
	if (pathExists(_sourcePath)) 
    {

        for (const auto& itr : fs::directory_iterator(_sourcePath)) 
        {			// Check if it's a file or directory

            fs::path item = itr.path();
			if (pathExists(item)) {
				if (count_files_in_directory(item.filename().string()) > 0) 
                {
					//log "$INFO" "There are files in the directory."
					process_into_single_file(item.filename().string());
				} 
                else 
                {
					//log "$INFO" "There are NO files in the directory."
					process_consolidated(item.filename().string());
				
				}
			}
		}
    }
	else
    {
		//there are files in the current directory, lets process it
	    log(INFO, "process_consolidated Error: '" + _sourcePath + "' is not a valid directory.");
		process_into_single_file(_sourcePath);
		
    }
	return 0;

}

void EncoderFactory::init_suffix()
{
	if( suffix_string.empty() )
	{
		suffix_string = encode_data.codec + ".mp4";
		if ( encode_data.encoder == "vaapi" )
			suffix_string = "va." + suffix_string;
		else if ( encode_data.encoder == "qsv" )
			suffix_string = "qsv." + suffix_string;
		else if ( encode_data.encoder == "nvenc" )
			suffix_string = "nvenc." + suffix_string;
		
		if ( encode_data.crf_string != "" )
			suffix_string = "crf" + encode_data.crf_string + "." + suffix_string;
		
		if ( encode_data.encoder_type == "ffmpeg" )
			suffix_string = "ff." + suffix_string;
		else 
			suffix_string = "hb." + suffix_string;
		

		log (INFO,  "init_suffix: " + suffix_string);
	}
}

void EncoderFactory::init_decode()
{
	encode_data.decode_string= add_decode_setting("");
	log (INFO,  "init_decode: " + encode_data.decode_string);
}

void EncoderFactory::init_encode()
{
	encode_data.encode_string = add_encode_setting() + add_encoder() + add_CRF() + add_maxrate() + add_bufsize() + add_preset() + add_audio_encode();
	log (INFO,  "init_encode: " + encode_data.encode_string);
}

string EncoderFactory::add_decode_setting(string input_file)
{

	string Decoder;
	if ( encode_data.encoder_type == "ffmpeg" )
    {
		Decoder = "-init_hw_device qsv -hwaccel qsv -hwaccel_output_format qsv ";

        if ( encode_data.encoder == "qsv" && encode_data.codec == "h264" )
		{ 
 			if ( get_decode_codec(input_file) == "av1" )
            {
				Decoder = "";
			}
		}
		else if (encode_data.encoder == "vaapi" )
		{
			Decoder="";
//			Decoder="-vaapi_device /dev/dri/renderD128 ";
		}
		else if ( encode_data.encoder == "nvenc" )
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
    }
	else if (encode_data.encoder == "qsv" )
	{
		Decoder="--enable-hw-decoding qsv ";
	}

	return Decoder;		
}

string EncoderFactory::add_concat(int multi_file) 
{
	if (encode_data.consolidate > 0 && multi_file > 0) 
	{
		return "-f concat -safe 0 ";
	}
    return string("");
}

string EncoderFactory::get_decode_codec(string  input_file)
{
	if (std::filesystem::exists(input_file)) 
    {
        std::string command = "ffprobe -v error -select_streams v:0 -show_entries stream=codec_name -of default=noprint_wrappers=1:nokey=1 " + input_file;
        // Execute the command and redirect output to a temporary file
        // NOTE: This approach needs platform-specific handling for file redirection (e.g., "> output.txt" in most shells)
        return execute_command(command);
    }

    return string("");
}


string EncoderFactory::add_encode_setting()
{
	if (encode_data.encoder_type == "ffmpeg" )
		if (encode_data.encoder == "vaapi" )
			if ( !encode_data.scale.empty() )
				return "-vf 'format=nv12,hwupload,scale_vaapi=" + encode_data.scale + "'";
			else
				return "-vf 'format=nv12,hwupload'";
    return string("");
}

string EncoderFactory::add_encoder()
{
	if (encode_data.encoder_type == "ffmpeg" )
    {
		if (encode_data.encoder == "qsv" || encode_data.encoder == "vaapi" )
        {
			if( encode_data.codec == "h264" ) return "-c:v h264_" + encode_data.encoder;
			if( encode_data.codec == "hevc" ) return "-c:v hevc_" + encode_data.encoder;
			if( encode_data.codec == "h265" ) return "-c:v hevc_" + encode_data.encoder;
			if( encode_data.codec == "av1" ) return "-c:v av1_" + encode_data.encoder;
			return "-c:v " + encode_data.codec;
        }
        else if (encode_data.encoder == "nvenc" )
		{
            if( encode_data.codec == "h264" ) return "-c:v h264_" + encode_data.encoder;
			if( encode_data.codec == "hevc" ) return "-c:v hevc_" + encode_data.encoder;
			if( encode_data.codec == "h265" ) return "-c:v hevc_" + encode_data.encoder;
			return "-c:v " + encode_data.codec;
        }
        else
			return "-c:v " + encode_data.codec;
    }
    else if (encode_data.encoder == "qsv" )
    {
        if( encode_data.codec == "h264" ) return "-e qsv_h264";
        if( encode_data.codec == "hevc" ) return "-e qsv_h265";
        if( encode_data.codec == "h265" ) return "-e qsv_h265";
        if( encode_data.codec == "av1" ) return "-e qsv_av1";
        return "-e qsv_h264";
    }
    else
        return "-e " + encode_data.codec;
    return "";
}

string EncoderFactory::add_CRF()
{
	if ( encode_data.crf_string != "" )
		if (encode_data.encoder_type == "ffmpeg" )
			if (encode_data.encoder == "qsv" || encode_data.encoder == "vaapi" )
				return "-global_quality:v " + encode_data.crf_string + " -extbrc 1 -look_ahead_depth 50";
			else
				if (encode_data.encoder == "nvenc" )
					if  (encode_data.codec == "libx264" || encode_data.codec == "libx265" )
                            return "-crf:v " + encode_data.crf_string + " -extbrc 1 -look_ahead_depth 50";
					else
						return "-rc:v vbr -cq:v " + encode_data.crf_string + " -extbrc 1 -look_ahead_depth 50";
		else if (encode_data.encoder_type == "handbrake" )
			return "-q:v "+ encode_data.crf_string;

    return "";
}	

string EncoderFactory::add_maxrate()
{
	if (encode_data.maxrate != "" )
		if (encode_data.encoder_type == "ffmpeg" )
			return "-maxrate " + encode_data.maxrate + "k";
		else if (encode_data.encoder_type == "handbrake" )
			return "--maxBitrate " + encode_data.maxrate;
	return "";
}

string EncoderFactory::add_bufsize()
{
	if (encode_data.bufsize != "" )
		if (encode_data.encoder_type == "ffmpeg" )
			return "-bufsize " + encode_data.bufsize + "k";
		else if (encode_data.encoder_type == "handbrake" )
			return "--vbv-bufsize " + encode_data.bufsize;
	return "";
}

string EncoderFactory::add_preset()
{
	if (encode_data.preset != "" )
    {
		if (encode_data.encoder_type == "ffmpeg" )
			return "-preset " + encode_data.preset;
		else if (encode_data.encoder_type == "handbrake" )
			return "--preset \"" + encode_data.preset + "\""; 
	}
	return "";
}

string EncoderFactory::add_audio_encode()
{
    if (encode_data.audio_encode != "" )
		return encode_data.audio_encode;
	else if (encode_data.encoder_type == "handbrake" )
		return "-E copy –audio-copy-mask ac3,dts,dtshd –audio-fallback ffac3";
	else
		return "-c:a copy";
	
    return "";
}

string EncoderFactory::add_scale()
{
	if (encode_data.scale != "")
   		return "-vf \"scale=" + encode_data.scale + "\"";

    return "";

}

string EncoderFactory::add_multi_pass()
{
    if (encode_data.multi_pass != "")
		if ( encode_data.encoder_type == "handbrake" )
			return "--multi-pass";
    return "";
}


string EncoderFactory::add_bitrate(const int32_t& bitrate)
{
	if ( bitrate != 0)
    {
		if ( encode_data.encoder_type == "ffmpeg" )
			return "-b:v " + to_string(bitrate) + "k";
		else if ( encode_data.encoder_type == "handbrake" )
			return "--vb " + to_string(bitrate) + "k";
    }
    return "";
}

int32_t EncoderFactory::get_bitrate(string input, int32_t file_size)
{
	int32_t bitrate = 0;
	if (encode_data.bitrate > 0)
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
                //int32_t value = stoi(encode_data.src_bitrate);
                bitrate = file_size / duration_value;
            }
			else
				bitrate = encode_data.bitrate;
        }

        bitrate = bitrate/1024;
        
        if ( bitrate > 0 )
        {
            if ( bitrate > encode_data.bitrate )
                bitrate = encode_data.bitrate;
        }
        else
            bitrate = encode_data.bitrate;
    }
        
    return bitrate;				
}


string EncoderFactory::add_audio_bitrate(const int32_t& bitrate)
{
	if ( bitrate != 0)
    {
		if ( encode_data.encoder_type == "ffmpeg" )
			return "-b:a " + to_string(bitrate) + "k";
	}
	return "";
}


int32_t EncoderFactory::get_audio_bitrate(const string& input_file)
{
	int32_t bitrate = 0;
	if (encode_data.audio_bitrate > 0)
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

bool EncoderFactory::check_addon(const string& file)
{
	if( encode_data.check_addon_string.empty() )
	{		
		string check = encode_data.codec;
		if (encode_data.encoder == "vaapi" )  check = "va." + check;
		else if (encode_data.encoder == "qsv" )  check = "qsv." + check;
		else if (encode_data.encoder == "nvenc" )  check = "nvenc." + check;

		encode_data.check_addon_string = check;
		log( INFO, "check_addon_string:  " + encode_data.check_addon_string);
	}

	//log( INFO, "check_addon_string:  " + file + " " +encode_data.check_addon_string);


	if (file.length() >= encode_data.check_addon_string.length()) {
        if(file.substr(file.length() - encode_data.check_addon_string.length()) == encode_data.check_addon_string ) {
	//		log( INFO, "check_addon:  returning true");
			return true;
		}
    }

//    if(file.find(encode_data.check_addon_string) != string::npos)
//        return true;
	//log( INFO, "check_addon:  returning false");
    
    return false;
}

string EncoderFactory::add_error_logging()
{
	string error = "";
	
	if (encode_data.error_logging > 0 ) 
        error="-xerror -loglevel info";
	
    return error;
}





