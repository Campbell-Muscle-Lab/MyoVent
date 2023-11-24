/**
/* @file		FiberSim_series_component.cpp
/* @brief		Source file for a FiberSim_series_component object
/* @author		Ken Campbell
*/

#include <cstdio>

#include "FiberSim_series_component.h"

#include "FiberSim_muscle.h"
#include "FiberSim_model.h"

#include "gsl_math.h"

#include "muscle.h"

#include "cmv_model.h"



// Constructor
FiberSim_series_component::FiberSim_series_component(FiberSim_muscle* set_p_parent_fs_muscle)
{
	//! Constructor

	p_parent_fs_muscle = set_p_parent_fs_muscle;

	// Update from model
	FiberSim_model* p_fs_model;
	p_fs_model = p_parent_fs_muscle->p_parent_muscle->p_cmv_model->p_fs_model;

	sc_k_stiff = p_fs_model->sc_k_stiff;

	// Set some defaults
	sc_extension = 0.0;
	sc_force = 0.0;
}

// Destructor
FiberSim_series_component::~FiberSim_series_component(void)
{
	// Destructor
}

// Other functions
double FiberSim_series_component::return_series_extension(double muscle_force)
{
	//! Returns the extension of the series component for a given force

	// Variables
	double ext;

	// Code
	if (gsl_isnan(sc_k_stiff))
	{
		printf("Error in fs_series component: sc_k_stiff is NAN\n");
		exit(1);
	}

	ext = muscle_force / sc_k_stiff;

	return ext;
}

double FiberSim_series_component::return_series_force(double series_extension)
{
	//! Returns the force in the series component for a given extension

	// Variables
	double series_force;

	// Code
	series_force = series_extension * sc_k_stiff;

	return series_force;
}
