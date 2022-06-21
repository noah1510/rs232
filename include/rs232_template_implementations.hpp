#ifdef SAKURAJIN_RS232_HPP_INCLUDED

    template<class Rep, class Period>
    std::tuple<unsigned char, int> sakurajin::RS232::ReadNextChar(std::chrono::duration<Rep, Period> waitTime, bool ignoreTime){
        if(!available){
            return {'\0', -1};
        }
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        unsigned char IOBuf = '\0';
        int errCode = 0;
        
        if(ignoreTime){
            
            do{
                errCode = Read(&IOBuf,1);
            }while(errCode != 1);
            
        }else{
            
            do{
                errCode = Read(&IOBuf,1);
                
                if(std::chrono::high_resolution_clock::now() - startTime > waitTime){
                    return {'\0', -2};
                }
            }while(errCode != 1);
            
        }
        
        return {IOBuf, 0};
    }

#else

    #error "do not direktly include this header!"

#endif
