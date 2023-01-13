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
#include "baro_activation.h"

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

	no_of_baro_activations = 0;

	// Null the activation pointers
	for (int i = 0; i < MAX_NO_OF_PERTURBATIONS; i++)
	{
		p_baro_activation[i] = NULL;
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
		if (p_baro_activation[i] != NULL)
			delete p_baro_activation[i];
	}
}

void cmv_protocol::initialise_protocol_from_JSON_file(string protocol_file_string)
{
	//! Code initialises a protocol from file

	// Variables
	errno_t file_error;
	FILE* fp;
	char readBuffer[65536];

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
	if (JSON_functions::check_JSON_member_exists(doc, "baroreflex"))
	{
		const rapidjson::Value& baro = doc["baroreflex"];

		// Pull aray
		JSON_functions::check_JSON_member_array(baro, "activations");
		const rapidjson::Value& act = baro["activations"];

		for (rapidjson::SizeType i = 0; i < act.Size(); i++)
		{
			const rapidjson::Value& temp = act[i];
			
			JSON_functions::check_JSON_member_number(temp, "t_start_s");
			t_start_s = temp["t_start_s"].GetDouble();

			JSON_functions::check_JSON_member_number(temp, "t_stop_s");
			t_stop_s = temp["t_stop_s"].GetDouble();

			p_baro_activation[i] = new baro_activation(t_start_s, t_stop_s);

			no_of_baro_activations = no_of_baro_activations + 1;
		}
	}
}

double cmv_protocol::return_baro_activation(double time_s)
{
	//! Function returns the baro_activation status
	
	// Variables
	double baro_activation = 0.0;

	// Code
	for (int i = 0; i < no_of_baro_activations; i++)
	{
		baro_activation = baro_activation + p_baro_activation[i]->return_status(time_s);
	}

	baro_activation = GSL_MIN(1.0, baro_activation);

	return (baro_activation);
}
