/**
/* @file		hemi_vent.cpp
/* @brief		Source file for a hemi_vent object
/* @author		Ken Campbell
*/

#include "stdio.h"
#include "math.h"

#include "hemi_vent.h"
#include "cmv_system.h"
#include "cmv_model.h"
#include "circulation.h"
#include "valve.h"
#include "half_sarcomere.h"
#include "myofilaments.h"
#include "cmv_results.h"
#include "cmv_options.h"

#include "gsl_math.h"
#include "gsl_const_mksa.h"
#include "gsl_const_num.h"

struct stats_structure {
	double mean_value;
	double min_value;
	double max_value;
};

// Constructor
hemi_vent::hemi_vent(circulation* set_p_parent_circulation)
{
	// Initialise

	// Code
	std::cout << "hemi_vent_constructor()\n";

	// Set pointers
	p_parent_circulation = set_p_parent_circulation;
	p_cmv_model = p_parent_circulation->p_cmv_model;
	p_parent_cmv_system = p_parent_circulation->p_parent_cmv_system;

	// Initialise with safe options
	p_cmv_results = NULL;
	p_cmv_options = NULL;

	vent_wall_density = p_cmv_model->vent_wall_density;
	vent_wall_volume = p_cmv_model->vent_wall_volume;

	vent_stroke_work_J = GSL_NAN;
	vent_stroke_energy_used_J = GSL_NAN;
	vent_efficiency = GSL_NAN;
	vent_ejection_fraction = GSL_NAN;
	vent_ATP_used_per_s = 0.0;
	vent_cardiac_output = GSL_NAN;
	vent_stroke_volume = GSL_NAN;
	vent_cardiac_output = GSL_NAN;

	// Initialise child half-sarcomere
	p_hs = new half_sarcomere(this);

	// Initialise aortic valve
	p_av = new valve(this, p_cmv_model->p_av);

	// Initialise mitral valve
	p_mv = new valve(this, p_cmv_model->p_mv);
}

// Destructor
hemi_vent::~hemi_vent(void)
{
	//! hemi_vent destructor

	// Tidy up
	delete p_hs;
	delete p_av;
	delete p_mv;
}

// Other functions
void hemi_vent::initialise_simulation(void)
{
	//! Code initialises simulation
	
	// Variables
	
	// Initialise options
	p_cmv_options = p_parent_circulation->p_cmv_options;

	if (p_cmv_options->hv_thick_wall_approximation == "True")
		vent_thick_wall_multiplier = 1.0;
	else
		vent_thick_wall_multiplier = 0.0;

	// Now add in the results
	p_cmv_results = p_parent_circulation->p_cmv_results;

	// And now daughter objects

	p_av->initialise_simulation();
	p_mv->initialise_simulation();

	p_hs->initialise_simulation();

	// Deduce the slack circumference of the ventricle and
	// set the number of half-sarcomeres
	// The p_hs->initialisation set hs_length so that stress was 0

	vent_circumference = return_lv_circumference_for_chamber_volume(
		p_parent_circulation->circ_slack_volume[0]);

	vent_n_hs = 1e9 * vent_circumference / p_hs->hs_length;

	// Add fields
	p_cmv_results->add_results_field("vent_stroke_work_J", &vent_stroke_work_J);
	p_cmv_results->add_results_field("vent_stroke_energy_used_J", &vent_stroke_energy_used_J);
	p_cmv_results->add_results_field("vent_efficiency", &vent_efficiency);
	p_cmv_results->add_results_field("vent_ejection_fraction", &vent_ejection_fraction);
	p_cmv_results->add_results_field("vent_ATP_used_per_s", &vent_ATP_used_per_s);
	p_cmv_results->add_results_field("vent_stroke_volume", &vent_stroke_volume);
	p_cmv_results->add_results_field("vent_cardiac_output", &vent_cardiac_output);
}

bool hemi_vent::implement_time_step(double time_step_s)
{
	//! Implements time-step
	
	// Variables
	bool new_beat;

	// Code

	p_av->implement_time_step(time_step_s);
	p_mv->implement_time_step(time_step_s);

	new_beat = p_hs->implement_time_step(time_step_s);

	// Calculate energy used per s
	calculate_vent_ATP_used_per_s();

	// Return
	return (new_beat);
}

double hemi_vent::return_wall_thickness_for_chamber_volume(double cv)
{
	//! Code sets object value of wall thickness
	//! Volumes are in liters, dimensions are in m

	// Variables
	double internal_r;
	double thickness;

	// Code
	internal_r = return_internal_radius_for_chamber_volume(cv);

	thickness = pow((0.001 * (cv + vent_wall_volume)) / ((2.0 / 3.0) * M_PI), (1.0 / 3.0)) -
		internal_r;

	return thickness;
}

double hemi_vent::return_lv_circumference_for_chamber_volume(double cv)
{
	//! Code returns lv circumference for chamber volume
	//! Volumes are in liters, dimensions are in m
	//! based on 2 * pi * (internal_r + wall_thickness)

	// Variables
	double thickness;
	double lv_circum;

	// Code
	thickness = return_wall_thickness_for_chamber_volume(cv);

	lv_circum = 2.0 * M_PI *
		(return_internal_radius_for_chamber_volume(cv) + (0.5 * thickness));

	return lv_circum;
}

double hemi_vent::return_internal_radius_for_chamber_volume(double cv)
{
	//! Returns internal radius in meters for a given chamber volume in liters

	// Variables
	double r;

	// Code

	if (cv < 0.0)
		cv = 0.0;

	r = pow((3.0 * 0.001 * cv) / (2.0 * M_PI), (1.0 / 3.0));

	return r;
}

double hemi_vent::return_pressure_for_chamber_volume(double cv)
{
	//! Code returns pressure for a given chamber volume

	// Variables
	double new_lv_circumference;
	double new_hs_length;
	double delta_hs_length;
	double new_stress;
	double internal_r;
	double wall_thickness;
	double P_in_Pascals;
	double P_in_mmHg;

	// Code
	new_lv_circumference = return_lv_circumference_for_chamber_volume(cv);

	new_hs_length = 1.0e9 * new_lv_circumference / vent_n_hs;

	delta_hs_length = new_hs_length - p_hs->hs_length;

	// Deduce stress for the new hs_length
	new_stress = p_hs->return_wall_stress_after_delta_hsl(delta_hs_length);

	new_stress = GSL_MAX(-1000.0, new_stress);

	internal_r = return_internal_radius_for_chamber_volume(cv);

	wall_thickness = return_wall_thickness_for_chamber_volume(cv);

	// Pressure from Laplace's law
	// https://www.annalsthoracicsurgery.org/action/showPdf?pii=S0003-4975%2810%2901981-8

	if (internal_r < 1e-6)
	{
		P_in_Pascals = 0.0;
		cout << "Hemi_vent, internal_r ~= 0.0 problem\n";
		cout << "delta_hs_length: " << delta_hs_length << "\n";
		cout << "new_stress: " << new_stress << "\n";
		cout << "wall_thickness: " << wall_thickness << "\n";
	}
	else
	{
		P_in_Pascals = (new_stress * wall_thickness *
			(2.0 + (vent_thick_wall_multiplier * (wall_thickness / internal_r)))) /
			internal_r;
	}

	P_in_mmHg = P_in_Pascals / (0.001 * GSL_CONST_MKSA_METER_OF_MERCURY);

	return P_in_mmHg;
}

void hemi_vent::update_chamber_volume(double new_volume)
{
	//! Function updates the chamber volume
	
	// Variables
	double new_circumference;
	double delta_circumference;
	double delta_hsl;

	// Code

	new_circumference = return_lv_circumference_for_chamber_volume(new_volume);

	delta_circumference = new_circumference - vent_circumference;

	delta_hsl = 1e9 * delta_circumference / vent_n_hs;

	p_hs->change_hs_length(delta_hsl);

	vent_circumference = new_circumference;

	// Dump if necessary
	if (!p_cmv_options->cb_dump_file_string.empty())
		p_hs->p_myofilaments->dump_cb_distributions();
}

void hemi_vent::update_beat_metrics()
{
	//! Code updates beat metrics
	
	// Variables
	double cardiac_cycle_s;

	// Update beat values
	vent_stroke_work_J = p_cmv_results->return_stroke_work(p_parent_cmv_system->sim_t_index);
	vent_stroke_energy_used_J = p_cmv_results->return_energy_used(p_parent_cmv_system->sim_t_index);
	vent_efficiency = -vent_stroke_work_J / vent_stroke_energy_used_J;

	// Calculate the ejection fraction
	stats_structure* p_v_stats = new stats_structure;

	// Calculate stroke volume
	p_cmv_results->calculate_sub_vector_statistics(
		p_cmv_results->gsl_results_vectors[p_cmv_results->volume_vent_field_index],
		p_cmv_results->last_beat_t_index, p_parent_cmv_system->sim_t_index, p_v_stats);

	vent_stroke_volume = p_v_stats->max_value - p_v_stats->min_value;

	vent_ejection_fraction = vent_stroke_volume / p_v_stats->max_value;

	// Calculate period of cardiac cycle to get cardiac output
	cardiac_cycle_s = p_parent_cmv_system->cum_time_s -
		gsl_vector_get(p_cmv_results->gsl_results_vectors[p_cmv_results->time_field_index],
			p_cmv_results->last_beat_t_index);

	vent_cardiac_output = 60.0 * vent_stroke_volume / cardiac_cycle_s;

	// Backfill results
	p_cmv_results->backfill_beat_data(
		p_cmv_results->gsl_results_vectors[p_cmv_results->vent_stroke_work_field_index],
		vent_stroke_work_J, p_parent_cmv_system->sim_t_index);

	p_cmv_results->backfill_beat_data(
		p_cmv_results->gsl_results_vectors[p_cmv_results->vent_stroke_energy_used_field_index],
		vent_stroke_energy_used_J, p_parent_cmv_system->sim_t_index);

	p_cmv_results->backfill_beat_data(
		p_cmv_results->gsl_results_vectors[p_cmv_results->vent_efficiency_field_index],
		vent_efficiency, p_parent_cmv_system->sim_t_index);

	p_cmv_results->backfill_beat_data(
		p_cmv_results->gsl_results_vectors[p_cmv_results->vent_ejection_fraction_field_index],
		vent_ejection_fraction, p_parent_cmv_system->sim_t_index);

	p_cmv_results->backfill_beat_data(
		p_cmv_results->gsl_results_vectors[p_cmv_results->vent_stroke_volume_field_index],
		vent_stroke_volume, p_parent_cmv_system->sim_t_index);

	p_cmv_results->backfill_beat_data(
		p_cmv_results->gsl_results_vectors[p_cmv_results->vent_cardiac_output_field_index],
		vent_cardiac_output, p_parent_cmv_system->sim_t_index);

	// Update hs metrics
	p_hs->update_beat_metrics();
}

void hemi_vent::calculate_vent_ATP_used_per_s()
{
	//! Function updates vent_ATP_used_per_s

	// Variables
	double d_heads;				// density of heads per m^3
	double v_myocardium;

	// Code
	d_heads = (1.0 - p_hs->hs_prop_fibrosis) *
		p_hs->hs_prop_myofilaments *
		p_hs->p_myofilaments->myof_cb_number_density *
		(1.0 / (1e-9 * p_hs->hs_reference_hs_length));

	v_myocardium = 0.001 * vent_wall_volume;

	vent_ATP_used_per_s = -d_heads * v_myocardium *
		p_hs->p_myofilaments->myof_ATP_flux /
		GSL_CONST_NUM_AVOGADRO;
}