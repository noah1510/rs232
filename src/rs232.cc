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
void sakurajin::RS232::Print(const std::string& text){
    if (!available){
        return;
    }

    for(unsigned char c : text){
        Write(c);
    }
}

bool sakurajin::RS232::IsAvailable() const{
    return available;
}

std::string_view sakurajin::RS232::GetDeviceName() const{ 
    return devname;
}

sakurajin::RS232::~RS232(){
  Close();
}
