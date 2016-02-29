/// \file
/// Project:                Migration Sandbox \n
/// File Name:              packet.cpp \n
/// Required Libraries:     none \n
/// Date created:           Wed Feb 17 2016 \n
/// Engineers:              Conor Gardner \n
/// Compiler:               g++ \n
/// Target OS:              Ubuntu Linux 14.04 \n
/// Target architecture:    x86 (64 bit) */

#include "packet.h"

class component;

packet::packet
(
    component* original_source_,
    component* final_destination_,
    packetType type_,
    const std::string& name_,
    unsigned address_,
    unsigned cooldown_
){
    
    this->original_source = original_source_;
    this->final_destination = final_destination_;
    this->type = type_;
    this->name = name_;
    this->address = address_;
    this->cooldown = cooldown_;
    
}
