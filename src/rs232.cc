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
    
    return ReadUntil({'\n'});
}

std::tuple<std::string, int> sakurajin::RS232::ReadUntil(std::vector<unsigned char> conditions){
    if(!available){
        return {"", -1};
    }
    
    std::string message = "";
    int errCode = 0;
    unsigned char nextChar = '\n';
    bool stop = false;
        
    while(!stop){
        //read the next char and append if there was no error
        std::tie(nextChar, errCode) = ReadNextChar();
        
        if(errCode < 0){
            return {"", -1};
        }
        message += nextChar;

        //check if a stop condition is met
        for(auto cond:conditions){
            if (cond == nextChar){
                stop = true;
                break;
            }
        }
    }
    
    return {message,0};
}

