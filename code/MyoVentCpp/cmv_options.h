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

	string rates_dump_relative_to;			/**< string defining path type
													for rates_dump file */

	string rates_dump_file_string;			/**< string with rates file */

	string hv_thick_wall_approximation;		/**< string defining thick wall approximation
													If True, use thick-wall approximation
													otherwise, use thin-wall */

	/**
	/* Function initialises protocol object from file
	*/
	void initialise_options_from_JSON_file(string JSON_options_file_string);

};