/**
/* @file		growth_activation.cpp
/* @brief		Source file for a growth_activation object
/* @author		Ken Campbell
*/

#include "stdio.h"
#include "math.h"

#include "growth_activation.h"

// Constructor
growth_activation::growth_activation(double set_t_start_s, double set_t_stop_s)
{
	// Initialise

	// Code

	t_start_s = set_t_start_s;
	t_stop_s = set_t_stop_s;
}

// Destructor
growth_activation::~growth_activation(void)
{
	// Destructor
	// 
	// Tidy up
}

// Other functions
int growth_activation::return_status(double t_test_s)
{
	// Returns 1 if growth is active, 0 otherwise

	if ((t_test_s >= t_start_s) && (t_test_s <= t_stop_s))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
