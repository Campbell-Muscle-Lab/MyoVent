#pragma once

/**
/* @file		membranes.h
/* @brief		Header file for a membranes object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>

#include "global_definitions.h"

// Forward declararations
class half_sarcomere;
class cmv_results;

class membranes
{
public:
	/**
	 * Constructor
	 */
	membranes(half_sarcomere* set_p_parent_hs);

	/**
	* Destructor
	*/
	~membranes(void);

	// Variables

	half_sarcomere* p_parent_hs;		/**< pointer to parent half-sarcomere */

	cmv_results* p_cmv_results;			/**< pointer to cmv_results object */

	double memb_Ca_cytosol;				/**< double with cytosolic Ca_conc in M */

	double memb_Ca_sr;					/**< double with sarcoplasmc reticular
												Ca concentration in M */

	double memb_activation;				/**< double with membrane activation
												1.0 when active
												0.0 when inactive */

	double memb_t_active_s;				/**< double with active duration in s */

	double memb_t_active_left_s;		/**< double with active duration left
												in s */

	double memb_k_serca;				/**< double describing rate at which
												SERCA pumps Ca in to
												sarcoplasmic reticulum
												in M^-1 s^-1 */

	double memb_k_leak;					/**< double desribing rate at which
												Ca leaks from sarcoplasmic reticulum
												in M s^-1 */

	double memb_k_active;				/**< double describing rate at which
												Ca is released from sarcoplasmic reticulum
												when the membrane is active
												in M s^-1 */

	/**
	/* function adds data fields and vectors to the results objet
	*/
	void prepare_for_cmv_results(void);

	/**
	/* function updates state variables
	*/
	void implement_time_step(double time_step_s, bool new_beat);
};
