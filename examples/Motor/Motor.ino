 #include <Motor.h>
 
VNH5019 motor(9,10,8);
 void setup(){
   motor.mirror();
   motor.brake(64);
   delay(2000);
   
   motor.brake();
   delay(2000);
   
   motor.coast(64);
   delay(2000);
 }
 void loop(){
      motor.brake();

   
 }
