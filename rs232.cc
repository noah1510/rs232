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

#ifndef kranfix_rs232_rs232_cc
#define kranfix_rs232_rs232_cc

#include "rs232.hpp"

#  ifdef __linux__
#    include "rs232_linux.cc"
#  else
#    include "rs232_win.cc"
#  endif // __linux__

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

#endif // kranfix_rs232_rs232_cc
