#pragma once

/**
/* @file		circulation.h
/* @brief		Header file for a circulation object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>

#include "global_definitions.h"

// Forward declararations
class cmv_system;
class cmv_results;

class circulation
{
public:
	/**
	 * Constructor
	 */
	circulation(cmv_system* set_p_parent_cmv_system);

	/**
	* Destructor
	*/
	~circulation(void);

	// Variables
	double pressure_aorta;

	double* p_data_fields[MAX_NO_OF_RESULT_FIELDS];			/**< array of pointers for data fields */

	std::string data_fields[MAX_NO_OF_RESULT_FIELDS];		/**< array of strings for data fields */

	cmv_system* p_parent_cmv_system;						/**< Pointer to the parent_cmv_system */

	cmv_results*  p_cmv_results;							/**< Pointer to cmv_results */

	// Functions

	/**
	/* function adds data fields and vectors to the results objet
	*/
	void prepare_for_cmv_results(void);
};