#pragma once

/**
/* @file		cmv_system.h
/* @brief		Header file for a cmv_system object
/* @author		Ken Campbell
*/

#include "stdio.h"

// Forward declarations
class cmv_results;
class circulation;
class hemi_vent;

class cmv_system
{
public:
	/**
	 * Constructor
	 */
	cmv_system(void);

	/**
	* Destructor
	*/
	~cmv_system(void);

	// Variables
	cmv_results*  p_cmv_results;			/**< Pointer to cmv_results */

	circulation* p_circulation;				/**< Pointer to a circulation */

	hemi_vent* p_hemi_vent;					/**< Pointer to a hemi_vent object */

	// Functions

	/**
	/* function runs a simulation
	*/
	void run_simulation(void);
};