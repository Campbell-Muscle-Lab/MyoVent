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
#include "gsl_interp.h"
#include "gsl_spline.h"

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
	m_state_stresses = NULL;

	// Initialize
	myof_cb_number_density = p_cmv_model->myof_cb_number_density;
	myof_prop_fibrosis = p_cmv_model->myof_prop_fibrosis;
	myof_prop_myofilaments = p_cmv_model->myof_prop_myofilaments;
	myof_k_cb = p_cmv_model->myof_k_cb;

	myof_int_pas_sigma = p_cmv_model->myof_int_pas_sigma;
	myof_int_pas_L = p_cmv_model->myof_int_pas_L;
	myof_int_pas_slack_hsl = p_cmv_model->myof_int_pas_slack_hsl;

	myof_ext_pas_sigma = p_cmv_model->myof_ext_pas_sigma;
	myof_ext_pas_L = p_cmv_model->myof_ext_pas_L;
	myof_ext_pas_slack_hsl = p_cmv_model->myof_ext_pas_slack_hsl;

	myof_fil_compliance_factor = p_cmv_model->myof_fil_compliance_factor;

	myof_thick_fil_length = p_cmv_model->myof_thick_fil_length;
	myof_bare_zone_length = p_cmv_model->myof_bare_zone_length;
	myof_thin_fil_length = p_cmv_model->myof_thin_fil_length;

	myof_a_k_on = p_cmv_model->myof_a_k_on;
	myof_a_k_off = p_cmv_model->myof_a_k_off;
	myof_a_k_coop = p_cmv_model->myof_a_k_coop;

	myof_a_off = 0.0;
	myof_a_on = 0.0;

	myof_m_bound = 0.0;
	myof_f_overlap = 1.0;

	m_pops_array = (double*)malloc(p_m_scheme->no_of_states * sizeof(double));
	m_stresses_array = (double*)malloc(p_m_scheme->no_of_states * sizeof(double));

	myof_stress_cb = 0.0;
	myof_stress_myof = 0.0;
	myof_stress_int_pas = 0.0;
	myof_stress_ext_pas = 0.0;
	myof_stress_total = 0.0;

	no_of_bin_positions = 0;
	y_length = 0;
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
	if (m_state_stresses != NULL)
	{
		gsl_vector_free(m_state_stresses);
	}

	free(m_pops_array);
	free(m_stresses_array);
}

// Other functions
void myofilaments::initialise_simulation(void)
{
	//! Function adds data fields to main results object

	// Variables

	// Code

	// Set the options
	p_cmv_options = p_parent_hs->p_cmv_options;

	// Now update the daughter kinetic_scheme
	p_m_scheme->initialise_simulation(this);

	// Now do lots of stuff specific to this class
	// Code
	no_of_bin_positions = 1 + (int)((p_cmv_options->bin_max - p_cmv_options->bin_min) /
		p_cmv_options->bin_width);

	// Assign x
	x = gsl_vector_alloc(no_of_bin_positions);

	for (int i = 0; i < no_of_bin_positions; i++)
	{
		gsl_vector_set(x, i, p_cmv_options->bin_min + ((double)i * p_cmv_options->bin_width));
	}

	// Now set the length of the system from the kinetic scheme + 2 for thin filament
	y_length = (size_t)p_m_scheme->no_of_detached_states +
		(size_t)(p_m_scheme->no_of_attached_states * no_of_bin_positions) +
		2;

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
	gsl_vector_set_zero(m_state_pops);
	gsl_vector_set(m_state_pops, 0, 1.0);

	// And the state forces
	m_state_stresses = gsl_vector_alloc(p_m_scheme->no_of_states);
	gsl_vector_set_zero(m_state_stresses);

	// Update class variables
	myof_a_off = gsl_vector_get(y, a_off_index);
	myof_a_on = gsl_vector_get(y, a_on_index);

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
		p_cmv_results->add_results_field(label, &m_pops_array[i]);
	}

	for (int i = 0; i < p_m_scheme->no_of_states; i++)
	{
		string label = string("myof_m_stress_") + to_string(i);
		p_cmv_results->add_results_field(label, &m_stresses_array[i]);
	}

	p_cmv_results->add_results_field("myof_stress_cb", &myof_stress_cb);
	p_cmv_results->add_results_field("myof_stress_int_pas", &myof_stress_int_pas);
	p_cmv_results->add_results_field("myof_stress_ext_pas", &myof_stress_ext_pas);
	p_cmv_results->add_results_field("myof_stress_myof", &myof_stress_myof);
	p_cmv_results->add_results_field("myof_stress_total", &myof_stress_total);
}

// This function is not a member of the myofilaments class but is used to interace
// with the GSL ODE system. It must appear before the myofilaments class members
// that calls it, and communicates with the myofilament class through a pointer to
// the class object

int myof_calculate_derivs(double t, const double y[], double f[], void* params)
{
	// Function sets derivs

	// Variables
	(void)(t);

	myofilaments* p_myof = (myofilaments*)params;

	double rate;

	double hs_stress;
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

	// Set state variables
	hs_stress = p_myof->myof_stress_myof;
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
						calculate_rate(0, 0, hs_stress, hs_length);

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
							p_transitions[t_counter]->calculate_rate(x_pos, x_ext, hs_stress, hs_length);

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
							p_transitions[t_counter]->calculate_rate(x_pos, x_ext, hs_stress, hs_length);

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
							p_transitions[t_counter]->calculate_rate(x_pos, x_ext, hs_stress, hs_length);

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

	double holder;
	double adjustment;
	
	// Code

	// Allocate memory for y
	y_calc = (double*)malloc(y_length * sizeof(double));
	
	// Fill y_calc
	for (size_t i = 0; i < y_length; i++)
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
		//Unpack, noting how many bridges we have
		holder = 0.0;
		for (int i = 0; i < y_length; i++)
		{
			gsl_vector_set(y, i, y_calc[i]);

			if (i < (y_length - 2))
			{
				holder = holder + y_calc[i];
			}
		}

		adjustment = 1.0 - holder;
		// Add back to first DRX state
		y_calc[1] = y_calc[1] + adjustment;
		gsl_vector_set(y, 1, y_calc[1]);
		if (fabs(adjustment) > 0.005)
			cout << "fast sliding: " << adjustment << "\n";
	}

	// Update class variables

	// Populations
	calculate_m_state_pops(y_calc);
	for (int i = 0; i < p_m_scheme->no_of_states; i++)
	{
		m_pops_array[i] = gsl_vector_get(m_state_pops, i);
	}

	myof_a_off = gsl_vector_get(y, a_off_index);
	myof_a_on = gsl_vector_get(y, a_on_index);

	// Forces
	calculate_m_state_stresses();
	for (int i = 0; i < p_m_scheme->no_of_states; i++)
	{
		m_stresses_array[i] = gsl_vector_get(m_state_stresses, i);
	}

	calculate_stresses();

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
	//! 
	//!     <-- Thin--->
	//!    |------------         |
	//!    |                     |
	//!    |      --------|------|
	//!           <--Thick------>|
	//!                    <Bare>|

	// Variables
	double x_no_overlap;
	double x_overlap;
	double max_x_overlap;
	double protrusion;

	// Code

	x_no_overlap = p_parent_hs->hs_length - myof_thick_fil_length;
	x_overlap = myof_thin_fil_length - x_no_overlap;
	max_x_overlap = myof_thick_fil_length - myof_bare_zone_length;
	protrusion = myof_thin_fil_length - p_parent_hs->hs_length;

	if (x_overlap == 0.0)
		myof_f_overlap = 0.0;

	if ((x_overlap > 0.0) && (x_overlap <= max_x_overlap))
		myof_f_overlap = x_overlap / max_x_overlap;

	if (x_overlap > max_x_overlap)
		myof_f_overlap = 1.0;

	if (protrusion > 0.0)
	{
		x_overlap = max_x_overlap - protrusion;
		myof_f_overlap = x_overlap / max_x_overlap;
	}
}

void myofilaments::calculate_m_state_stresses(void)
{
	//| Function calculates cross-bridge stresses

	// Variables

	int bin_index;

	double x_bin;
	double x_ext;
	double bin_pop;

	double holder;

	// Code

	// Cycle through through the states
	for (int state_counter = 0; state_counter < p_m_scheme->no_of_states;
		state_counter++)
	{
		char state_type = p_m_scheme->p_m_states[state_counter]->state_type;

		holder = 0.0;

		if (state_type == 'A')
		{
			x_ext = p_m_scheme->p_m_states[state_counter]->extension;

			// Set the bin index
			bin_index = gsl_matrix_int_get(m_y_indices, state_counter, 0);

			for (int i = 0; i < no_of_bin_positions; i++)
			{
				// Get bin position
				x_bin = gsl_vector_get(x, i);

				/// Get population
				bin_pop = gsl_vector_get(y, bin_index);

				// Add force to holder
				holder = holder + (bin_pop * (x_bin + x_ext));

				bin_index = bin_index + 1;
			}
		}

		// Adjust for fibrosis, myofilament area, and units
		holder = (1.0 - myof_prop_fibrosis) * myof_prop_myofilaments *
			myof_cb_number_density * 1e-9 * myof_k_cb * holder;

		gsl_vector_set(m_state_stresses, state_counter, holder);
	}
}

void myofilaments::calculate_stresses(bool check_only)
{
	//! Code calculates forces

	// Code
	calculate_cb_stress();
	calculate_int_pas_stress();
	calculate_ext_pas_stress();

	myof_stress_myof = myof_stress_cb + myof_stress_int_pas;
	myof_stress_total = myof_stress_myof + myof_stress_ext_pas;
}

double myofilaments::calculate_cb_stress(bool check_only)
{
	//! Code calculates cb force

	// Variables
	double holder = 0.0;

	for (int i = 0; i < p_m_scheme->no_of_states; i++)
	{
		holder = holder + gsl_vector_get(m_state_stresses, i);
	}

	if (check_only == false)
		myof_stress_cb = holder;

	return holder;
}

double myofilaments::calculate_int_pas_stress(bool check_only, double delta_hsl)
{
	//! Code calculates internal passive force for a given delta_hsl
	//! If check_only is true, does not set class variable

	// Variables

	double x;			// length relative to slack in nm

	double pas_stress;	// intracellular passive stress

	// Code
	
	x = (p_parent_hs->hs_length + delta_hsl) - myof_int_pas_slack_hsl;

	if (x > 0.0)
	{
		pas_stress = myof_int_pas_sigma * (exp(x / myof_int_pas_L) - 1.0);
	}
	else
	{
		pas_stress = -myof_int_pas_sigma * (exp(fabs(x) / myof_int_pas_L) - 1.0);
	}

	if (check_only == false)
		myof_stress_int_pas = pas_stress;

	return pas_stress;
}

double myofilaments::calculate_ext_pas_stress(bool check_only, double delta_hsl)
{
	//! Code calculates external passive force for a given delta_hsl
	//! If check_only is true, does not set class variable

	// Variables

	double x;			// length relative to slack in nm

	double pas_stress;	// intracellular passive stress

	// Code

	x = (p_parent_hs->hs_length + delta_hsl) - myof_ext_pas_slack_hsl;

	if (x > 0.0)
	{
		pas_stress = myof_ext_pas_sigma * (exp(x / myof_ext_pas_L) - 1.0);
	}
	else
	{
		pas_stress = -myof_ext_pas_sigma * (exp(fabs(x) / myof_ext_pas_L) - 1.0);
	}

	if (check_only == false)
		myof_stress_ext_pas = pas_stress;

	return pas_stress;
}

double myofilaments::return_stress_after_delta_hsl(double delta_hsl)
{
	//! Function returns stress after given delta_hsl

	// Variables
	double delta_int_pas_stress;
	double delta_ext_pas_stress;
	double delta_cb_stress;
	double new_stress;

	// Code
	delta_int_pas_stress = calculate_int_pas_stress(true, delta_hsl) -
		myof_stress_int_pas;

	delta_ext_pas_stress = calculate_ext_pas_stress(true, delta_hsl) -
		myof_stress_ext_pas;

	delta_cb_stress = (1.0 - myof_prop_fibrosis) * myof_prop_myofilaments *
		myof_cb_number_density * 1e-9 * myof_k_cb * myof_m_bound *
		myof_fil_compliance_factor * delta_hsl;

	new_stress = myof_stress_total +
		delta_int_pas_stress +
		delta_ext_pas_stress +
		delta_cb_stress;

	return new_stress;
}

void myofilaments::move_cb_populations(double delta_hsl)
{
	//! Code displaces cross-bridge populations

	// Variables
	bool x_defined = false;

	double* x_calc = NULL;
	double* y_calc = NULL;
	
	double x_shift;
	double y_temp;

	gsl_interp_accel* acc = NULL;
	gsl_spline* spline = NULL;

	// Code

	// Skip out if delta_hsl == 0
	if (delta_hsl == 0.0)
	{
		return;
	}

	// Allocate memory for y_calc
	x_calc = (double*)malloc(no_of_bin_positions * sizeof(double));
	y_calc = (double*)malloc(no_of_bin_positions * sizeof(double));

	
	// Cycle through states
	for (size_t state_counter = 0; state_counter < (size_t)(p_m_scheme->no_of_states) ; state_counter++)
	{
		if (p_m_scheme->p_m_states[state_counter]->state_type == 'A')
		{
			// We need to move populations

			// Set x_calc
			if (x_defined == false)
			{
				// First time
				// Set x
				for (size_t ind = 0; ind < no_of_bin_positions; ind++)
				{
					x_calc[ind] = gsl_vector_get(x, ind);
				}

				// Initialise for interpolation
				acc = gsl_interp_accel_alloc();
				spline = gsl_spline_alloc(gsl_interp_cspline, no_of_bin_positions);

				// Work out the shift
				x_shift = myof_fil_compliance_factor * delta_hsl;

				// Note we are set up
				x_defined = true;
			}

			for (size_t ind = 0 ; ind < no_of_bin_positions ; ind++)
			{
				y_calc[ind] = gsl_vector_get(y, ind + gsl_matrix_int_get(m_y_indices, state_counter, 0));
			}

			gsl_spline_init(spline, x_calc, y_calc, no_of_bin_positions);

			// Calculate at the new positions
			for (size_t ind = 0; ind < no_of_bin_positions; ind++)
			{
				double new_pos = x_calc[ind] - x_shift;

				if ((new_pos < p_cmv_options->bin_min) || (new_pos > p_cmv_options->bin_max))
					y_temp = 0.0;
				else
				{
					y_temp = gsl_spline_eval(spline, new_pos, acc);
				}

				// Assign
				gsl_vector_set(y, gsl_matrix_int_get(m_y_indices, state_counter, 0) + ind,
					y_temp);
			}
		}
	}

	// Tidy up
	gsl_spline_free(spline);
	gsl_interp_accel_free(acc);

	free(x_calc);
	free(y_calc);
}
