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
#include "activation.h"

#include "gsl_math.h"

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

	no_of_activations = 0;

	// Null the activation pointers
	for (int i = 0; i < MAX_NO_OF_ACTIVATIONS; i++)
	{
		p_activation[i] = NULL;
	}

	// Now update from file
	initialise_protocol_from_JSON_file(protocol_file_string);
}

// Destructor
cmv_protocol::~cmv_protocol(void)
{
	// Initialise

	// Code
	cout << "cmv_protocol desctructor\n";

	// Tidy up
	for (int i = 0; i < MAX_NO_OF_PERTURBATIONS; i++)
	{
		if (p_activation[i] != NULL)
			delete p_activation[i];
	}
}

void cmv_protocol::initialise_protocol_from_JSON_file(string protocol_file_string)
{
	//! Code initialises a protocol from file

	// Variables
	errno_t file_error;
	FILE* fp;
	char readBuffer[65536];

	string activation_type;
	double t_start_s;
	double t_stop_s;

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

	// Check for baroreflex activations
	if (JSON_functions::check_JSON_member_exists(doc, "activation"))
	{
		JSON_functions::check_JSON_member_array(doc, "activation");
		const rapidjson::Value& act = doc["activation"];

		// Pull aray
		for (rapidjson::SizeType i = 0; i < act.Size(); i++)
		{
			const rapidjson::Value& temp = act[i];

			JSON_functions::check_JSON_member_string(temp, "type");
			activation_type = temp["type"].GetString();
			
			JSON_functions::check_JSON_member_number(temp, "t_start_s");
			t_start_s = temp["t_start_s"].GetDouble();

			JSON_functions::check_JSON_member_number(temp, "t_stop_s");
			t_stop_s = temp["t_stop_s"].GetDouble();

			p_activation[i] = new activation(activation_type, t_start_s, t_stop_s);

			no_of_activations = no_of_activations + 1;
		}
	}
}

double cmv_protocol::return_activation(string activation_type, double time_s)
{
	//! Function returns the activation status for a given type
	
	// Variables
	double activation = 0.0;

	// Code
	for (int i = 0; i < no_of_activations; i++)
	{
		activation = activation + p_activation[i]->return_status(activation_type, time_s);
	}

	activation = GSL_MIN(1.0, activation);

	return (activation);
}
