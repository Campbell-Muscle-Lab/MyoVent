#pragma once

/**
/* @file		growth.h
/* @brief		Header file for a baroreflex object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>

#include "global_definitions.h"

// Forward declararations
class cmv_model;
class cmv_system;
class cmv_results;
class cmv_options;

class circulation;
class growth_control;

using namespace::std;

class growth
{
public:
	/**
	 * Constructor
	 */
	growth(circulation* set_p_parent_circulation);

	/**
	 * Destructor
	 */
	 ~growth(void);

	 // Variables

	cmv_system* p_parent_cmv_system;		/**< pointer to parent cmv_system */

	cmv_model* p_cmv_model;					/**< pointer to cmv_model object */

	cmv_results* p_cmv_results;				/**< pointer to cmv_results object */

	cmv_options* p_cmv_options;				/**< pointer to cmv_options object */

	circulation* p_parent_circulation;		/**< pointer the parent circulation */

	growth_control* p_gc[MAX_NO_OF_GROWTH_CONTROLS];
											/**< array of pointers to growth controls */

	int no_of_growth_controls;				/**< int with the number of growth controls */

	// Other variables
	double growth_active;						/**< double defining whether the reflex is
													1.0, active
													0.0, inactive */

	double gr_master_rate;

	double gr_shrink_concentric_rate;
	double gr_shrink_eccentric_rate;

	double gr_shrink_concentric_output;
	double gr_shrink_eccentric_output;

	// Other functions
	void initialise_simulation(void);

	void implement_time_step(double time_step_s, bool new_beat);

	//void set_p_gr_shrink_signal(void);
};
