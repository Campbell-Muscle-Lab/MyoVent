/**
 * @file    MyoVentCpp.cpp
 * @brief   Core file for the MyoVentCpp model
 * @author  Ken Campbell
 */

#include <stdio.h>

// Includes
#include "cmv_system.h"

// Pointers
cmv_system* p_cmv_system;       /**< pointer to a cmv_system */


int main()
{
    printf("Ken was here\n");

    // Initialize
    p_cmv_system = new cmv_system();

    // Tidy up
    delete p_cmv_system;

    // Close
    printf("Closing MyoVentCpp\n");

    return(1);

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
