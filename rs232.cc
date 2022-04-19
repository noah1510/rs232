/*
***********************************************
*
* Author: Frank Andre Moreno Vera
*
* Copyright (C) 2014 Frank Andre Moreno Vera
*
* frank@ariot.pe
*
***********************************************
*/

#include "rs232.hpp"

// Sends a string to serial port till finding a '\0'
void kfx::RS232::Print(const char *text){
  while(*text != 0) Write( *(text++) );
}

bool kfx::RS232::IsAvailable() const{
    return available;
}

std::string_view kfx::RS232::GetDeviceName() const{ 
    return devname;
}
