#pragma once

/**
/* @file		FiberSim_series_component.h
/* @brief		Header file for a FiberSim_series_component object
/* @author		Ken Campbell
*/

#include <iostream>

// Forward declaration
class FiberSim_muscle;

class cmv_results;

class FiberSim_series_component
{
public:

	// Variables
	
	FiberSim_muscle* p_parent_fs_muscle;					/**< Pointer to the parent FiberSim muscle */

	cmv_results* p_cmv_results_beat;						/**< pointer to a simulation results object */

	double sc_extension;									/**< double holding sc_length in nm */

	double sc_k_stiff;										/**< double holding sc_stiffness in N M^-1 */

	double sc_force;										/**< double holding the sc_force in N m^-2 */

	// Functions

	/**
	* Constructor
	*/
	FiberSim_series_component(FiberSim_muscle* set_p_parent_fs_muscle);

	/**
	* Destructor
	*/
	~FiberSim_series_component(void);

	// Other functions
	double return_series_extension(double muscle_force);

	double return_series_force(double series_extension);

	void initialise_for_simulation(void);

};