/**
/* @file		myofilaments.cpp
/* @brief		Source file for a myofilaments object
/* @author		Ken Campbell
*/

#include <iostream>

#include "myofilaments.h"
#include "half_sarcomere.h"
#include "kinetic_scheme.h"
#include "m_state.h"
#include "transition.h"
#include "membranes.h"
#include "cmv_model.h"
#include "cmv_options.h"
#include "cmv_results.h"

#include "gsl_errno.h"
#include "gsl_odeiv2.h"

using namespace std;

// Constructor
myofilaments::myofilaments(half_sarcomere* set_p_parent_hs)
{
	//! Constructor

	// Code
	cout << "myofilaments constructor()\n";

	// Set the pointers to the appropriate places
	p_parent_hs = set_p_parent_hs;
	p_cmv_model = p_parent_hs->p_cmv_model;
	
	p_m_scheme = p_cmv_model->p_m_scheme;

	// Set other pointers safe
	p_cmv_results = NULL;
	p_cmv_options = NULL;
	x = NULL;
	y = NULL;
	m_y_indices = NULL;
	m_state_pops = NULL;

	// Initialize
	myof_cb_number_density = p_cmv_model->myof_cb_number_density;
	myof_prop_fibrosis = p_cmv_model->myof_prop_fibrosis;
	myof_prop_myofilaments = p_cmv_model->myof_prop_myofilaments;
	myof_k_cb = p_cmv_model->myof_k_cb;

	myof_a_k_on = p_cmv_model->myof_a_k_on;
	myof_a_k_off = p_cmv_model->myof_a_k_off;
	myof_a_k_coop = p_cmv_model->myof_a_k_coop;

	myof_a_off = 0.0;
	myof_a_on = 0.0;

	myof_m_bound = 0.0;
	myof_f_overlap = 1.0;

	m_pop_array = (double*)malloc(p_m_scheme->no_of_states * sizeof(double));
}

// Destructor
myofilaments::~myofilaments(void)
{
	//! Destructor

	// Code
	cout << "Myofilaments destructor()\n";

	// Tidy up
	if (x != NULL)
	{
		gsl_vector_free(x);
	}
	if (y != NULL)
	{
		gsl_vector_free(y);
	}
	if (m_y_indices != NULL)
	{
		gsl_matrix_int_free(m_y_indices);
	}
	if (m_state_pops != NULL)
	{
		gsl_vector_free(m_state_pops);
	}

	free(m_pop_array);
}

// Other functions
void myofilaments::prepare_for_cmv_results(void)
{
	//! Function adds data fields to main results object

	// Variables

	// Initialize

	// Set the pointer to the results object
	p_cmv_results = p_parent_hs->p_cmv_results;

	// Now add the results fields
	p_cmv_results->add_results_field("myof_a_off", &myof_a_off);
	p_cmv_results->add_results_field("myof_a_on", &myof_a_on);
	p_cmv_results->add_results_field("myof_f_overlap", &myof_f_overlap);
	p_cmv_results->add_results_field("myof_m_bound", &myof_m_bound);

	for (int i = 0; i < p_m_scheme->no_of_states; i++)
	{
		string label = string("myof_m_pop_") + to_string(i);
		p_cmv_results->add_results_field(label, &m_pop_array[i]);
	}
}

void myofilaments::update_p_cmv_options(cmv_options* set_p_cmv_options)
{
	//! Function updates the pointer to the cmv_options
	//! This is not available when the myofilaments are constructed
	//! from a model file

	p_cmv_options = set_p_cmv_options;

	// Now update the daughter kinetic_scheme
	p_m_scheme->update_p_cmv_options(p_cmv_options);
}

void myofilaments::initialise_simulation(void)
{
	//! Code initialises simulation

	// Variables

	// Code
	no_of_bin_positions = 1 + (int)(p_cmv_options->bin_max - p_cmv_options->bin_min) /
		p_cmv_options->bin_width;

	// Assign x
	x = gsl_vector_alloc(no_of_bin_positions);

	for (int i = 0; i < no_of_bin_positions; i++)
	{
		gsl_vector_set(x, i, p_cmv_options->bin_min + ((double)i * p_cmv_options->bin_width));
	}

	cout << "Last bin x: " << gsl_vector_get(x, no_of_bin_positions - 1) << "\n";

	// Now set the length of the system from the kinetic scheme + 2 for thin filament
	y_length = p_m_scheme->no_of_detached_states +
		(p_m_scheme->no_of_attached_states * no_of_bin_positions) +
		2;

	m_length = y_length - 2;

	// Allocate space
	y = gsl_vector_alloc(y_length);

	// Set indices
	a_off_index = y_length - 2;
	a_on_index = y_length - 1;

	// Initialise with all myosins in y[0] and actins in y[end-2]
	gsl_vector_set_zero(y);
	gsl_vector_set(y, 0, 1.0);
	gsl_vector_set(y, a_off_index, 1.0);

	// Initialise and zero the m_bin_indices
	m_y_indices = gsl_matrix_int_alloc(p_m_scheme->no_of_states, 2);
	gsl_matrix_int_set_zero(m_y_indices);

	int y_index = 0;
	int state_counter;
	for (int state_counter = 0; state_counter < p_m_scheme->no_of_states; state_counter++)
	{
		// Check whether state is attached
		if (p_m_scheme->p_m_states[state_counter]->state_type == 'A')
		{
			gsl_matrix_int_set(m_y_indices, state_counter, 0, y_index);
			y_index = y_index + no_of_bin_positions - 1;
			gsl_matrix_int_set(m_y_indices, state_counter, 1, y_index);
		}
		else
		{
			gsl_matrix_int_set(m_y_indices, state_counter, 0, y_index);
			gsl_matrix_int_set(m_y_indices, state_counter, 1, y_index);
		}

		y_index = y_index + 1;
	}

	cout << "bin indices \n";
	for (int i = 0; i < p_m_scheme->no_of_states; i++)
	{
		cout << gsl_matrix_int_get(m_y_indices, i, 0) << "   " << gsl_matrix_int_get(m_y_indices, i, 1) << "\n";
	}

	// Initialise and set the bin populations
	m_state_pops = gsl_vector_alloc(p_m_scheme->no_of_states);

	// Update class variables
	myof_a_off = gsl_vector_get(y, a_off_index);
	myof_a_on = gsl_vector_get(y, a_on_index);
}

// This function is not a member of the membranes class but is used to interace
// with the GSL ODE system. It must appear before the membranes class members
// that calls it, and communicates with the membrane class through a pointer to
// the class object

int myof_calculate_derivs(double t, const double y[], double f[], void* params)
{
	// Function sets derivs

	// Variables
	(void)(t);

	myofilaments* p_myof = (myofilaments*)params;

	double rate;

	double hs_force;
	double hs_length;

	double x_pos;
	double x_ext;

	double f_overlap;
	double m_bound;

	double J_on;
	double J_off;

	double flux;

	int current_ind;
	int new_ind;
	
	// Code

	// Calculat f_overlap
	p_myof->calculate_f_overlap();
	f_overlap = p_myof->myof_f_overlap;

	// Calculate state populations
	p_myof->calculate_m_state_pops(y);
	m_bound = p_myof->myof_m_bound;

	// Set state variables from parent
	hs_force = p_myof->p_parent_hs->hs_force;
	hs_length = p_myof->p_parent_hs->hs_length;

	// Initalise derivs
	for (int i = 0; i < p_myof->y_length; i++)
	{
		f[i] = 0.0;
	}

	// Start with myosin

	// Work through the states
	for (int state_counter = 0; state_counter < p_myof->p_m_scheme->no_of_states;
		state_counter++)
	{
		char current_state_type = p_myof->p_m_scheme->p_m_states[state_counter]->state_type;

		// Now through the transitions
		for (int t_counter = 0; t_counter < p_myof->p_m_scheme->max_no_of_transitions;
			t_counter++)
		{
			int new_state = p_myof->p_m_scheme->p_m_states[state_counter]->
					p_transitions[t_counter]->new_state;

			if (new_state == 0)
			{
				// Transition is not allowed out - skip out
				continue;
			}

			char new_state_type = p_myof->p_m_scheme->p_m_states[new_state - 1]->state_type;

			if ((current_state_type == 'S') || (current_state_type == 'D'))
			{
				// Deatched state
				if ((new_state_type == 'S') || (new_state_type == 'D'))
				{
					// Detached to detached
					rate = p_myof->p_m_scheme->p_m_states[state_counter]->p_transitions[t_counter]->
						calculate_rate(0, 0, hs_force, hs_length);

					// Find current index
					current_ind = gsl_matrix_int_get(p_myof->m_y_indices, state_counter, 0);

					// Find flux
					flux = rate * y[current_ind];

					// Cross-bridges leaving current state
					f[current_ind] = f[current_ind] - flux;

					// Cross-bridges arriving at new state
					new_ind = gsl_matrix_int_get(p_myof->m_y_indices, new_state - 1, 0);

					f[new_ind] = f[new_ind] + flux;
				}
				else
				{
					// Detached to attached
					current_ind = gsl_matrix_int_get(p_myof->m_y_indices, state_counter, 0);

					// Cycle through the bins
					for (int bin_index = 0; bin_index < p_myof->no_of_bin_positions;
						bin_index++)
					{
						// Get x position
						x_pos = gsl_vector_get(p_myof->x, bin_index);
						x_ext = p_myof->p_m_scheme->p_m_states[state_counter]->extension;

						// Calculate rate
						rate = 	p_myof->p_m_scheme->p_m_states[state_counter]->
							p_transitions[t_counter]->calculate_rate(x_pos, x_ext, hs_force, hs_length);

						flux = p_myof->p_cmv_options->bin_width *
							rate * y[current_ind] * (y[p_myof->a_on_index] - m_bound);

						new_ind = gsl_matrix_int_get(p_myof->m_y_indices, new_state - 1, 0) +
							bin_index;

						// Cross-bridges leaving this state
						f[current_ind] = f[current_ind] - flux;

						// Cross-bridges arriving at new state
						f[new_ind] = f[new_ind] + flux;
					}
				}
			}
			else
			{
				// We are in an attached state
				if ((new_state_type == 'S') || (new_state_type == 'D'))
				{
					// Attached to detached
					new_ind = gsl_matrix_int_get(p_myof->m_y_indices, new_state - 1, 0);

					// Cycle through the bins
					for (int bin_index = 0; bin_index < p_myof->no_of_bin_positions;
						bin_index++)
					{
						// Get x position
						x_pos = gsl_vector_get(p_myof->x, bin_index);
						x_ext = p_myof->p_m_scheme->p_m_states[state_counter]->extension;

						current_ind = gsl_matrix_int_get(p_myof->m_y_indices, state_counter, 0) + bin_index;

						// Calculate rate
						rate = p_myof->p_m_scheme->p_m_states[state_counter]->
							p_transitions[t_counter]->calculate_rate(x_pos, x_ext, hs_force, hs_length);

						flux =	rate * y[current_ind];

						// Cross-bridges leaving this state
						f[current_ind] = f[current_ind] - flux;

						// Cross-bridges arriving at new state
						f[new_ind] = f[new_ind] + flux;
					}
				}
				else
				{
					// Cross-bridges transitioning between bound states

					// Cycle through the bins
					for (int bin_index = 0; bin_index < p_myof->no_of_bin_positions;
						bin_index++)
					{
						// Get x position
						x_pos = gsl_vector_get(p_myof->x, bin_index);
						x_ext = p_myof->p_m_scheme->p_m_states[state_counter]->extension;

						current_ind = gsl_matrix_int_get(p_myof->m_y_indices, state_counter, 0) +
							bin_index;

						// Calculate rate
						rate = p_myof->p_m_scheme->p_m_states[state_counter]->
							p_transitions[t_counter]->calculate_rate(x_pos, x_ext, hs_force, hs_length);

						new_ind = gsl_matrix_int_get(p_myof->m_y_indices, new_state - 1, 0) +
							bin_index;

						flux = rate * y[current_ind];

						// Cross-bridges leaving this state
						f[current_ind] = f[current_ind] - flux;

						// Cross-bridgse arriving at new state
						f[new_ind] = f[new_ind] + flux;
					}
				}
			}
/*
			printf("flux\n");
			for (int i = 0; i < p_myof->y_length; i++)
			{
				printf("f[%i]: %.3f\t", i, f[i]);
				if (i == (p_myof->y_length - 1))
					printf("\n");
			}
*/
		}
	}

	// Now handle the actin
	if (f_overlap > 0.0)
	{
		J_on = p_myof->myof_a_k_on *
			(p_myof->p_parent_hs->p_membranes->memb_Ca_cytosol) *
			(f_overlap - y[p_myof->a_on_index]) *
			(1.0 + (p_myof->myof_a_k_coop * (y[p_myof->a_on_index] / f_overlap)));

		J_off = p_myof->myof_a_k_off *
			(y[p_myof->a_on_index] - m_bound) *
			(1.0 + (p_myof->myof_a_k_coop * ((f_overlap - y[p_myof->a_on_index]) / f_overlap)));
	}
	else
	{
		J_on = 0.0;
		J_off = p_myof->myof_a_k_off * (y[p_myof->a_on_index] - m_bound);
	}

	f[p_myof->a_off_index] = -J_on + J_off;
	f[p_myof->a_on_index] = -f[p_myof->a_off_index];

	return GSL_SUCCESS;
}

void myofilaments::implement_time_step(double time_step_s)
{
	//! Code advances the simulation by time_step

	// Variables

	// Code

	// Variables
	double eps_abs = 1e-4;
	double eps_rel = 1e-4;

	int status;

	double t_start_s = 0.0;
	double t_stop_s = time_step_s;

	double* y_calc = NULL;
	
	// Code

	// Allocate memory for y
	y_calc = (double*)malloc(y_length * sizeof(double));
	
	// Fill y_calc
	for (int i = 0; i < y_length; i++)
	{
		y_calc[i] = gsl_vector_get(y, i);
	}

	gsl_odeiv2_system sys = { myof_calculate_derivs, NULL, y_length, this };

	gsl_odeiv2_driver* d =
		gsl_odeiv2_driver_alloc_y_new(&sys, gsl_odeiv2_step_rkf45,
			0.5*time_step_s, eps_abs, eps_rel);

	status = gsl_odeiv2_driver_apply(d, &t_start_s, t_stop_s, y_calc);

	if (status != GSL_SUCCESS)
	{
		std::cout << "Integration problem in membranes::implement_time_step\n";
	}
	else
	{
		//Unpack
		for (int i = 0; i < y_length; i++)
		{
			gsl_vector_set(y, i, y_calc[i]);
		}
	}

	// Update class variables
	calculate_m_state_pops(y_calc);

	for (int i = 0; i < p_m_scheme->no_of_states; i++)
	{
		m_pop_array[i] = gsl_vector_get(m_state_pops, i);

		/*
		printf("\t%.3f", m_pop_array[i]);
		if (i == (p_m_scheme->no_of_states - 1))
			printf("\n");
		*/
	}


	myof_a_off = gsl_vector_get(y, a_off_index);
	myof_a_on = gsl_vector_get(y, a_on_index);

	// Tidy up
	free(y_calc);
}

void myofilaments::calculate_m_state_pops(const double y_calc[])
{
	//! Function returns m_bound

	// Variables
	double holder;
	double bound_holder;

	// Code

	bound_holder = 0.0;

	for (int state_counter = 0; state_counter < p_m_scheme->no_of_states;
		state_counter++)
	{
		holder = 0.0;

		for (int i = gsl_matrix_int_get(m_y_indices, state_counter, 0);
			i <= gsl_matrix_int_get(m_y_indices, state_counter, 1); i++)
		{
			holder = holder + y_calc[i];
		}
		gsl_vector_set(m_state_pops, state_counter, holder);

		if (p_m_scheme->p_m_states[state_counter]->state_type == 'A')
		{
			bound_holder = bound_holder + holder;
		}
	}

	myof_m_bound = bound_holder;
}

void myofilaments::calculate_f_overlap(void)
{
	//! Calculate f_overlap

	myof_f_overlap = 1.0;
}
