#pragma once

/**
/* @file		cmv_system.h
/* @brief		Header file for a cmv_system object
/* @author		Ken Campbell
*/

#include "stdio.h"
#include <string>

// Forward declarations
class cmv_model;
class cmv_options;
class cmv_protocol;
class cmv_results;
class circulation;
class hemi_vent;

using namespace std;

class cmv_system
{
public:
	/**
	 * Constructor
	 */
	cmv_system(string JSON_model_file_string);

	/**
	* Destructor
	*/
	~cmv_system(void);

	// Variables
	cmv_model* p_cmv_model;					/**< Pointer to cmv_model object */

	cmv_options* p_cmv_options;				/**< Poniter to cmv_options object */

	cmv_protocol* p_cmv_protocol;			/**< Pointer to a cmv_protocol object */

	cmv_results*  p_cmv_results;			/**< Pointer to cmv_results */

	circulation* p_circulation;				/**< Pointer to a circulation */

	hemi_vent* p_hemi_vent;					/**< Pointer to a hemi_vent object */

	double cum_time_s;						/** double, with system time in s */

	// Functions

	/**
	/* function runs a simulation
	*/
	void run_simulation(string options_file_string, string protocol_file_string,
		string results_file_string);

	void prepare_for_cmv_results(void);

	void implement_time_step(double time_step_s);
};