/// \file
/// Project:                HMC Migration Simulator \n
/// File Name:              packet.cpp \n
/// Date created:           Feb 17 2016 \n
/// Engineers:              Khalique Ahmed
///                         Conor Gardner
///                         Dong Kai Wang\n
/// Compilers:              g++, vc++ \n
/// Target OS:              Ubuntu Linux 14.04
///							Windows 7 \n
/// Target architecture:    x86_64 */

#include "packet.h"

class component;

packet::packet
(
    component* original_source_,
    component* final_destination_,
	component* swap_destination_,
	unsigned swap_tag_,
    packetType type_,
    uint64_t address_,
    unsigned bytes_accessed_,
    unsigned cooldown_,
    const std::string& name_
){
    
    this->original_source = original_source_;
    this->final_destination = final_destination_;
	this->swap_destination = swap_destination_;
	this->swap_tag = swap_tag_;
    this->type = type_;
    this->name = name_;
    this->address = address_;
    this->bytes_accessed = bytes_accessed_;
    this->cooldown = cooldown_;
    
}
