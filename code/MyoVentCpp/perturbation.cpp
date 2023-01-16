/**
/* @file		activation.cpp
/* @brief		Source file for an activation object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include <iostream>

#include "cmv_protocol.h"
#include "perturbation.h"
#include "circulation.h"
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

	// Code

	if ((sim_time_s >= t_start_s) && (sim_time_s <= t_stop_s))
	{
		if (class_name == "baroreflex")
		{
			if (variable == "baro_P_set")
			{
				p_double = &(p_cmv_protocol->p_cmv_sytem->p_circulation->p_baroreflex->baro_P_set);

				*p_double = *p_double + increment;
			}
		}
	}
}
