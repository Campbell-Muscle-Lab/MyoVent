#pragma once

/**
/* @file		cmv_protocol.h
/* @brief		Header file for a cmv_protocol object
/* @author		Ken Campbell
*/

#include "stdio.h"
#include <iostream>
#include <string>

// Definitions for JSON parsing
#ifndef _RAPIDJSON_DOCUMENT
#define _RAPIDJSON_DOCUMENT
#include "rapidjson/document.h"
#endif

#include "global_definitions.h"

using namespace std;

class activation;

class cmv_protocol
{
public:
	/**
	 * Constructor
	 */
	cmv_protocol(string protocol_file_string);

	/**
	* Destructor
	*/
	~cmv_protocol(void);

	// Variables
	string protocol_file_string;			/**< string for the protocol file */

	double time_step_s;						/**< double holding time_step in s */
	
	int no_of_time_steps;					/**< int holding number of time-steps */

	int no_of_activations;					/**< int holding the number of baro activations */

	activation* p_activation[MAX_NO_OF_ACTIVATIONS];
											/**< an array of pointers to baro_activation
													objects */

	// Functions

	/**
	/* Function initialises protocol object from file
	*/
	void initialise_protocol_from_JSON_file(string JSON_protocol_file_string);

	double return_activation(string activation_type, double time_s);
};