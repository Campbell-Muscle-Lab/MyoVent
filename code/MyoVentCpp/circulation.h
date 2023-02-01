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
class cmv_protocol;
class cmv_options;
class cmv_results;

class hemi_vent;
class valve;
class baroreflex;
class growth;

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

	cmv_protocol* p_cmv_protocol;						/**< Pointer to the cmv protocol object */

	cmv_options* p_cmv_options;							/**< Pointer to cmv_options */

	cmv_results*  p_cmv_results;						/**< Pointer to cmv_results */

	hemi_vent* p_hemi_vent;								/**< Pointer to a hemi_vent object */

	valve* p_av;										/**< Pointer to the aortic valve */

	valve* p_mv;										/**< Pointer to the mitral valve */

	baroreflex* p_baroreflex;							/**< Pointer to the baroreflex object */

	growth* p_growth;									/**< Pointer to the growth object */

	double circ_blood_volume;							/**< double holding total blood volume
																in liters */

	int circ_no_of_compartments;						/**< integer holding number of
																compartments */

	double* circ_resistance;							/**< Pointer to array of doubles
																holding resistances for
																each compartment */

	double* circ_compliance;							/**< Pointer to array of doubles
																holding compliances for
																each compartment */

	double* circ_slack_volume;							/**< Pointer to array of doubles
																holding slack_volumes for
																each compartment */

	double* circ_inertance;								/**< Pointer to array of doubles
																holding inertances for
																each compartment */

	double* circ_pressure;								/**< Pointer to array of doubles
																holding pressure in each
																compartment */

	double* circ_volume;								/**< Pointer to array of doubles
																holding pressure in each
																compartment */

	double* circ_flow;									/**< Pointer to array of doubles
																holding flows between
																compartments */

	double* circ_last_flow;								/**< pointer to array of doubles
																holding flows between
																compartments on last
																time_step */

	double circ_total_slack_volume;						/**< double holding total slack
																volume in liters */

	double time_step_s;

	// Functions

	void initialise_simulation(void);

	bool implement_time_step(double time_step_s);

	void calculate_pressures(const double v[], double p[]);

	void calculate_flows(const double v[]);

	void update_beat_metrics(void);
};