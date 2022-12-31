/**
/* @file		cmv_options.cpp
/* @brief		Source file for a cmv_options object
/* @author		Ken Campbell
*/

#include <iostream>
#include <filesystem>

#include "JSON_functions.h"

#include "rapidjson\document.h"
#include "rapidjson\filereadstream.h"

#include "cmv_options.h"

using namespace std::filesystem;
using namespace std;

// Constructor
cmv_options::cmv_options(string set_options_file_string)
{
	// Initialise

	// Code
	cout << "cmv_options constructor\n";

	// Initialise variables
	options_file_string = set_options_file_string;

	// Now update from file
	initialise_options_from_JSON_file(options_file_string);
}

// Destructor
cmv_options::~cmv_options(void)
{
	// Initialise

	// Code
	cout << "cmv_options desctructor\n";
}

void cmv_options::initialise_options_from_JSON_file(string options_file_string)
{
	//! Code initialises options from file

	// Variables
	errno_t file_error;
	FILE* fp;
	char readBuffer[65536];

	// Code
	file_error = fopen_s(&fp, options_file_string.c_str(), "rb");
	if (file_error != 0)
	{
		cout << "Error opening options file: " << options_file_string;
		exit(1);
	}

	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document doc;
	doc.ParseStream(is);

	fclose(fp);

	// Now trying to read file
	cout << "Parsing options file: " << options_file_string << "\n";

	// Load the protocol object
	JSON_functions::check_JSON_member_object(doc, "implementation");
	const rapidjson::Value& impl = doc["implementation"];

	// Load protocol variables
	JSON_functions::check_JSON_member_number(impl, "bin_min");
	bin_min = impl["bin_min"].GetDouble();

	JSON_functions::check_JSON_member_number(impl, "bin_max");
	bin_max = impl["bin_max"].GetDouble();

	JSON_functions::check_JSON_member_number(impl, "bin_width");
	bin_width = impl["bin_width"].GetDouble();

	JSON_functions::check_JSON_member_number(impl, "max_rate");
	max_rate = impl["max_rate"].GetDouble();

	// Check for rates dump
	if (JSON_functions::check_JSON_member_exists(doc, "rates_dump"))
	{
		const rapidjson::Value& rd = doc["rates_dump"];

		if (JSON_functions::check_JSON_member_exists(rd, "relative_to"))
		{
			rates_dump_relative_to = rd["relative_to"].GetString();
		}
		else
		{
			rates_dump_relative_to = "";
		}

		JSON_functions::check_JSON_member_string(rd, "file_string");
		rates_dump_file_string = rd["file_string"].GetString();
	}
	else
	{
		rates_dump_relative_to = "";
		rates_dump_file_string = "";
	}
}
