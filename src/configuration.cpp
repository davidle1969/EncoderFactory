#include <string>
#include <vector>
#include <iostream>
//#include <filesystem> 
#include <cstdlib>
#include <filesystem> // Required for std::filesystem

#include "configuration.h"
#include "logger.h"
#include "utilities.h"

namespace fs = std::filesystem;
using namespace std;


// Constructor definition
Configuration::Configuration()// : encode_data(_encode_data)
{
//    encode_data=_encode_data;
    encode_data= NULL;
   
}

Configuration::~Configuration()
{}


void Configuration::set_encode_data(encode_struct& _encode_data) 
{
    encode_data = &_encode_data;

    encode_data->encode_string="";
	encode_data->decode_string="";
	encode_data->trash_path="./trash/";

	encode_data->audio_encode="";
	encode_data->config_file="./config/config.sh";
	encode_data->source_path="./";
	encode_data->dest_path="./";
	encode_data->staging_path="./";
	encode_data->handbrake_path="";
	encode_data->ffmpeg_path="";
	encode_data->maxrate="";
	encode_data->bufsize="";
	encode_data->encoder="";
	encode_data->codec="";
	encode_data->preset="";
	encode_data->encoder_type="";
	encode_data->multi_pass="";
	encode_data->crf_string="";
	encode_data->addon_string="";
//    encode_data->src_bitrate="";
	encode_data->scale="";
    encode_data->check_addon_string="";

    encode_data->consolidate=0;
	encode_data->error_logging=0;
    encode_data->bitrate=0;
	encode_data->audio_bitrate=0;
	encode_data->max_size=0;     
	encode_data->trash_file_size=0;
    encode_data->extensions_array.clear();
}




string Configuration::dump(const char* value, int type) 
{
    if (config[value])
    {
        char buffer[256];

        switch(type)
        {
            case 1: // Integer
                snprintf(buffer, 256, "%s = %d\n", value, config[value].as<int>());
                return string(buffer);
            case 2: // Boolean
                snprintf(buffer, 256, "%s = %s\n", value, config[value].as<bool>() ? "true" : "false");
                return string(buffer);
            default: // String
                snprintf(buffer, 256, "%s = %s\n", value, config[value].as<string>().c_str());
                return string(buffer);
        }
    }
    return string("");
}

string Configuration::get_string(const char* value) 
{
try {
        if (config[value])
        {
            return config[value].as<string>();
        }
        else
        {
            return "";
        }
    } catch (const YAML::BadSubscript& e) {
            std::cerr << "Error: " << e.what() << std::endl;
    }

    return "";

    
}


int32_t Configuration::get_int(const char* value) 
{
try {
    if (config[value])
    {
        return config[value].as<int>();
    }
    else
    {
        return 0;
    }
    } catch (const YAML::BadSubscript& e) {
            std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}

int Configuration::initialize()
{
    encode_data->config_file=get_string("config_file");
//    endcode_string=get_string("endcode_string");
//	decode_string=get_string("decode_string");
//	
	encode_data->trash_path=get_string("trash_path");
	encode_data->source_path=get_string("source_path");
	encode_data->dest_path=get_string("dest_path");
	encode_data->staging_path=get_string("staging_path");
	encode_data->handbrake_path=get_string("handbrake_path");
	encode_data->ffmpeg_path=get_string("ffmpeg_path");

    //	encode_data->audio_bitrate=get_int("audio_bitrate");
//	encode_data->bufsize=get_int("bufsize");
	encode_data->encoder=get_string("encoder");
	encode_data->codec=get_string("codec");
	encode_data->preset=get_string("preset");
	encode_data->encoder_type=get_string("encoder_type");
    encode_data->scale=get_string("scale");
    encode_data->audio_encode=get_string("audio_encode");

    encode_data->crf_string = get_string("crf");
    encode_data->maxrate = get_string("maxrate");
    encode_data->bufsize = get_string("bufsize");

    encode_data->multi_pass = get_string("multi_pass");
    encode_data->audio_encode = get_string("audio_encode");

	
	//encode_data->addon=get_string("addon");
	encode_data->consolidate=get_int("consolidate");
	encode_data->error_logging=get_int("error_logging");
	encode_data->bitrate=get_int("bitrate");
    encode_data->audio_bitrate = get_int("audio_bitrate");
	encode_data->max_size=get_int("max_size");
    encode_data->trash_file_size=get_int("trash_file_size");     
    
    return 0;
} 


int Configuration::validate_config() 
{
	// Check if the source directory exists
    if (!pathExists(encode_data->source_path)) 
    {
        log (INFO, "Error: '" + encode_data->source_path + "' is not a valid source directory.");
        return -1;
    }	

	//Check if the staging directory exists
    if (!pathExists(encode_data->staging_path)) 
    {
        log (INFO, "Error: '" + encode_data->staging_path + "' is not a valid source directory.");
        return -1;
    }	


	// Check if the destination directory is provided and exists
    if (!pathExists(encode_data->dest_path)) 
    {
        log (INFO, "Error: '" + encode_data->dest_path + "' is not a valid source directory.");
        return -1;
    }	

	// Check if the trash directory exists
    if (!pathExists(encode_data->trash_path)) 
    {
        log (INFO, "Error: '" + encode_data->trash_path + "' is not a valid source directory.");
        return -1;
    }	
	
//	encode_data->src_bitrate=encode_data->bitrate;
    encode_data->max_size= encode_data->max_size * 1024 * 1024;
	if( encode_data->trash_file_size == 0 )
        encode_data->trash_file_size= encode_data->max_size * 9 / 10;


    return 0;

}



bool Configuration::process(const char* config_file) 
{
 	
	// Check if a configuration file was provided
	if ( config_file == NULL )
	{
    	log(ERROR, "Usage: NO Config File given");
		return -1;
    }

    config = YAML::LoadFile(config_file);
    if (config.IsNull())
    {
        log(ERROR, "Failed to load configuration file:", config_file);
        return false;
    }
    
    //Load the configuration file from the first argument
    initialize();
	
//	log(INFO, dump( "config"));
    log(INFO, dump( "handbrake_path")); 
	log(INFO, dump( "ffmpeg_path"));
	log(INFO, dump( "trash_path"));

	log(INFO, dump( "source_path"));
	log(INFO, dump( "dest_path"));
	log(INFO, dump( "staging_path"));


    log(INFO, dump( "audio_encode"));
	
/*	if [[ ${#extensions_array[@]} -gt 0 ]]; then
#		log(INFO,"Array loaded successfully:"
#		for value in "${extensions_array[@]}"; do
#			log(INFO,"$value"
#		done
#	else
#		log("$WARNING" "Array not loaded."
#	fi


	log(INFO,"extensions_array = ${extensions_array[*]}");
*/
	log(INFO, dump( "encoder"));
	log(INFO, dump( "codec"));

    log(INFO, dump( "preset"));
    log(INFO, dump( "multi_pass"));
    log(INFO, dump( "encoder_type"));
    log(INFO, dump( "scale"));

//	log(INFO, dump( "src_bitrate", 1));
	log(INFO, dump( "max_size", 1));
	log(INFO, dump( "bitrate", 1));
	log(INFO, dump( "maxrate", 1));
	log(INFO, dump( "bufsize", 1));
	log(INFO, dump( "consolidate", 1));
	log(INFO, dump( "error_logging", 1));
    return true;
	
	



//	init_addon
//	init_decode
//	init_encode
}


