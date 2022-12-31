#pragma once

/**
/* @file		cmv_options.h
/* @brief		Header file for a cmv_options object
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

using namespace std;

class cmv_options
{
public:
	/**
	 * Constructor
	 */
	cmv_options(string set_options_file_string);

	/**
	* Destructor
	*/
	~cmv_options(void);

	// Variables
	string options_file_string;				/**< string for the protocol file */

	double bin_min;							/**< double with minimum cross-bridge
													bin position */

	double bin_max;							/**< double with maximum cross-bridge
													bin position */

	double bin_width;						/**< double with width of cross-bridge
													bin */

	double max_rate;						/**< double with maximum rate for a
													cross-bridge rate in s^-1 */

	/**
	/* Function initialises protocol object from file
	*/
	void initialise_options_from_JSON_file(string JSON_options_file_string);

};