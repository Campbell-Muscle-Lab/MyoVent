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
class cmv_model;
class cmv_options;
class cmv_results;
class hemi_vent;

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

	cmv_system* p_parent_cmv_system;					/**< Pointer to the parent_cmv_system */

	cmv_model* p_cmv_model;								/**< Pointer to the cmv_model */

	cmv_options* p_cmv_options;							/**< Pointer to cmv_options */

	cmv_results*  p_cmv_results;						/**< Pointer to cmv_results */

	hemi_vent* p_hemi_vent;								/**< Pointer to a hemi_vent object */

	// Functions

	/**
	/* function adds data fields and vectors to the results objet
	*/
	void prepare_for_cmv_results(void);

	void update_p_cmv_options(void);

	void initialise_simulation(void);

	void implement_time_step(double time_step_s);
};