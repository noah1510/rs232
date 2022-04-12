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

int kfx::RS232::IsAvailable() {
    return available;
}

const std::string& kfx::RS232::GetDeviceName() { 
    return devname;
}
