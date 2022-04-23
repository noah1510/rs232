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
    if(!available){
        return {'\0', -1};
    }
    
    unsigned char IOBuf = '\0';
    int errCode = 0;
    
    do{
        errCode = Read(&IOBuf,1);
    }while(errCode != 1);
    
    return {IOBuf, 0};
}

std::tuple<std::string, int> sakurajin::RS232::ReadNextMessage(){
    if(!available){
        return {"", -1};
    }
    
    std::string message = "";
    int errCode = 0;
    unsigned char nextChar = '\0';
        
    do{
        std::tie(nextChar, errCode) = ReadNextChar();
        
        if(errCode < 0){
            return {"", -1};
        }
        message += nextChar;
        
    }while(nextChar != '\0' && nextChar != '\n' && nextChar != '\r');
    
    return {message,0};
}

