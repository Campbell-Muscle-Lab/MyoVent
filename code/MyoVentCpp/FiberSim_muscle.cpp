/**
/* @file		FiberSim_muscle.cpp
/* @brief		Source file for a FiberSim_muscle object
/* @author		Ken Campbell
*/

#include <cstdio>

#include "cmv_options.h"
#include "cmv_results.h"

#include "muscle.h"
#include "membranes.h"

#include "FiberSim_options.h"
#include "FiberSim_muscle.h"
#include "FiberSim_half_sarcomere.h"
#include "FiberSim_series_component.h"


#include "gsl_math.h"
#include "gsl_vector.h"
#include "gsl_multiroots.h"
#include "gsl_roots.h"

// Structure used for root-finding for myofibril in length - or force control mode
struct fs_m_control_params
{
	double time_step_s;
	FiberSim_muscle* p_fs_m;
	double target_force;
};

struct fs_m_force_control_params
{
	double target_force;
	double time_step;
	FiberSim_half_sarcomere* p_fs_hs;
	double delta_hsl;
};

// This is a function used by the root finding algorithm that handles the recasting of pointers
int wrapper_length_control_myofibril_with_series_compliance(const gsl_vector* x, void* params, gsl_vector* f);

// Constructor
FiberSim_muscle::FiberSim_muscle(muscle* set_p_parent_muscle)
{
	//! Constructor
	
	// Set pointer to parent
	p_parent_muscle = set_p_parent_muscle;

	// Set options
	p_FiberSim_options = p_parent_muscle->p_cmv_options->p_FiberSim_options;

	// Make a FiberSim half-sarcomere
	p_FiberSim_hs = new FiberSim_half_sarcomere(this, 0);

	// Make a new FiberSim series component
	p_FiberSim_sc = new FiberSim_series_component(this);

	// Set the length
	fs_m_length = p_FiberSim_hs->hs_length + p_FiberSim_sc->sc_extension;

	// Impose force balance
	change_muscle_length(0.0, 0.0);
}

// Destructor
FiberSim_muscle::~FiberSim_muscle(void)
{
	//! Destructor
	
	// Code
	delete p_FiberSim_hs;
}

// Other functions

void FiberSim_muscle::initialise_for_simulation(void)
{
	//! Code sets up for a simulation
	
	// Variables

	// Update a pointer
	p_cmv_results_beat = p_parent_muscle->p_cmv_results_beat;

	// Add fields
	p_cmv_results_beat->add_results_field("fs_muscle_length", &fs_m_length);
	p_cmv_results_beat->add_results_field("fs_muscle_stress", &fs_m_stress);


	// Apply to the daughter objects
	p_FiberSim_hs->initialise_for_simulation();
	p_FiberSim_sc->initialise_for_simulation();
}

void FiberSim_muscle::implement_time_step(double time_step_s)
{
	//! Function runs sarcomere kinetics

	// Variables

	double pCa;

	// Code

	pCa = -log10(p_parent_muscle->p_membranes->memb_Ca_cytosol);

	p_FiberSim_hs->sarcomere_kinetics(time_step_s, pCa);
}

int FiberSim_muscle::change_muscle_length(double delta_ml, double time_step_s)
{
	//! Code changes the muscle length by delta_ml
	//! 
	//! Tries to find a vector x such that the force in the half-sarcomere and the
	//! force in the series elastic element (which is the length of the muscle - the
	//! length of the half-sarcomere) are all equal
	
	// Variables
	int x_length;					// The length of the x_vector
	int myofibril_iterations;
	int max_lattice_iterations;
	int status;

	double new_hs_length;
	double delta_length;

	gsl_vector* x;					// A vector

	// Code
	
	// Update the length
	fs_m_length = fs_m_length + delta_ml;

	// Allocate the vector
	x_length = 2;
	x = gsl_vector_alloc(x_length);

	// The x-vector has the length of the half-sarcomere followed by the
	// force in the half-sarcomere
	gsl_vector_set(x, 0, p_FiberSim_hs->hs_length);
	gsl_vector_set(x, 1, p_FiberSim_hs->hs_force);

	// Do the root-finding
	const gsl_multiroot_fsolver_type* T;
	gsl_multiroot_fsolver* s;
	const size_t calculation_size = x_length;

	fs_m_control_params* par = new fs_m_control_params;
	par->p_fs_m = this;
	par->time_step_s = time_step_s;

	gsl_multiroot_function f = { &wrapper_length_control_myofibril_with_series_compliance, calculation_size, par };

	T = gsl_multiroot_fsolver_hybrid;
	s = gsl_multiroot_fsolver_alloc(T, calculation_size);
	gsl_multiroot_fsolver_set(s, &f, x);

	myofibril_iterations = 0;

	do
	{
		gsl_vector* y = gsl_vector_alloc(x_length);

		status = gsl_multiroot_fsolver_iterate(s);

		myofibril_iterations++;

		if (status)
		{
			printf("Myofibril multiroot solver break - Status: %i\t", status);

			if (status == GSL_EBADFUNC)
			{
				printf("Bad function value\n");
			}

			if (status == GSL_ENOPROG)
			{
				printf("Not making progress\n");
			}

			if (status == GSL_ENOPROGJ)
			{
				printf("Jacobian evaluations are not helping\n");
			}
		}

		//status = gsl_multiroot_test_residual(s->f, p_FiberSim_options->myofibril_force_tolerance);
		status = gsl_multiroot_test_delta(s->dx, s->x, p_FiberSim_options->myofibril_force_tolerance, 0.0);

		gsl_vector_free(y);
	} while ((status == GSL_CONTINUE) && (myofibril_iterations < p_FiberSim_options->myofibril_max_iterations));

	// At this point, the s->x vector contains the lengths of the n half-sarcomeres
	// followed by the force in the series element

	// Implement the change
	
	// First the half-sarcomere
	new_hs_length = gsl_vector_get(s->x, 0);
	delta_length = new_hs_length - p_FiberSim_hs->hs_length;

	max_lattice_iterations = p_FiberSim_hs->update_lattice(time_step_s, delta_length);

	p_FiberSim_sc->sc_extension = (fs_m_length - new_hs_length);
	p_FiberSim_sc->sc_force = p_FiberSim_sc->return_series_force(p_FiberSim_sc->sc_extension);

	// Update muscle force
	fs_m_stress = p_FiberSim_sc->sc_force;

/*
	printf("hsl: %g\t\tscl: %g\t\ttotal_l: %g\n",
		p_FiberSim_hs->hs_length, p_FiberSim_sc->sc_extension, fs_m_length);
	printf("hs_force: %g\t\tsc_f: %g\t\ttotal_f: %g\n\n",
		p_FiberSim_hs->hs_force, p_FiberSim_sc->sc_force, fs_m_stress);
*/

	// Tidy up
	gsl_multiroot_fsolver_free(s);
	gsl_vector_free(x);

	delete par;

	// Return the max number of lattice iterations
	return max_lattice_iterations;
}

int wrapper_length_control_myofibril_with_series_compliance(const gsl_vector* x, void* p, gsl_vector* f)
{
	//! This is a wrapper around muscle::check_residuals_for_myofibril_length_control()
	//! that handles the re-casting of pointers

	// Variables
	int f_return_value;

	struct fs_m_control_params* params =
		(struct fs_m_control_params*)p;

	// Code

	FiberSim_muscle* p_fs_m = params->p_fs_m;

	f_return_value = (int)p_fs_m->worker_length_control_myofibril_with_series_compliance(x, params, f);

	return f_return_value;
}

size_t FiberSim_muscle::worker_length_control_myofibril_with_series_compliance(
	const gsl_vector* x, void* p, gsl_vector* f)
{
	//! This code calculates the f_vector that is minimized towards 0 to
	//! ensure that the force in the myofibril and its length are constrained

	// Variables
	struct fs_m_control_params* params =
		(struct fs_m_control_params*)p;

	double delta_hsl;
	double cum_hs_length;
	double test_se_length;
	double force_diff;

	// Code

	// The f-vector is the difference between the force in each half-sarcomere and
	// the force in the series component
	// The x vector has a series of lengths followed by a muscle force
	// Calculate the force in each half-sarcomere and compare it to the force
	// Store up the half-sarcomere lengths as you go, and use that to calculate the length
	// of the series component

	// We need the force-control params for the calculation

	// Serial operation

	// Set the force control parameters
	fs_m_force_control_params* fp = new fs_m_force_control_params;
	fp->target_force = gsl_vector_get(x, x->size - 1);
	fp->time_step = params->time_step_s;
	fp->p_fs_hs = p_FiberSim_hs;

	// Get the length-change for the half-sacomere
	delta_hsl = gsl_vector_get(x, 0) - p_FiberSim_hs->hs_length;
	
	cum_hs_length = gsl_vector_get(x, 0);

	// Constrain delta_hsl to a plausible range
	if (delta_hsl > p_FiberSim_options->myofibril_max_delta_hs_length)
		delta_hsl = p_FiberSim_options->myofibril_max_delta_hs_length;
	if (delta_hsl < -p_FiberSim_options->myofibril_max_delta_hs_length)
		delta_hsl = -p_FiberSim_options->myofibril_max_delta_hs_length;

	force_diff = p_FiberSim_hs->test_force_wrapper(delta_hsl, fp);

	gsl_vector_set(f, 0, force_diff);

	// Now deduce the series elastic force
	test_se_length = fs_m_length - cum_hs_length;
	force_diff = p_FiberSim_sc->return_series_force(test_se_length) - gsl_vector_get(x, x->size - 1);

	gsl_vector_set(f, f->size - 1, force_diff);

	// Tidy up
	delete fp;

	return GSL_SUCCESS;
}

double FiberSim_muscle::return_muscle_length_for_force(double target_force, double time_step_s)
{
	//! Returns the muscle length for force
	
	// Variables
	double delta_ml;

	// Code

	delta_ml = calculate_delta_ml_for_force(target_force, time_step_s);

	// Return
	return (fs_m_length + delta_ml);
}

double test_force_wrapper(double delta_ml, void* params)
{
	//! Code used by root finding for force balance

	// Variables
	struct fs_m_control_params* p =
		(struct fs_m_control_params*)params;
	FiberSim_muscle* p_fs_m = p->p_fs_m;

	double test_value;

	// Code
	test_value = p_fs_m->return_wall_stress_after_test_delta_ml(delta_ml, p->time_step_s);

	if (!gsl_finite(test_value))
	{
		if (delta_ml > 0)
			test_value = DBL_MAX;
		else
			test_value = -DBL_MAX;
	}

	return test_value;
}

double FiberSim_muscle::calculate_delta_ml_for_force(double target_force, double time_step_s)
{
	//! Returns the delta_ml required for the muscle to generate the target force
	
	// Variables
	int status;
	int iter = 0, max_iter = 100;
	const gsl_root_fsolver_type* T;
	gsl_root_fsolver* s;

	double r = 0.0;
	double x_lo = GSL_MAX(-(fs_m_length - 10.0), -p_FiberSim_options->hs_force_control_max_delta_hs_length);
	double x_hi = p_FiberSim_options->hs_force_control_max_delta_hs_length;
	struct fs_m_control_params params = { time_step_s, this, target_force};

	gsl_function F;
	F.function = &test_force_wrapper;
	F.params = &params;

	// Test

	double test_value;
	test_value = test_force_wrapper(x_lo, &params);
	printf("x_lo: %g\t\ttest_value: %g\n", x_lo, test_value);

	test_value = test_force_wrapper(x_hi, &params);
	printf("x_hi: %g\t\ttest_value: %g\n", x_hi, test_value);

	// Code
	T = gsl_root_fsolver_brent;
	s = gsl_root_fsolver_alloc(T);
	gsl_root_fsolver_set(s, &F, x_lo, x_hi);

	do
	{
		iter++;
		status = gsl_root_fsolver_iterate(s);
		r = gsl_root_fsolver_root(s);
		x_lo = gsl_root_fsolver_x_lower(s);
		x_hi = gsl_root_fsolver_x_upper(s);
		status = gsl_root_test_interval(x_lo, x_hi, 0.01, 0);
	} while (status == GSL_CONTINUE && iter < max_iter);

	gsl_root_fsolver_free(s);

	return r;
}

double FiberSim_muscle::return_wall_stress_after_test_delta_ml(double delta_ml, double time_step_s)
{
	//! Function returns force after a test length change
	
	// Variables
	double test_force;

	// Code

	// Apply the change
	change_muscle_length(delta_ml, time_step_s);

	// Note the force
	test_force = fs_m_stress;

	// Change back
	change_muscle_length(-delta_ml, time_step_s);

	// Return
	return test_force;
}

