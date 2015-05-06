#ifndef MOTORS_H
#define MOTORS_H
#include "Arduino.h"
/*
 * A simple library to set up different ways to speak to motor control chips
 * tekdemo@gmail.com
 */

//This is a helpful guide to reduce noise, but 
//unfortunately affects the delay() and millis() timing. 
//http://playground.arduino.cc/Main/TimerPWMCheatsheet


/*
 Create a template for which other classes can then inherit and override to simplify class creation
 https://stackoverflow.com/questions/9756893/how-to-implement-interfaces-in-c
 https://stackoverflow.com/questions/318064/how-do-you-declare-an-interface-in-c 
 https://en.wikibooks.org/wiki/C++_Programming/Classes/Abstract_Classes
 
 class IDemo
 {
 public:
	 virtual IDemo() {} //constructor
	 virtual ~IDemo() {} //destructor
	 virtual void OverrideMe() = 0;
	 };
	 
class Parent
{
public:
	virtual ~Parent();
};
	
class Child : public Parent, public IDemo
{
public:
	virtual void OverrideMe()
	{
		//do stuff
	}
};

//*/

class Motor{
	/*Suggested variables for typical class declarations
	int _val;
	boolean _enabled;
	boolean _mirrored;
	boolean _enableCoastMode;
	byte _brakeValue;
	byte _coastValue;
	*/
public:
	/** Not defined: Must be created per class, as per the needed declarations
	 */
	//Motor() ; //constructor
	//virtual ~motor() ; //destructor
		
	/** Enables writes to the motor, sets to last known value (if any)
	 */
	virtual void enable()=0;
	
	/** Stops motor, and disables writes to the motor
	 *  May be implimented in hardware or software, depending on motor controller
	 */
	virtual void disable()=0;
	virtual boolean isEnabled()=0;	
	
	/** Reverse all motor direction writes
	 */
	virtual void mirror()=0;
	virtual void mirror(boolean mirrored)=0;
	
	/** Enable Coast Mode
	 *  Passing True provides coast mode 
	 *  Passing False provides brake mode (default for most cases)
	 */
	virtual void enableCoastMode(boolean coast)=0;
	
	/** Engage the motor while braking between motor pulses
	 *  This pushes, but doesn't easily reverse if negative force is applied
	 */
	virtual void brake(int value)=0;

	/** Brake the motor (hard stop)
	 * Same as brake(0);
	 */
	virtual void brake(void)=0;

	/** Engage the motor while braking between motor pulses
	 *  This pushes, provides lower torque on lower value writes, and allows pushback when negative force is applied. 
	 *  Also known as mixed-decay mode in documentation and datasheets. 
	 * NOTE: Not possible with all hardware, and may be disabled or set to brake();
	 */
	virtual void coast(int value)=0;
	
	/** Disconnect the motor (soft stop)
 	 * Same as coast(0);
	 */
	virtual void coast(void)=0;

	/**
	 * Return the last value written to the motor
	 */
	virtual int read()=0;
	
	/** Write a value to the motor using the currently set brake mode if enabled.
	 *  A sane person uses this coast(int) or brake(int) according to the default brake mode
	 */
	virtual void write(int value)=0;
};



class DualPWM :public Motor{
private:
protected:
	int _val;
	int _a; //anticlockwise rotating pin
	int _c; //clockwise rotating pin
	int _en; //pin for enable
	boolean _enabled;
	boolean _mirrored;
	boolean _enableCoastMode;
	
	byte _brakeValue;
	byte _coastValue;
public:			
	DualPWM(int a, int c,int en){
		//Constructor: Set motor pins for writing
		_a=a;
		_c=c; 
		_en=en;
		pinMode(_a,OUTPUT);
		pinMode(_c,OUTPUT);
		pinMode(_en,OUTPUT);
		enable();
               _brakeValue=255;
               _coastValue=0;

	};
	
	int read(){
		return _val;
	};
	
	void enable(){
		digitalWrite(_en,HIGH);
		_enabled=true;
	}
	void disable(){
		digitalWrite(_en,LOW);	
		_enabled=false;
		
	}
	boolean isEnabled(){
		return _enabled;
	}

	void brake(int value){
		if(!_enabled)return;
		
		if(_mirrored)value=-value;
		
		_val=constrain(value,-255,255);
		if(_val>=0){
			analogWrite(_a,_brakeValue);
			analogWrite(_c,_val);
		}
		else if(_val<0){
			analogWrite(_a,abs(_val));
			analogWrite(_c,_brakeValue);
		}
	};
	
	void coast(int value){
		if(!_enabled)return;
		
		if(_mirrored)value=-value;
				
		_val=constrain(value,-255,255);

		//TODO: Verify _val inversion is correct and does sane things
		_val=255-_val;
		
		if(_val>=0){
			analogWrite(_a,_coastValue);
			analogWrite(_c,_val);
		}
		else if(_val<0){
			analogWrite(_a,abs(_val));
			analogWrite(_c,_coastValue);
		}	
	};
	void brake(void){
		brake(_brakeValue);
	};
	void coast(void){
		coast(_coastValue);
	};

	void write(int value){
		if(_enableCoastMode==true)coast(value);
		else brake(value);
	}
		
	void mirror(){
		_mirrored=!_mirrored;
	}
	void mirror(boolean mirrored){
		_mirrored=mirrored;
	}
		
	void enableCoastMode(boolean coast){
		_enableCoastMode=coast;
	}
	
}; //manditory for classes, apparently.

//Simple DualPWM class with no modifications
class DVR8837 :public DualPWM{
public:
	DVR8837(int a,int c, int en)
	:DualPWM(a,c,en){}
};

//TODO: Test on hardware
class ZXBM5210 :public DualPWM{
public:
	ZXBM5210(int a,int c, int en)
	:DualPWM(a,c,en){}
};


//DualPWM, but with slightly annoying to impliment coast mode.
//Will coast when disabled
//TODO: add proper support for coast
class SN754410NE :public DualPWM{
public:
	SN754410NE(int a,int c, int en)
	:DualPWM(a,c,en){}
	
	//TODO Mixed-mode coast can technically be implimented as a third PWM pin on enable
	//However, as it requires a third PWM, it's not yet implimented due to 
	// problematic results when used with non-pwm pins.
	// when disabled, will result in coast mode
	void coast(int value){
		DualPWM::brake(value);
	}
	void coast(void ){
		DualPWM::brake();
	}
};



class FourWire :public Motor{
private:
protected:
	int _val;
	int _a; //anticlockwise rotating pin
	int _c; //clockwise rotating pin
	int _pwm; //clockwise rotating pin
	int _en; //pin for enable
	boolean _enabled;
	boolean _mirrored;
	boolean _enableCoastMode;
	
	byte _brakeValue;
	byte _coastValue;
public:			
	FourWire(int a, int c,int pwm,int en){
		//Constructor: Set motor pins for writing
		_a=a;
		_c=c; 
		_en=en;
		_pwm=pwm;
		pinMode(_a,OUTPUT);
		pinMode(_c,OUTPUT);
		pinMode(_en,OUTPUT);
		pinMode(_pwm,OUTPUT);
		enable();
               _brakeValue=255;
               _coastValue=0;

	};
	
	int read(){
		return _val;
	};
	
	void enable(){
		digitalWrite(_en,HIGH);
		_enabled=true;
	}
	void disable(){
		digitalWrite(_en,LOW);	
		write(0);
		
		_enabled=false;
		
	}
	boolean isEnabled(){
		return _enabled;
	}

	void brake(int value){
		if(!_enabled)return;
		
		if(_mirrored)value=-value;
		
		_val=constrain(value,-255,255);
				
		if(_val==0){
			digitalWrite(_a,HIGH);
			digitalWrite(_c,HIGH);
		}
		else if(_val>0){ //anticlockwise
			digitalWrite(_a,HIGH);
			digitalWrite(_c,LOW);//low
		}
		else if(_val<0){
			digitalWrite(_a,LOW);
			digitalWrite(_c,HIGH);
		}
		analogWrite(_pwm, abs(_val));
	};
	
	void coast(int value){
		if(!_enabled)return;
		
		if(_mirrored)value=-value;

		_val=constrain(value,-255,255);

		/*TODO: This emulates a mixed mode decay fairly well,
		 *but doesn't seem to be a proper way to handle it. 
		 * Eventually, a slog through the datasheet may be in order,
		 * and possibly attempting to PWM the _en pin.
		 */ 
		analogWrite(_pwm, 128);
		
		if(_val==0){
			analogWrite(_c,0);
			analogWrite(_a,0);
		}
		else if(_val>0){
			analogWrite(_c,0);
			analogWrite(_a,255-_val);
		}
		else if(_val<0){
			analogWrite(_a,0);
			analogWrite(_c,255-abs(_val));
		}

	};
	void brake(void){
		brake(_brakeValue);
	};
	void coast(void){
		coast(_coastValue);
	};

	void write(int value){
		if(_enableCoastMode==true)coast(value);
		else brake(value);
	}
		
	void mirror(){
		_mirrored=!_mirrored;
	}
	void mirror(boolean mirrored){
		_mirrored=mirrored;
	}
		
	void enableCoastMode(boolean coast){
		_enableCoastMode=coast;
	}
	
}; //manditory for classes, apparently.

// Found on Pololu Dual Motor Driver shields, https://www.pololu.com/product/2502
class VNH5019 :public FourWire{
public:
	VNH5019(int a,int c, int en,int pwm)
	:FourWire(a,c,en,pwm){}
}; 







/**
class DualHBridge: public Motor {
private:
	int _val;
	int _ah; //anticlockwise rotating pin
	int _al; //anticlockwise rotating pin
	int _ch; //anticlockwise rotating pin
	int _cl; //clockwise rotating pin

	
public:			
	DualHBridge(int ah, int al,int ch, int low cl){
		//Constructor: Set motor pins for writing
		_a=a;
		_c=c;
		pinMode(_a,OUTPUT);
		pinMode(_c,OUTPUT);
		pinMode(_a,OUTPUT);
		pinMode(_c,OUTPUT);
		enable();
               _brakeValue=255;
               _coastValue=0;
	};
	
	int read(){
		return _val;
	};
	
	//Should be present on all motors for API reasons, but may not actually do anything useful. 
	void enable(){

	}
	void disable(){
		
	}
	
	//Drives the motor at full capability. Brakes between pulses.
	void brake(int val){
		if(!_enabled)return;
		
		if(_mirrored)val=-val;
		
		_val=constrain(val,-255,255);
		
		// Turn off all motors
		analogWrite(_ah,MOTOR_BRAKE);
		analogWrite(_al,MOTOR_COAST);
		analogWrite(_ch,MOTOR_BRAKE);
		analogWrite(_cl,MOTOR_COAST);
		
		if(val>=0){
			analogWrite(_ah,MOTOR_BRAKE);
			analogWrite(_al,MOTOR_COAST);
			analogWrite(_ch,MOTOR_BRAKE);
			analogWrite(_cl,MOTOR_COAST);
		}
		if(val<0){
			analogWrite(_ah,MOTOR_BRAKE);
			analogWrite(_al,MOTOR_COAST);
			analogWrite(_ch,MOTOR_BRAKE);
			analogWrite(_cl,MOTOR_COAST);
		}
	};
	
	//This coasts the motor between pulses, allowing for gentler movements when force is applied.
	void coast(int val){
		_val=val;
		if(val>=0){
			analogWrite(_a,MOTOR_COAST);
			analogWrite(_c,val);
		}
		if(val<0){
			analogWrite(_a,-val);
			analogWrite(_c,MOTOR_COAST);
		}	
		//TODO: Since this is active low, we should write out (255-val), not val
		
	};
	void brake(void){
		brake(0);
	}
	void coast(void){
		coast(0);
	};
	
	
}; //manditory for classes, apparently.
//*/

#endif
