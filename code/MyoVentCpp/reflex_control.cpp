/**
/* @file		reflex_control.cpp
/* @brief		Source file for a reflex control object
/* @author		Ken Campbell
*/

#include "stdio.h"
#include "math.h"

#include <iostream>
#include <sstream>

#include "reflex_control.h"

#include "cmv_model.h"
#include "cmv_system.h"
#include "cmv_results.h"
#include "cmv_options.h"

#include "baroreflex.h"
#include "circulation.h"
#include "hemi_vent.h"
#include "half_sarcomere.h"
#include "heart_rate.h"
#include "membranes.h"
#include "myofilaments.h"

#include "gsl_math.h"

struct cmv_model_rc_structure {
	string level;
	string variable;
	double k_control;
	double k_recov;
	double para_factor;
	double symp_factor;
};

// Constructor
reflex_control::reflex_control(baroreflex* set_p_parent_baroreflex, int set_rc_number,
	cmv_model_rc_structure* p_struct)
{
	// Initialise

	// Code
	
	// Set pointers
	p_parent_baroreflex = set_p_parent_baroreflex;

	p_parent_circulation = p_parent_baroreflex->p_parent_circulation;
	p_parent_cmv_system = p_parent_circulation->p_parent_cmv_system;
	p_cmv_model = p_parent_circulation->p_cmv_model;

	// Initialise with safe options
	p_cmv_results = NULL;
	p_cmv_options = NULL;

	// Other variables
	rc_number = set_rc_number;
	rc_baro_C = 0.5;

	// Others come from the structure
	rc_level = p_struct->level;
	rc_variable = p_struct->variable;
	rc_k_control = p_struct->k_control;
	rc_k_recov = p_struct->k_recov;
	rc_para_factor = p_struct->para_factor;
	rc_symp_factor = p_struct->symp_factor;

	cout << "rc_variable: " << rc_variable << "\n";
}

// Destructor
reflex_control::~reflex_control(void)
{
	// Tidy up
}

// Other functions
void reflex_control::initialise_simulation(void)
{
	//! Code initialises simulation
	
	// Variables
	bool reflex_assigned = false;
	
	// Initialise options
	p_cmv_options = p_parent_circulation->p_cmv_options;

	// Now add in the results
	p_cmv_results = p_parent_circulation->p_cmv_results;

	// Find the variable this object is controlling
	if (rc_level == "heart_rate")
	{
		if (rc_variable == "t_RR_interval")
		{
			p_controlled_variable = &p_parent_circulation->p_hemi_vent->p_hs->p_heart_rate->hr_t_RR_interval_s;
			reflex_assigned = true;
		}
	}

	if (rc_level == "membranes")
	{
		if (rc_variable == "k_act")
		{
			p_controlled_variable = &p_parent_circulation->p_hemi_vent->p_hs->p_membranes->memb_k_active;
			reflex_assigned = true;
		}
		
		if (rc_variable == "k_serca")
		{
			p_controlled_variable = &p_parent_circulation->p_hemi_vent->p_hs->p_membranes->memb_k_serca;
			reflex_assigned = true;
		}
	}

	if (rc_level == "myofilaments")
	{
		if (rc_variable == "k_on")
		{
			p_controlled_variable = &p_parent_circulation->p_hemi_vent->p_hs->p_myofilaments->myof_a_k_on;
			reflex_assigned = true;
		}

		if (rc_variable.rfind("m_state", 0) == 0)
		{
			int found;
			string temp;
			stringstream s;
			
			s << rc_variable;

			while (!s.eof())
			{
				s >> temp;

				cout << "ken\n";

				if (stringstream(temp) >> found)
					cout << "xx: " << found << "\n";
				else
					cout << "yy\n";
			}
			reflex_assigned = false;
		}
	}


	// Add fields
	string temp_string = "reflex_control_" + to_string(rc_number) + "_C";
	p_cmv_results->add_results_field(temp_string, &rc_baro_C);
}

void reflex_control::implement_time_step(double time_step_s)
{
	//! Implements time-step
	
	// Variables

	// Code
}
