#pragma once

/**
/* @file		hemi_vent.h
/* @brief		Header file for a hemi_vent object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>

#include "global_definitions.h"

// Forward declararations
class cmv_system;
class half_sarcomere;

class cmv_results;

class hemi_vent
{
public:
	/**
	 * Constructor
	 */
	hemi_vent(cmv_system* set_p_parent_cmv_system);

	/**
	 * Destructor
	 */
	 ~hemi_vent(void);

	 // Variables

	cmv_system* p_parent_cmv_system;		/**< pointer to parent cmv_system */

	cmv_results* p_cmv_results;				/**< pointer to cmv_results object */

	half_sarcomere* p_hs;					/**< pointer to child half-sarcomere */

	double pressure_ventricle;				/**< double with pressure in the ventricle in mm Hg */

	// Other functions

	/**
	/* function adds data fields and vectors to the results objet
	*/
	void prepare_for_cmv_results(void);

	void implement_time_step(double time_step_s);
};
