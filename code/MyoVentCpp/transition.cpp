/**
* @file		transition.cpp
* @brief	Source file for the transition class
* @author	Ken Campbell
*/

#include <cstdio>
#include <math.h>

#include "cmv_model.h"
#include "cmv_options.h"
#include "transition.h"
#include "m_state.h"
#include "kinetic_scheme.h"
#include "half_sarcomere.h"
#include "global_definitions.h"
#include "JSON_functions.h"

#include "rapidjson\document.h"

#include "gsl_vector.h"
#include "gsl_math.h"
#include "gsl_const_mksa.h"


// Constructor
transition::transition(const rapidjson::Value& tr, m_state* set_p_parent_m_state)
{
	// Set p_parent_m_state
	p_parent_m_state = set_p_parent_m_state;

	// Set transition_type to unknown - will be set later on
	transition_type = 'x';

	JSON_functions::check_JSON_member_int(tr, "new_state");
	new_state = tr["new_state"].GetInt();

	JSON_functions::check_JSON_member_string(tr, "rate_type");
	sprintf_s(rate_type, _MAX_PATH, tr["rate_type"].GetString());

	// Read in parameters
	JSON_functions::check_JSON_member_array(tr, "rate_parameters");
	const rapidjson::Value& rp = tr["rate_parameters"];

	rate_parameters = gsl_vector_alloc(MAX_NO_OF_RATE_PARAMETERS);
	gsl_vector_set_all(rate_parameters, GSL_NAN);

	for (int i = 0; i < (int)rp.Size(); i++)
	{
		gsl_vector_set(rate_parameters, i, rp[i].GetDouble());
	}
}

transition::transition()
{
	// Default constructor - used if there is no defined transition
	p_parent_m_state = NULL;
	new_state = 0;
	transition_type = 'x';
	sprintf_s(rate_type, _MAX_PATH, "");
	rate_parameters = gsl_vector_alloc(MAX_NO_OF_RATE_PARAMETERS);
	gsl_vector_set_all(rate_parameters, GSL_NAN);
}

// Destructor
transition::~transition(void)
{
	// Tidy up
	gsl_vector_free(rate_parameters);
}

// Functions

double transition::calculate_rate(double x, double x_ext, double force)
{
	return 0.0;
}

/*
double transition::calculate_rate(double x, double x_ext, double force,
	int mybpc_state, int mybpc_iso, short int active_neigh, half_sarcomere* p_hs)
{
	//! Returns the rate for a transition with a given x
	//! 
	// printf("%s \n ", rate_type);

	// Variables
	double rate = 0.0;

	cmv_options* p_options;

	// Code

	// Set options
	p_options = p_parent_m_state->p_parent_scheme->p_cmv_options;

	// Constant
	if (!strcmp(rate_type, "constant"))
	{
		rate = gsl_vector_get(rate_parameters, 0);
	}

	// Force-dependent
	if (!strcmp(rate_type, "force_dependent"))
	{
		rate = gsl_vector_get(rate_parameters, 0) *
			(1.0 + (gsl_max(node_force, 0.0) * gsl_vector_get(rate_parameters, 1)));
	}

	// Gaussian
	if (!strcmp(rate_type, "gaussian"))
	{
		p_model = p_parent_m_state->p_parent_scheme->p_fs_model;
		double k_cb = p_model->m_k_cb;

		rate = gsl_vector_get(rate_parameters, 0) *
			exp(-(0.5 * k_cb * gsl_pow_int(x, 2)) /
				(1e18 * GSL_CONST_MKSA_BOLTZMANN * p_model->temperature));
	}

	// Gaussian_hsl
	if (!strcmp(rate_type, "gaussian_hsl"))
	{
		// Distance between surface of thick and thin filaments is
		// (2/3)*d_1,0 - r_thin - r_thick
		// Assume d_1_0 at hsl = 1100 nm is 37 nm, r_thin = 5.5 nm, t_thick = 7.5 nm
		// d at hsl = x is (2/3) * (37 / sqrt(x/1100)) - 5/5 - 7.5
		// first passage time to position y is t = y^2 / (2*D)
		// rate is proportional to 1/t
		// rate at hsl == x is ref_rate * (y_ref / y_x)^2
		// See PMID 35450825 and first passage in
		// Mechanics of motor proteins and the cytoskeleton, Joe Howard book

		FiberSim_model* p_model = p_parent_m_state->p_parent_scheme->p_fs_model;
		double k_cb = p_model->m_k_cb;

		double hs_length;
		double y_ref;		// distance between filaments at 1100 nm
		double y_actual;	// distance between filaments at current hsl
		double r_thick = 7.5;
		double r_thin = 5.5;

		y_ref = ((2.0 / 3.0) * 37.0) - r_thick - r_thin;

		if (p_hs == NULL)
			hs_length = 1100.0;
		else
			hs_length = p_hs->hs_length;

		y_actual = (2.0 / 3.0) * (37.0 / sqrt(hs_length / 1100.0)) - r_thick - r_thin;

		rate = gsl_vector_get(rate_parameters, 0) *
			exp(-(0.5 * k_cb * gsl_pow_int(x, 2)) /
				(1e18 * GSL_CONST_MKSA_BOLTZMANN * p_model->temperature));

		rate = rate * gsl_pow_2(y_ref / y_actual);
	}

	// Gaussian MyBP-C 

	if (!strcmp(rate_type, "gaussian_pc"))
	{
		FiberSim_model* p_model = p_parent_m_state->p_parent_scheme->p_fs_model;
		double k_pc = p_model->c_k_stiff; // use MyBPC stiffness as default

		double temp = gsl_vector_get(rate_parameters, 1); // optional parameter which sets mybpc stiffness

		if (!gsl_isnan(temp)) { // optional parameter is not specified, use the state extension instead
			k_pc = temp;
		}

		rate = gsl_vector_get(rate_parameters, 0) *
			exp(-(0.5 * k_pc * gsl_pow_int(x, 2)) /
					(1e18 * GSL_CONST_MKSA_BOLTZMANN * p_model->temperature));
	}

	// Poly
	if (!strcmp(rate_type, "poly"))
	{
		double x_center = gsl_vector_get(rate_parameters, 3); // optional parameter defining the zero of the polynomial

		if (gsl_isnan(x_center)) { // optional parameter is not specified, use the state extension instead
			x_center = x_ext;
		}	

		rate = gsl_vector_get(rate_parameters, 0) +
				(gsl_vector_get(rate_parameters, 1) *
					gsl_pow_int(x + x_center, (int)gsl_vector_get(rate_parameters, 2)));

	}

	// Poly_asymmetric
	if (!strcmp(rate_type, "poly_asym"))
	{
		double x_center = gsl_vector_get(rate_parameters, 5); // optional parameter defining the zero of the polynomial

		if (gsl_isnan(x_center)) { // optional parameter is not specified, use the state extension instead
			x_center = x_ext;
		}

		if (x > x_center)
			rate = gsl_vector_get(rate_parameters, 0) +
				(gsl_vector_get(rate_parameters, 1) *
					gsl_pow_int(x + x_center, (int)gsl_vector_get(rate_parameters, 3)));
		else
			rate = gsl_vector_get(rate_parameters, 0) +
			(gsl_vector_get(rate_parameters, 2) *
				gsl_pow_int(x + x_center, (int)gsl_vector_get(rate_parameters, 4)));
	}

	// Decreasing load-dependent exponential
	if (!strcmp(rate_type, "exp"))
	{
		double A = gsl_vector_get(rate_parameters, 0);
		double B = gsl_vector_get(rate_parameters, 1);
		double C = gsl_vector_get(rate_parameters, 2);
		double x_center = gsl_vector_get(rate_parameters, 3);
		double x_wall = gsl_vector_get(rate_parameters, 4);

		if (x < x_wall)
			rate = A + B * exp(-C * (x + x_center));
		else
			rate = p_options->max_rate;
	}

	if (!strcmp(rate_type, "exp_wall"))
	{
		// Variables

		FiberSim_model* p_model = p_parent_m_state->p_parent_scheme->p_fs_model;
		double k0 = gsl_vector_get(rate_parameters, 0);
		double F = p_model->m_k_cb * (x + x_ext);
		double d = gsl_vector_get(rate_parameters, 1);
		double x_wall = gsl_vector_get(rate_parameters, 2);
		double x_smooth = gsl_vector_get(rate_parameters, 3);

		// Code
		rate = k0 * exp(-(F * d) /
				(1e18 * GSL_CONST_MKSA_BOLTZMANN * p_model->temperature));

		rate = rate + p_options->max_rate * (1 /
			(1 + exp(-x_smooth * (x - x_wall))));
	}

	if (!strcmp(rate_type, "exp_wall_sweep"))
	{
		// Variables

		FiberSim_model* p_model = p_parent_m_state->p_parent_scheme->p_fs_model;
		double k0 = gsl_vector_get(rate_parameters, 0);
		double F = p_model->m_k_cb * (x + x_ext);
		double d = gsl_vector_get(rate_parameters, 1);
		double x_wall = gsl_vector_get(rate_parameters, 2);
		double x_smooth = gsl_vector_get(rate_parameters, 3);
		double sweep = gsl_vector_get(rate_parameters, 4);

		// Code
		rate = k0 * exp(-(F * d) /
			(1e18 * GSL_CONST_MKSA_BOLTZMANN * p_model->temperature));

		rate = rate + p_options->max_rate * (1 /
			(1 + exp(-x_smooth * (x - x_wall))));

		// Add in the sweep, whereby neighboring units in the off state
		// increase the detachment rate
		rate = rate + sweep * (2.0 - (double)active_neigh);

		//printf("active_neigh: %g  rate: %f\n", (double)active_neigh, rate);
	}

	// Curtail at max rate

	if (rate > (p_options->max_rate))
		rate = p_options->max_rate;

	if (rate < 0.0)
		rate = 0.0;
	
	// Return
	return rate;
}
*/
