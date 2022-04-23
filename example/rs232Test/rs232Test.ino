#include "Arduino.h"

void setup() {
  Serial.begin(19200);
}

void loop() {
  while(Serial.available() == 0){
    delay(5);
  }
  
  auto start = Serial.read();
  while(Serial.available()){
    start = Serial.read();
  }
  
  Serial.println("SnycingIO...");
  
  char A[80];
  int cont = 0;
  while(!Serial.available());
  while(Serial.available()){
    A[cont] = Serial.read();
    delay(10); // A timeout with delay
    if(cont == 0 && A[cont] == '\n'){
      continue;
    }
    cont++;
  }
  A[cont] = 0; // Null character

  Serial.print("Recived buffer: ");
  Serial.print(A);
  Serial.println();
  
  delay(500);

}
