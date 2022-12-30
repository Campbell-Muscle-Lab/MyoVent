/**
/* @file		cmv_protocol.cpp
/* @brief		Source file for a cmv_protocol object
/* @author		Ken Campbell
*/

#include <iostream>
#include <filesystem>

#include "JSON_functions.h"

#include "rapidjson\document.h"
#include "rapidjson\filereadstream.h"

#include "cmv_protocol.h"

using namespace std::filesystem;
using namespace std;

// Constructor
cmv_protocol::cmv_protocol(string set_protocol_file_string)
{
	// Initialise

	// Code
	cout << "cmv_protocol constructor\n";

	// Initialise variables
	protocol_file_string = set_protocol_file_string;
	time_step_s = 0.0;
	no_of_time_steps = 0;

	// Now update from file
	initialise_protocol_from_JSON_file(protocol_file_string);
}

// Destructor
cmv_protocol::~cmv_protocol(void)
{
	// Initialise

	// Code
	cout << "cmv_protocol desctructor\n";
}

void cmv_protocol::initialise_protocol_from_JSON_file(string protocol_file_string)
{
	//! Code initialises a protocol from file

	// Variables
	errno_t file_error;
	FILE* fp;
	char readBuffer[65536];

	// Code
	file_error = fopen_s(&fp, protocol_file_string.c_str(), "rb");
	if (file_error != 0)
	{
		cout << "Error opening protocol file: " << protocol_file_string;
		exit(1);
	}

	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	rapidjson::Document doc;
	doc.ParseStream(is);

	fclose(fp);

	// Now trying to read file
	cout << "Parsing protocol file: " << protocol_file_string << "\n";

	// Load the protocol object
	JSON_functions::check_JSON_member_object(doc, "protocol");
	const rapidjson::Value& prot = doc["protocol"];

	// Load protocol variables
	JSON_functions::check_JSON_member_number(prot, "time_step");
	time_step_s = prot["time_step"].GetDouble();

	JSON_functions::check_JSON_member_int(prot, "no_of_time_steps");
	no_of_time_steps = prot["no_of_time_steps"].GetInt();
}
