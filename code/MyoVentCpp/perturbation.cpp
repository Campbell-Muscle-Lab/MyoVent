/**
/* @file		activation.cpp
/* @brief		Source file for an activation object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>
#include <regex>

#include "cmv_protocol.h"
#include "perturbation.h"
#include "cmv_system.h"
#include "circulation.h"
#include "hemi_vent.h"
#include "valve.h"
#include "half_sarcomere.h"
#include "membranes.h"
#include "mitochondria.h"
#include "myofilaments.h"
#include "kinetic_scheme.h"
#include "transition.h"
#include "baroreflex.h"

#include "gsl_math.h"

using namespace::std;

struct perturbation_struct {
	string class_name;
	string variable;
	double t_start_s;
	double t_stop_s;
	double total_change;
};

// Constructor
perturbation::perturbation(cmv_protocol* set_p_cmv_protocol, perturbation_struct* p_struct)
{
	// Initialise

	// Code
	p_cmv_protocol=set_p_cmv_protocol;

	prot_time_step_s = p_cmv_protocol->time_step_s;

	class_name = p_struct->class_name;
	variable = p_struct->variable;
	t_start_s = p_struct->t_start_s;
	t_stop_s = p_struct->t_stop_s;
	total_change = p_struct->total_change;

	// Set the increment
	double n_steps = GSL_MAX(1.0, (t_stop_s - t_start_s) / prot_time_step_s);

	increment = total_change / n_steps;

	cout << "n_steps: " << n_steps << " total_change: " << total_change << " increment " << increment << "\n";
}

// Destructor
perturbation::~perturbation(void)
{
	// Destructor
	// 
	// Tidy up
}

// Other functions

void perturbation::impose(double sim_time_s)
{
	//! Function imposes perturbation
	
	// Variables
	double* p_double;

	double delta_n_hs;
	double delta_hs_length;

	// Code

	if ((sim_time_s >= t_start_s) && (sim_time_s <= t_stop_s))
	{
		if (class_name == "baroreflex")
		{
			if (variable == "baro_P_set")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_baroreflex->baro_P_set);

				*p_double = *p_double + increment;
			}
		}

		if (class_name == "circulation")
		{
			if (variable.rfind("resistance", 0) == 0)
			{
				// Starts with resistance
				int no_of_digits = 1;
				int digits[1];
				int compartment_index;

				for (int i = 0; i < no_of_digits; i++)
					digits[i] = 0;

				extract_digits(variable, digits, 1);

				compartment_index = digits[0] - 1;

				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->circ_resistance[compartment_index]);

				*p_double = *p_double + increment;
			}

			if (variable == "blood_volume")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->circ_blood_volume);

				*p_double = *p_double + increment;
			}


		}

		if (class_name == "ventricle")
		{
			if (variable == "vent_wall_volume")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->vent_wall_volume);

				*p_double = *p_double + increment;
			}

			if (variable == "vent_n_hs")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->vent_n_hs);

				// When we change vent_n_hs, we need to adjust the length of the existing half-sarcomeres
				// and the wall volume as well

				delta_n_hs = increment;

				// Work out how far half-sarcomeres move using chain rule
				delta_hs_length = -(delta_n_hs * p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_hs->hs_length) /
					p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->vent_n_hs;

				// Apply to half-sarcomere
				p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_hs->change_hs_length(delta_hs_length);

				// And the wall volume
				p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->vent_wall_volume =
					p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->vent_wall_volume *
					(1.0 + (delta_n_hs / p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->vent_n_hs));

				// Finally increment the vent_n_hs
				*p_double = *p_double + increment;
			}
		}

		if (class_name == "valve")
		{
			if (variable == "mv_valve_k")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_mv->valve_k);

				*p_double = *p_double + increment;
			}

			if (variable == "mv_valve_mass")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_mv->valve_mass);

				*p_double = *p_double + increment;
			}

			if (variable == "mv_valve_eta")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_mv->valve_eta);

				*p_double = *p_double + increment;
			}

			if (variable == "mv_valve_leak")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_mv->valve_leak);

				*p_double = *p_double + increment;
			}

			if (variable == "av_valve_k")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_av->valve_k);

				*p_double = *p_double + increment;
			}

			if (variable == "av_valve_mass")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_av->valve_mass);

				*p_double = *p_double + increment;
			}

			if (variable == "av_valve_eta")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_av->valve_eta);

				*p_double = *p_double + increment;
			}

			if (variable == "av_valve_leak")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_av->valve_leak);

				*p_double = *p_double + increment;
			}
		}

		if (class_name == "half_sarcomere")
		{
			if (variable == "prop_fibrosis")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_hs->hs_prop_fibrosis);

				*p_double = *p_double + increment;
			}
		}
		if (class_name == "membranes")
		{
			if (variable == "t_open")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_hs->p_membranes->memb_t_open_s);

				*p_double = *p_double + increment;
			}

			if (variable == "k_leak")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_hs->p_membranes->memb_k_leak);

				*p_double = *p_double + increment;
			}
		}


		if (class_name == "mitochondria")
		{
			if (variable == "ATP_generation_rate")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_hs->
					p_mitochondria->mito_ATP_generation_rate);

				*p_double = *p_double + increment;
			}
		}

		if (class_name == "myofilaments")
		{
			if (variable.rfind("m_state", 0) == 0)
			{
				// Starts with m_state
				int no_of_digits = 3;
				int digits[3];
				int state_index;
				int transition_index;
				int parameter_index;

				for (int i = 0; i < no_of_digits; i++)
					digits[i] = 0;

				extract_digits(variable, digits, 3);

				state_index = digits[0] - 1;
				transition_index = digits[1] - 1;
				parameter_index = digits[2] - 1;

				// This is tricky because the variable is stored in a gsl_vector
				gsl_vector* p_gsl_v = p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->
					p_hs->p_myofilaments->p_m_scheme->p_m_states[state_index]->p_transitions[transition_index]->rate_parameters;
				
				gsl_vector_set(p_gsl_v, parameter_index,
					gsl_vector_get(p_gsl_v, parameter_index) + increment);
			}

			if (variable == "a_k_on")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_hs->
					p_myofilaments->myof_a_k_on);

				*p_double = *p_double + increment;
			}

			if (variable == "a_k_off")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_hs->
					p_myofilaments->myof_a_k_off);

				*p_double = *p_double + increment;
			}

			if (variable == "a_k_coop")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_hs->
					p_myofilaments->myof_a_k_coop);

				*p_double = *p_double + increment;
			}

			if (variable == "int_pas_L")
			{
				p_double = &(p_cmv_protocol->p_cmv_system->p_circulation->p_hemi_vent->p_hs->
					p_myofilaments->myof_int_pas_L);

				*p_double = *p_double + increment;
			}
		}
	}
}

void perturbation::extract_digits(string test_string, int digits[], int no_of_digits)
{
	//! Function fills digits array with [0-9] extracted from string
	//! See https://en.cppreference.com/w/cpp/regex/regex_iterator

	// Variables
	int counter;

	// Code
	regex ex("[0-9]");

	auto digits_begin = sregex_iterator(test_string.begin(), test_string.end(), ex);
	auto digits_end = sregex_iterator();

	counter = 0;
	for (regex_iterator i = digits_begin; i != digits_end; i++)
	{
		smatch match = *i;
		digits[counter] = atoi((match.str().c_str()));
		counter = counter + 1;
	}
}
