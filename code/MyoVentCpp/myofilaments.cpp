/**
/* @file		myofilaments.cpp
/* @brief		Source file for a myofilaments object
/* @author		Ken Campbell
*/

#include <iostream>

#include "myofilaments.h"
#include "half_sarcomere.h"
#include "kinetic_scheme.h"
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

void myofilaments::update_p_cmv_options(cmv_options* set_p_cmv_options)
{
	//! Function updates the pointer to the cmv_options
	//! This is not available when the myofilaments are constructed
	//! from a model file

	p_cmv_options = set_p_cmv_options;

	// Now update the daughter kinetic_scheme
	p_m_scheme->update_p_cmv_options(p_cmv_options);

}
