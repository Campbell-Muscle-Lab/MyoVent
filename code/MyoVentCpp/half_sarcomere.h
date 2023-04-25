#pragma once

/**
/* @file		half_sarcomere.h
/* @brief		Header file for a half_sarcomere object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>

#include "global_definitions.h"

// Forward declararations
class cmv_system;
class cmv_model;
class cmv_results;
class cmv_options;

class hemi_vent;
class membranes;
class myofilaments;
class heart_rate;
class mitochondria;

class half_sarcomere
{
public:
	/**
	* Constructor
	*/
	half_sarcomere(hemi_vent* set_p_parent_hemi_vent);

	/**
	* Destructor
	*/
	~half_sarcomere(void);

	// Variables
	hemi_vent* p_parent_hemi_vent;
	
	cmv_model* p_cmv_model;							/**< Pointer to the cmv_model object */

	cmv_results* p_cmv_results;						/**< Pointer to cmv_results */

	cmv_options* p_cmv_options;						/**< Pointer to cmv_options */

	cmv_system* p_cmv_system;						/**< Pointer to the cmv system */

	heart_rate* p_heart_rate;						/**< Pointer to heart-rate object */

	membranes* p_membranes;							/**< Pointer to the daughter
															membranes object */

	mitochondria* p_mitochondria;					/**< Pointer to the daughter
															mitochondria object */

	myofilaments* p_myofilaments;					/**< Pointer to the daughter
															myofilaments object */

	double hs_prop_fibrosis;						/**< Double with the proportion of the
															hs cross-sectional area
															that is fibrosis */

	double hs_prop_myofilaments;					/**< Double with the proportion of the
															hs non-fibrotic area that is
															that is myofilaments, remainder
															is mitochondria */

	double hs_length;
	double hs_reference_hs_length;						
	double hs_stress;

	double hs_ATP_used_per_liter_per_s;				/**< double with ATP used in moles
															per liter per s */

	double hs_ATP_concentration;					/**< double with ATP concentration */

	double hs_delta_G_ATP;							/**< double with energy in Joules
															per mole of ATP */

	/**
	/* function adds data fields and vectors to the results objet
	*/
	
	void initialise_simulation(void);
	
	bool implement_time_step(double time_step_s);

	void change_hs_length(double delta_hsl);

	double return_wall_stress_after_delta_hsl(double delta_hsl);

	double return_hs_length_for_stress(double target_stress);

	void calculate_hs_ATP_concentration(double time_step);
	
	void update_beat_metrics(void);
};