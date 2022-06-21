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
        Write(&c,1);
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

std::tuple<unsigned char, int> sakurajin::RS232::ReadNextChar() {
    return ReadNextChar(std::chrono::microseconds(1), true);
}

std::tuple<std::string, int> sakurajin::RS232::ReadNextMessage(){
    return ReadNextMessage(std::chrono::microseconds(1), true);
}

std::tuple<std::string, int> sakurajin::RS232::ReadUntil(const std::vector<unsigned char>& conditions){
    return ReadUntil(conditions, std::chrono::microseconds(1), true);
}

