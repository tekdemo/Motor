# Motor.h

This class aims to simplify adding motors and motor control ICs to Arduino projects. Rather than cluttering each directional change with multiple PWM writes, enable IO writes, and other IC specific handling, this class wraps common functions into a nice, simple API:
``` Arduino
#include <Motor.h>
DualPWM motor(9,10,8);

void setup(){
	motor.write(128);
	delay(2000);
	motor.write(-128);
	delay(2000);
	motor.write(0);
}
void loop(){}
```

This class also adds additional features to clean up the code base for readablity, and enable more complex features without code redundancy :

- Initialization automatically configures pin directions, and sets motor in a known state.
- `motor.mirror()` lets you write the same value to motors mounted opposite each other. This avoids tracking negatives all over your code.
- `motor.enable()` and `motor.disable()` will halt all future writes to the motor, even if the motor IC does not have hardware disabling. Great for emergency functions, error handling, or disarm functions via remote control.
- `motor.write()` writes motor values according to  `motor.enableCoastMode(boolean)` which lets you change the default brake/coast modes in a single place.
- `brake()` function accepts optional parameters( `brake(-125)` ), allowing you to override enableCoastMode settings for critical movement. 
- Similarly, `coast()` accepts values for mixed-mode decay operation, if supported by the driver IC.
- `motor.read()` reports the last value, reducing variables elsewhere


## Extending 
The base class enforces interface compliance, ensuring that any additional motors will be interchangable with existing code. 

In the above example, switching to a new chip in hardware can be done by replacing `DualPWM motor(9,10,8);` with  `DVR8837 motor(9,10,8);`. This makes operating and maintining multiple hardware revisions painless. This also allows you to create container
classes that have one or more Motor instances, without internally relying on specific implimentations.

## Current Support:
Partially tested and functional chips:

	- Generic PWM/PWM/Enable chips
	- TI DVR8837
	- TI SN754410NE
	- ZXBM5210
	- VNH5019
	
Additional ICs planned as they're added to hardware and tested

	- MOSTFET and BJT arrays, with switching deadband and software shoot-through protection
	- Generic OutputA/OutputB/PWM ICs
	- Generic Direction/PWM ICs
	
## Known Issues
	- Handling of Coast mode for dual-PWM profiles is lacking
	- Coast mode output values aren't sane for some chips.