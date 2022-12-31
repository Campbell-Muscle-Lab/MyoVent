/**
/* @file		myofilaments.cpp
/* @brief		Source file for a myofilaments object
/* @author		Ken Campbell
*/

#include <iostream>

#include "myofilaments.h"
#include "half_sarcomere.h"
#include "cmv_model.h"
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

	// Set the pointer to the parent system
	p_parent_hs = set_p_parent_hs;
	p_cmv_model = p_parent_hs->p_cmv_model;

	// Set other pointers safe
	p_cmv_results = NULL;

	// Initialize
	myof_cb_number_density = p_cmv_model->myof_cb_number_density;
	myof_prop_fibrosis = p_cmv_model->myof_prop_fibrosis;
	myof_prop_myofilaments = p_cmv_model->myof_prop_myofilaments;
	myof_k_cb = p_cmv_model->myof_k_cb;
}

// Destructor
myofilaments::~myofilaments(void)
{
	//! Destructor

	// Code
	cout << "Myofilaments destructor()\n";
}
