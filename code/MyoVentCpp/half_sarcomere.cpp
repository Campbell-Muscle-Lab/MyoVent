/**
/* @file		half_sarcomere.cpp
/* @brief		Source file for a half_sarcomere object
/* @author		Ken Campbell
*/

#include "stdio.h"

#include "cmv_model.h"
#include "cmv_options.h"
#include "cmv_results.h"

#include "FiberSim_model.h"

#include "half_sarcomere.h"
#include "MyoSim_half_sarcomere.h"
#include "FiberSim_half_sarcomere.h"
#include "hemi_vent.h"
#include "membranes.h"
#include "mitochondria.h"
#include "myofilaments.h"
#include "heart_rate.h"

#include "gsl_errno.h"
#include "gsl_math.h"
#include "gsl_roots.h"
#include "gsl_const_num.h"

struct stats_structure {
	double mean_value;
	double min_value;
	double max_value;
	double sum;
};

// Constructor
half_sarcomere::half_sarcomere(hemi_vent* set_p_parent_hemi_vent)
{
	//! Constructor

	// Code
	printf("half_sarcomere constructor()\n");

	// Set the pointers to the parent system
	p_parent_hemi_vent = set_p_parent_hemi_vent;
	p_cmv_model = p_parent_hemi_vent->p_cmv_model;
	p_cmv_system = p_parent_hemi_vent->p_parent_cmv_system;
	p_cmv_options = p_parent_hemi_vent->p_cmv_options;

	// Create the daugher objects
	p_heart_rate = new heart_rate(this);
	p_membranes = new membranes(this);
	p_mitochondria = new mitochondria(this);

	// Create the correct half-sarcomere model
	if (p_cmv_model->p_fs_model == NULL)
	{
		p_MyoSim_hs = new MyoSim_half_sarcomere(this);
		//p_myofilaments = new myofilaments(this);
		p_FiberSim_hs = NULL;
	}
	else
	{
		p_MyoSim_hs = NULL;
		p_FiberSim_hs = new FiberSim_half_sarcomere(this, 0);
	}

	// Set safe values
	p_cmv_results_beat = NULL;

	// Initialise
	hs_stress = 0.0;
	hs_ATP_used_per_liter_per_s = 0.0;
	hs_length = p_cmv_model->hs_reference_hs_length;
	hs_reference_hs_length = p_cmv_model->hs_reference_hs_length;
	hs_delta_G_ATP = p_cmv_model->hs_delta_G_ATP;
	hs_ATP_concentration = p_cmv_model->hs_initial_ATP_concentration;
	hs_prop_fibrosis = p_cmv_model->hs_prop_fibrosis;
	hs_prop_myofilaments = p_cmv_model->hs_prop_myofilaments;
}

// Destructor
half_sarcomere::~half_sarcomere(void)
{
	//! Destructor

	// Code

	// Tidy up
	delete p_heart_rate;
	delete p_membranes;
	delete p_mitochondria;
	delete p_myofilaments;
	delete p_FiberSim_hs;
	delete p_MyoSim_hs;
}

// Other functions
void half_sarcomere::initialise_simulation(void)
{
	//! Code initialises simulation
	
	// Variables
	double slack_hs_length;
	
	// Code

	// Set results from parent
	p_cmv_results_beat = p_parent_hemi_vent->p_cmv_results_beat;

	// Now add the results fields
	p_cmv_results_beat->add_results_field("hs_length", &hs_length);
	p_cmv_results_beat->add_results_field("hs_stress", &hs_stress);
	p_cmv_results_beat->add_results_field("hs_ATP_used_per_liter_per_s", &hs_ATP_used_per_liter_per_s);
	p_cmv_results_beat->add_results_field("hs_ATP_concentration", &hs_ATP_concentration);

	// Now initialise daughter objects
	p_heart_rate->initialise_simulation();
	p_membranes->initialise_simulation();
	p_mitochondria->initialise_simulation();
	
	if (p_FiberSim_hs != NULL)
	{
		p_FiberSim_hs->initialise_for_MyoVent_simulation();

		// Adjust to slack length
		slack_hs_length = p_FiberSim_hs->return_hs_length_for_force(0.0, 0.0);
		p_FiberSim_hs->update_lattice(0.0, (slack_hs_length - p_FiberSim_hs->hs_length));

		// Now calculate forces
		p_FiberSim_hs->calculate_force(0.0, 0.0);

		hs_stress = p_FiberSim_hs->hs_force;
	}
	else
	{
		p_myofilaments->initialise_simulation();

		// Set the hs_length so that wall stress is zero
		slack_hs_length = return_hs_length_for_stress(0.0);

		change_hs_length(slack_hs_length - hs_length);

		// Now calculate the wall stress
		p_myofilaments->calculate_stresses();

		hs_stress = p_myofilaments->myof_stress_total;

	}
}

bool half_sarcomere::implement_time_step(double time_step_s)
{
	//! Implements time-step

	// Variables
	bool new_beat = false;

	// Code

	// Update daughter objects
	new_beat = p_heart_rate->implement_time_step(time_step_s);
	
	p_membranes->implement_time_step(time_step_s, new_beat);

	p_mitochondria->implement_time_step(time_step_s);

	if (p_FiberSim_hs != NULL)
	{
		double pCa = -log10(p_membranes->memb_Ca_cytosol);
		p_FiberSim_hs->sarcomere_kinetics(time_step_s, pCa);
	}
	else
	{
		p_myofilaments->implement_time_step(time_step_s);
	}

	// Update the ATP
	calculate_hs_ATP_concentration(time_step_s);

	// Return new beat status
	return (new_beat);
}

void half_sarcomere::change_hs_length(double delta_hsl)
{
	//! Changes half-sarcomere length by delta_hsl

	// Code

	// Adjust this half-sarcomere
	hs_length = hs_length + delta_hsl;

	if (p_FiberSim_hs != NULL)
	{
		p_FiberSim_hs->update_lattice(0.0, delta_hsl);

		hs_stress = p_FiberSim_hs->calculate_force(delta_hsl, 0.0);

	}
	else
	{
		// And the cross-bridges
		p_myofilaments->move_cb_populations(delta_hsl);

		// Now update wall stress
		p_myofilaments->calculate_stresses();

		hs_stress = p_myofilaments->myof_stress_total;
	}
}

double half_sarcomere::return_wall_stress_after_delta_hsl(double delta_hsl, double time_step_s)
{
	//! Function returns wall stress after given delta_hsl
	
	// Variables
	double wall_stress;

	// Code
	if (p_FiberSim_hs != NULL)
	{
		wall_stress = p_FiberSim_hs->return_force_for_test_delta_hsl(delta_hsl, time_step_s);
	}
	else
	{
		wall_stress = p_myofilaments->return_stress_after_delta_hsl(delta_hsl);
	}

	return wall_stress;
}

struct gsl_root_params
{
	half_sarcomere* p_hs_temp;
	double stress_target;
};

double hsl_stress_root_finder(double x, void* params)
{
	//! Function for the root finder
	
	// Variables
	double new_stress;
	double stress_difference;

	// Code
	
	// Unpack the pointer
	struct gsl_root_params* p = (struct gsl_root_params*)params;

	// Calculate the new stress after a length change
	new_stress = p->p_hs_temp->return_wall_stress_after_delta_hsl(x, 0.001);

	// Calculate the difference between the new stress and the target
	stress_difference = new_stress - p->stress_target;

	// Return the difference
	return stress_difference;
}

double half_sarcomere::return_hs_length_for_stress(double target_stress)
{
	//! Code returns the hs_length at which force is equal to the target

	// Variables
	double x_lo = -500;
	double x_hi = 500;
	double r;

	const gsl_root_fsolver_type* T;
	gsl_root_fsolver* s;

	gsl_function F;
	struct gsl_root_params params = { this, target_stress };

	int status;
	int iter = 0;
	int max_iter = 100;

	double epsabs = 0.01;
	double epsrel = 0.01;

	// Code

	F.function = &hsl_stress_root_finder;
	F.params = &params;

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
		status = gsl_root_test_interval(x_lo, x_hi, epsabs, epsrel);

	} while ((status == GSL_CONTINUE) && (iter < max_iter));

	gsl_root_fsolver_free(s);

	// Calculate the length

	return (hs_length + r);
}

void half_sarcomere::calculate_hs_ATP_concentration(double time_step_s)
{
	//! Function updates hs ATP concentration

	// Variables
	double d_heads;					// number of heads per liter
	double cb_number_density;
	double ATP_flux;

	// Code

	if (p_FiberSim_hs != NULL)
	{
		ATP_flux = p_FiberSim_hs->ATP_flux;
		cb_number_density = p_cmv_model->p_fs_model->m_filament_density *
			p_FiberSim_hs->m_cbs_per_thick_filament;
	}
	else
	{
		ATP_flux = p_myofilaments->myof_ATP_flux;
		cb_number_density = p_myofilaments->myof_cb_number_density;
	}

	d_heads = 0.001 *
		(1.0 - hs_prop_fibrosis) * hs_prop_myofilaments *
			cb_number_density *
			(1.0 / (1e-9 * hs_reference_hs_length));

	hs_ATP_used_per_liter_per_s = -d_heads * ATP_flux /
								GSL_CONST_NUM_AVOGADRO;

	// Euler step

	hs_ATP_concentration = hs_ATP_concentration +
		(time_step_s *
			(hs_ATP_used_per_liter_per_s +
				p_mitochondria->mito_ATP_generated_M_per_liter_per_s));
}

void half_sarcomere::update_beat_metrics(void)
{
	//! Update beat metrics

	// Variables
	stats_structure* p_stats;

	// Code

	p_stats = new stats_structure;

	if (p_cmv_results_beat->hs_length_field_index >= 0)
	{
		p_cmv_results_beat->calculate_sub_vector_statistics(
			p_cmv_results_beat->gsl_results_vectors[p_cmv_results_beat->hs_length_field_index],
			0, p_parent_hemi_vent->p_parent_cmv_system->beat_t_index,
			p_stats);
	}

	// Tidy up
	delete p_stats;
}

