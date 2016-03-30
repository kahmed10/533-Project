/// \file
/// Project:                Migration Sandbox \n
/// File Name:              system_driver.cpp \n
/// Required Libraries:     none \n
/// Date created:           Thurs Mar 17 2016 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 and Windows \n
/// Target architecture:    x86 (64 bit) */

#include <climits>
#include <iostream>
#include "component.h"
#include "debug.h"
#include "system_driver.h"

system_driver::~system_driver()
{
    // destroy and de-alocate all resident components.  Nothing weird here.
    unsigned num_resident_components = this->resident_components.size();
    for (unsigned ix = 0; ix < num_resident_components; ix++)
    {
        delete this->resident_components[ix];
    }
}

void system_driver::add_component(component* c)
{
    check(c != NULL, "Cannot add NULL component to system_driver");
    this->resident_components.push_back(c);
}

// this function steps through each component's generate(),
// advance_cooldowns(), and wake_packets() functions.  Quiescence occurs
// when all components return UINT_MAX as their cooldown for all stages
unsigned long system_driver::simulate()
{
    
    unsigned long elapsed_ticks = 0;
    
    unsigned advancement_accumulator;
    unsigned advancement_amount;
    unsigned num_resident_components = this->resident_components.size();
    unsigned iteration = 1;
    do
    {
        
        advancement_accumulator = UINT_MAX;
        
        // print...
        std::cout
            << std::endl << std::endl
            << "### Iteration "
            << iteration
            << "   Elapsed Time "
            << elapsed_ticks
            << " ###"
            << std::endl;
        iteration++;
        for (unsigned ix = 0; ix < num_resident_components; ix++)
        {
            this->resident_components[ix]->print();
        }
        
        // generate...
        for (unsigned ix = 0; ix < num_resident_components; ix++)
        {
            unsigned cur_cooldown = this->resident_components[ix]->generate();
            if (cur_cooldown < advancement_accumulator)
                advancement_accumulator = cur_cooldown;
        }
        
        // advance cooldowns...
        for (unsigned ix = 0; ix < num_resident_components; ix++)
        {
            unsigned cur_cooldown = this->resident_components[ix]->advance_cooldowns(advancement_amount);
            if (cur_cooldown < advancement_accumulator)
                advancement_accumulator = cur_cooldown;
        }
        
        // wake packets...
        for (unsigned ix = 0; ix < num_resident_components; ix++)
        {
            unsigned cur_cooldown = this->resident_components[ix]->wake_packets();
            if (cur_cooldown < advancement_accumulator)
                advancement_accumulator = cur_cooldown;
        }
        
        advancement_amount = advancement_accumulator;
        elapsed_ticks += (unsigned long)advancement_accumulator;
        
    } while (advancement_accumulator != UINT_MAX);
    
    return elapsed_ticks;
    
}


