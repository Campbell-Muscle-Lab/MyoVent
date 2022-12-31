/**
/* @file		myofilaments.cpp
/* @brief		Source file for a myofilaments object
/* @author		Ken Campbell
*/

#include <iostream>

#include "myofilaments.h"
#include "half_sarcomere.h"
#include "kinetic_scheme.h"
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
	k_matrix = NULL;

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
	if (k_matrix != NULL)
	{
		gsl_matrix_free(k_matrix);
	}
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

	// Initialise the k_matrix
	k_matrix = gsl_matrix_alloc(y_length, y_length);

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

	double f_overlap = p_myof->myof_f_overlap;
	double m_bound = p_myof->myof_m_bound;

	double J_on;
	double J_off;
	
	// Code
	
	// Calculate f as k_matrix * y for myosin
	for (int r = 0; r < p_myof->m_length; r++)
	{
		f[r] = 0.0;

		for (int c = 0; c < p_myof->m_length; c++)
		{
			f[r] = f[r] + (gsl_matrix_get(p_myof->k_matrix, r, c) * y[r]);
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
	set_k_matrix();

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
	myof_a_off = gsl_vector_get(y, a_off_index);
	myof_a_on = gsl_vector_get(y, a_on_index);

	// Tidy up
	free(y_calc);
}


void myofilaments::set_k_matrix(void)
{
	//! Function updates the k_matrix

	double J_on;
	double J_off;

	// Code

	// Start from scratch
	gsl_matrix_set_zero(k_matrix);
}
