#include "EncoderBase.h"

#include <filesystem> // Required for std::filesystem
#include <cstring>  // For memset
#include <iostream>
#include <cstdlib> // For system()

#include "utilities.h"
#include "logger.h"

namespace fs = std::filesystem;
using namespace std;

EncoderBase::EncoderBase()
{

}

EncoderBase::~EncoderBase()
{

}



void EncoderBase::set_encode_data(encode_struct& _encode_data) 
{
    encode_data = &_encode_data;
}


int EncoderBase::process()
{
	init_suffix();
	init_encode();
	init_decode();

	// Check if the directory exists
    if (pathExists(encode_data->source_path))
    {
        log (INFO, "Processing source path: " + encode_data->source_path);
        return process_directory(encode_data->source_path);
    }
    else if (fileExists(encode_data->source_path))
    {
        log (INFO, "Processing source file: " + encode_data->source_path);
        return process_file(encode_data->source_path, fs::absolute(encode_data->source_path).string());
    }
    else
    {
        log (INFO, "Error: '" + encode_data->source_path + "' is not a valid source directory.");
        return -1;
    }	

    return 0;
}


int EncoderBase::process_consolidated()
{
	init_suffix();
	init_encode();
	init_decode();

	// Check if the directory exists
    if (pathExists(encode_data->source_path))
    {
        log (INFO, "process_consolidated:  Processing source path: " + encode_data->source_path);
        return process_consolidated(encode_data->source_path);
    }
    else
    {
        log (ERROR, "process_consolidated: " + encode_data->source_path + "' is not a valid source directory.");
        return -1;
    }	

    return 0;
}

int EncoderBase::process_directory(const string& sourcepath) 
{
    log (INFO, "process_directory " + sourcepath);

    fs::path _sourcepath(sourcepath);
	
	//Check if the directory exists
    if (!pathExists(_sourcepath)) 
    {
        log (INFO, "Error: '" + _sourcepath.string() + "' is not a valid directory.");
        return -1;
    }

	std::vector<std::filesystem::path> files_in_directory;
	std::copy(std::filesystem::directory_iterator(_sourcepath), std::filesystem::directory_iterator(), std::back_inserter(files_in_directory));
	std::sort(files_in_directory.begin(), files_in_directory.end());

	for (const auto& entry : files_in_directory) 
	{
	
 
    // Create a directory_iterator to traverse the directory
    // This will iterate through the immediate children of the directory
//    for (const auto& entry : std::filesystem::directory_iterator(_sourcepath)) 
//    {
        // 'entry' is a std::filesystem::directory_entry object
        // You can access various properties of the entry:
        if (fileExists(entry)) 
            process_file (entry.string(), _sourcepath.string());	         
        else if (pathExists(entry)) 
            process_directory (entry.string());
		else
			log (WARNING, "process_directory: " + entry.string() + " is neither a file nor a directory");
        
    }
    return 0;
}

int EncoderBase::process_file(const string& _item, const string& _sourcepath)
{
	fs::path sourcepath = _sourcepath;
    fs::path item = _item;

	log (INFO, "process_file: Starting " + item.string());

    if (fileExists(item)) 
    {
		string filename_without_extension = item.stem().string();

		struct video_detail video_detail;
		get_video_detail(video_detail, item.string());

		struct encode_detail encoder;
		memset(&encoder, 0, sizeof(encode_detail));
		encoder.output_video = encode_data->staging_path+filename_without_extension+"."+suffix_string;
		
//		bool bFound = false;//has_been_converted(filename_without_extension);

		if( !has_been_converted(filename_without_extension) )
        {
//			log (INFO, "process_file: The filename " + filename_without_extension + " Encoding needed");
						
			if ( !fileExists(encoder.output_video, true) ) 
            {
//				bFound = false;
				
                if(std::find(encode_data->extensions_vector.begin(), encode_data->extensions_vector.end(), item.extension().string()) != encode_data->extensions_vector.end() )
				{
					if (video_detail.file_size > encode_data->max_size ) 
					{
						encoder.input_file = item.string();
						encoder.src_file_count = count_files_in_directory(_sourcepath);
				//		encoder.bitrate = video_detail.bitrate;
				//		encoder.audio_bitrate = video_detail.audio_bitrate;
						encoder.codec = video_detail.codec;
						encoder.res = video_detail.res;
						encoder.bMultifile = false;
				//		encoder.extension = encode_data->extension;
						encoder.bitrate = encode_data->bitrate;//get_bitrate(item.string(), src_size, encode_data->bitrate);
						if (encode_data->bitrate > 0)
						{
							encoder.bitrate = video_detail.bitrate/1024;
							
							if ( encoder.bitrate > encode_data->bitrate )
								encoder.bitrate = encode_data->bitrate;
						}									

						//encode_data->audio_bitrate = "";
						if (encode_data->audio_bitrate > 0)
							encoder.audio_bitrate = video_detail.audio_bitrate;
						
						dump(video_detail, 0);
						if (encode_video(encoder))
						{
							process_outputFile(encoder.output_video, item.parent_path(), video_detail.file_size);	
						} 
						else if (fileExists(encoder.output_video)) 
						{
							move_file(encoder.output_video, encode_data->trash_path);
							log (ERROR, "process_file: Move to trash_path " + encode_data->trash_path + "/" + encoder.output_video);
						}	
					}
					else
					{
						log (ERROR, "process_file: src_size > max_size " + item.string() + " " + to_string(video_detail.file_size) + " <= " + to_string(encode_data->max_size));
					}
				}
				else
					log (DEBUG, "process_file: Extension is not in the vector " + item.extension().string());

			} 
            else 
            {
				log (INFO, "process_file:  " + encoder.output_video + " ALREADY exist");
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

/*int EncoderBase::process_file(const string& _item, const string& _sourcepath)
{
	fs::path sourcepath = _sourcepath;
    fs::path item = _item;

	log (INFO, "process_file: Starting " + item.string());

    if (fileExists(item)) 
    {
		string filename_without_extension = item.stem().string();

		struct video_detail video_detail;
		get_video_detail(video_detail, item.string());

		struct encode_detail encoder;
		memset(&encoder, 0, sizeof(encode_detail));
		encoder.output_video = encode_data->staging_path+filename_without_extension+"."+suffix_string;
		encoder.input_file = item.string();
		encoder.src_file_count = count_files_in_directory(_sourcepath);
		encoder.bitrate = video_detail.bitrate;
		encoder.audio_bitrate = video_detail.audio_bitrate;
		encoder.codec = video_detail.codec;
		encoder.res = video_detail.res;

		//string filename = item.filename().string();
		
		
//		string file_codec = get_decode_codec(item.string());
		
//		log "$INFO" "process_file:  $item CODEC $(get_decode_codec "$item")"
		
		bool bFound = false;//has_been_converted(filename_without_extension);

		if( !has_been_converted(filename_without_extension) )
        {
//			log (INFO, "process_file: The filename " + filename_without_extension + " Encoding needed");

						
			if ( !fileExists(encoder.output_video, true) ) 
            {
//				string file_extension = item.extension().string();
//				int64_t src_size = get_filesize(item.string());
//				int64_t bitrate = encode_data->bitrate;
					
				bFound = false;
				
//                for element in "${extensions_array[@]}"; do
//					if [[ "$element" == "$file_extension" ]]; then
//						found=1
//						break
//					fi
//				done

				if (video_detail.file_size > encode_data->max_size || bFound) 
                {
				    int64_t bitrate = video_detail.bitrate;//get_bitrate(item.string(), src_size, encode_data->bitrate);
					if (encode_data->bitrate > 0)
					{
						encoder.bitrate = video_detail.bitrate/1024;
						
						if ( encoder.bitrate > 0 )
						{
							if ( encoder.bitrate > encode_data->bitrate )
								encoder.bitrate = encode_data->bitrate;
						}
						else
							encoder.bitrate = encode_data->bitrate;
					}									

					//encode_data->audio_bitrate = "";
					//int64_t audio_bitrate = get_audio_bitrate(item.string(), encode_data->audio_bitrate);	
					int64_t audio_bitrate = 0;
					if (encode_data->audio_bitrate > 0)
						encoder.audio_bitrate = video_detail.audio_bitrate;
					
					dump(video_detail, 0);
					if (encode_video(encoder))//item.string(), output_video, bitrate, audio_bitrate)) 
                    {
						//process_outputFile(output_video, filename_without_extension + "." + encode_data->addon_string, _sourcepath, video_detail.file_size);
						process_outputFile(encoder.output_video, item.parent_path(), video_detail.file_size);	
					} 
                    else if (fileExists(encoder.output_video)) 
                    {
						move_file(encoder.output_video, encode_data->trash_path);
						log (ERROR, "process_file: Move to trash_path " + encode_data->trash_path + "/" + encoder.output_video);
					}	
				}
				else
				{
					log (ERROR, "process_file: src_size > max_size " + item.string() + " " + to_string(video_detail.file_size) + " <= " + to_string(encode_data->max_size));
				}
			} 
            else 
            {
				log (INFO, "process_file:  " + encoder.output_video + " ALREADY exist");
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
*/

int EncoderBase::process_outputFile(const string& outputFile, const string& sourcePath, int64_t src_size)
{
    int64_t outSize = get_filesize(outputFile);
	
	if (outSize == 0) 
 		delete_file(outputFile);
	else
    {					
		if ( pathExists(encode_data->dest_path))
		{ 
			move_file(outputFile, encode_data->dest_path);
			log (DEBUG, "process_outputFile: Move to dest_path " + encode_data->dest_path + " " + outputFile);
		}
		else
		{
            if (encode_data->trash_file_size == 0 )
			{
				if (outSize < src_size) {
				//	move_file(outputFile, encode_data->source_path);
					move_file(outputFile, sourcePath);
				//	log (ERROR, "process_outputFile: outSize < src_size" encode_data->source_path + "/" + outputName);
				} 
                else 
                {
					move_file(outputFile, encode_data->trash_path);
					log (DEBUG, "process_outputFile: outSize Larger than  src_size" + encode_data->trash_path + "/" + outputFile);
				}
			} 
            else if (outSize > encode_data->trash_file_size) 
            {
				move_file(outputFile, encode_data->trash_path);
				log (DEBUG, "process_outputFile: outSize Larger than  trash_file_size" + encode_data->trash_path + "/" + outputFile);
			} 
            else 
            {
				//move_file(outputFile, encode_data->source_path);
				move_file(outputFile, sourcePath);
				log (DEBUG, "process_outputFile: Move to Source " + sourcePath + "/" + outputFile);
            }
        }
    }
    return 0;
}

int EncoderBase::process_into_single_file(const string& _sourcePath)
{
	log (DEBUG, "process_into_single_file: " + _sourcePath);
 
	if (!pathExists(_sourcePath)) 
    {
        log (WARNING, "process_into_single_file Error: '" + _sourcePath + "' is not a valid directory.");
        return 1;
    }

    string current_dir_name = fs::path(_sourcePath).filename();
	log (DEBUG, "process_into_single_file:  current_dir_name " + current_dir_name);

	string current_dir_name_wo_dates = remove_dates_from_filename(current_dir_name);
	log (DEBUG, "process_into_single_file:  current_dir_name_wo_dates " + current_dir_name_wo_dates);

	struct encode_detail encoder;
	memset(&encoder, 0, sizeof(encode_detail));
	encoder.output_video = encode_data->staging_path + current_dir_name + "." + suffix_string;
    
	if (fileExists(encoder.output_video, true))
	{ 
		//check to see if it's not empty
		log (WARNING, "process_into_single_file:  " + encoder.output_video + " Already exist in DEST");
		return 1;
	}

	encoder.input_file = encode_data->staging_path + current_dir_name + ".txt";
	encoder.src_file_count = count_files_in_directory(_sourcePath);
	encoder.bitrate = encode_data->bitrate;
	
	//Create or clear the output file
    touch_file(encoder.input_file);

	// Loop through each file in the directory
	std::vector<std::filesystem::path> files_in_directory;
	std::copy(std::filesystem::directory_iterator(_sourcePath), std::filesystem::directory_iterator(), std::back_inserter(files_in_directory));
	std::sort(files_in_directory.begin(), files_in_directory.end());

	for (const auto& itr : files_in_directory) 
	{
        std::filesystem::path item = itr;
		// Check if it's a file or directory
        if (fileExists(item)) 
        {
			//check to see if it's a single file
			if (encoder.src_file_count == 1) 
            {
				process_file(item.string(), _sourcePath);
				delete_file(encoder.input_file);
				return 1;
			}

			string filename=item.filename().string();
			string filename_without_extension=filename.substr(0, filename.find_last_of('.'));
			//check to see if new file has NOT already been created and it matches the input wildcard 
			if (!has_been_converted(filename_without_extension)) 
			{
				if (filename_without_extension.find(current_dir_name_wo_dates) == 0) 
				{
					if(std::find(encode_data->extensions_vector.begin(), encode_data->extensions_vector.end(), item.extension().string()) != encode_data->extensions_vector.end() )
					{
						struct video_detail video_detail;
						get_video_detail(video_detail, item.string());

						if( encoder.res.height == 0)
						{
							encoder.res = video_detail.res;
						}

						//check the resolution and make it smaller if the sizes are different
						if( encoder.res.height != video_detail.res.height )
						{
							encoder.bRescale = true;
						}
						if( encoder.res.height > video_detail.res.height )
						{
							encoder.res = video_detail.res;
						}
						
						encoder.src_size += video_detail.file_size;
						//check to see if new file has NOT already been created and it matches the input wildcard 
						append_to_file(encoder.input_file, "file '" + item.string() + "'\n");
					}
					else
					{
						log (DEBUG, "process_into_single_file: Extension is not in the vector " + item.extension().string());
					}
				}
				else
				{
					log(WARNING, "process_into_single_file: error - WILDCARD " + filename_without_extension + " DOES NOT matches " + current_dir_name_wo_dates);
				}

			}
			else
			{
				log (DEBUG, "process_into_single_file: " + filename_without_extension + " already converted");
			}

/*			
			struct video_detail video_detail;
			get_video_detail(video_detail, item.string());

			if(std::find(encode_data->extensions_vector.begin(), encode_data->extensions_vector.end(), item.extension().string()) != encode_data->extensions_vector.end() )
			{
				if( encoder.res.height == 0)
				{
					encoder.res = video_detail.res;
				}

				//check the resolution and make it smaller if the sizes are different
				if( encoder.res.height != video_detail.res.height )
				{
					encoder.bRescale = true;
				}
				if( encoder.res.height > video_detail.res.height )
				{
					encoder.res = video_detail.res;
				}
				
				encoder.src_size += video_detail.file_size;
			
				string filename=item.filename().string();
				string filename_without_extension=filename.substr(0, filename.find_last_of('.'));
				//check to see if new file has NOT already been created and it matches the input wildcard 
				if (!has_been_converted(filename_without_extension)) 
				{
					if (filename_without_extension.find(current_dir_name_wo_dates) == 0) 
					{
						append_to_file(encoder.input_file, "file '" + item.string() + "'\n");
					}
					else
						log(WARNING, "process_into_single_file: error - WILDCARD " + filename_without_extension + " DOES NOT matches " + current_dir_name_wo_dates);
					
				} 
				else
					log (DEBUG, "process_into_single_file: info - CHECK " + filename_without_extension + " does matches check string");
			}
			else
				log (DEBUG, "process_into_single_file: Extension is not in the vector " + item.extension().string());*/			
			
        }
		else
			log (DEBUG, "process_into_single_file: " + item.string()  + " is not a file");
    }
	
	if (fileExists(encoder.input_file, true)) 
    {
		encoder.bMultifile = true;
		//check to see if it's not empty
		dump_directory(_sourcePath);
//		if (encode_video(encoder.input_file, encoder.output_video, encoder.bitrate, 0, 1)) 
		if (encode_video(encoder)) 
        {
			//process_outputFile(output_video, current_dir_name + "." + encode_data->addon_string, _sourcePath, src_size);
			process_outputFile(encoder.output_video, _sourcePath, encoder.src_size);	
		}
        else
        {
			if (fileExists(encoder.output_video)) 
            {
				move_file(encoder.output_video, encode_data->trash_path);
			}
		}
    }
    else
    {
		log (DEBUG, "process_into_single_file: output_file is empty " + encoder.input_file);
		delete_file(encoder.input_file);
	}
    return 0;
}

int EncoderBase::process_consolidated(const string& _sourcePath)
{
    log(INFO, "process_consolidated: " + _sourcePath);
	
	//check to see if current directory has files in it
	if (count_files_in_directory(_sourcePath) > 0) 
	{
		log(DEBUG, "process_consolidated: There are files in the directory " + _sourcePath);
		process_into_single_file(_sourcePath);
		//return 1;
	}
				
		
	if (pathExists(_sourcePath)) 
    {
		std::vector<std::filesystem::path> files_in_directory;
		std::copy(std::filesystem::directory_iterator(_sourcePath), std::filesystem::directory_iterator(), std::back_inserter(files_in_directory));
		std::sort(files_in_directory.begin(), files_in_directory.end());

		for (const auto& itr : files_in_directory) 
		{
            fs::path item = itr;
			if (pathExists(item)) 
			{
				int32_t count = count_files_in_directory(item.string());
				if (count > 0) 
                {
					//log "$INFO" "There are files in the directory."
					log(DEBUG, "process_consolidated " + item.string() + " has " + to_string(count) + " files.");
					process_into_single_file(item.string());
				} 
                else 
                {
					//log "$INFO" "There are NO files in the directory."
					log(DEBUG, "process_consolidated " + item.string() + " has no files.");
					process_consolidated(item.string());
				
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

void EncoderBase::init_decode()
{
	encode_data->decode_string= add_decode_setting("");
	log (INFO,  "init_decode: " + encode_data->decode_string);
}

void EncoderBase::init_encode()
{
	encode_data->encode_string = add_encode_setting() + add_encoder() + add_CRF() + add_maxrate() + add_bufsize() + add_preset() + add_audio_encode();
	log (INFO,  "init_encode: " + encode_data->encode_string);
}


struct resolution EncoderBase::get_resolution(const string& input_file)
{
	struct resolution res;
    res.height = 0;
    res.width = 0;
	return res;
}

string EncoderBase::get_convert_string()
{
	return "";	
}
void EncoderBase::get_video_detail(struct video_detail& data, const string& fileName)
{
	fs::path item = fileName;

    data.filename = fileName;
    data.file_size = get_filesize(fileName);
    data.bitrate = get_bitrate(fileName, data.file_size);
    data.audio_bitrate = get_audio_bitrate(fileName);
    data.res = get_resolution(fileName);

	data.extension = item.extension().string();
	data.codec = get_decode_codec(fileName);
	if (data.codec.find('\n') != std::string::npos) 
	{
        // Remove line breaks
        data.codec.erase(std::remove(data.codec.begin(), data.codec.end(), '\n'), data.codec.end());
	}		
}

void EncoderBase::dump(struct video_detail& data, int type)
{
	log (INFO, "");
	log (INFO, "********************************************");
	if(type == 0) 
		log (INFO, "*****             SOURCE             *******");
	else
		log (INFO, "*****             TARGET             *******");
	
	log (INFO, "*****");
	dump(data);
	log (INFO, "*****");
	log (INFO, "********************************************");
	log (INFO, "");

}

void EncoderBase::dump(struct video_detail& data)
{
	log (INFO, "Filename: " + data.filename);
	log (INFO, "File Size: " + to_string(data.file_size));
	log (INFO, "Bitrate: " + to_string(data.bitrate));
	log (INFO, "Audio Bitrate: " + to_string(data.audio_bitrate));
	log (INFO, "Codec: " + data.codec);
	log (INFO, "Extension: " + data.extension);
	log (INFO, "Resolution: " + to_string(data.res.width) + " x " + to_string(data.res.height));
}

void EncoderBase::dump(const string& filename, int type)
{
	struct video_detail video_detail;
	get_video_detail(video_detail, filename);
	dump(video_detail, type);
}

void EncoderBase::dump_directory(const string& path, int type)
{
	string filename;
	int64_t size = 0;
	int64_t bitrate = 0;
				
	log (INFO, "");
	log (INFO, "********************************************");
	if(type == 0) 
		log (INFO, "*****             SOURCE             *******");
	else
		log (INFO, "*****             TARGET             *******");
	log (INFO, "*****");
	
	std::vector<std::filesystem::path> files_in_directory;
	std::copy(std::filesystem::directory_iterator(path), std::filesystem::directory_iterator(), std::back_inserter(files_in_directory));
	std::sort(files_in_directory.begin(), files_in_directory.end());

	for (const auto& itr : files_in_directory) 
	{		
		fs::path item = itr;
		if (fileExists(item)) 
		{
			struct video_detail video_detail;
			get_video_detail(video_detail, item.string());
			dump(video_detail);
		}
		log (INFO, "*****");
	}
	log (INFO, "********************************************");
	log (INFO, "");

}

