# foundry10 Underwater ROV Code
## Arduino code for ROV project at foundry10 (Jan 2015 - July 2016)
* One Arduino above the surface, another underwater in the ROV  
* Controlled with either an XBox controller or simple buttons for testing purposes
* Both can communicate with each other through an Ethernet cable and UDP packets

#### Surface Arduino
* XBox controller interfaces to Arduino through USB Shield
* Communicates to underwater Arduino through Ethernet cable, which requires an Ethernet Shield on top of the USB shield

#### Underwater Arduino
* Recieves data from surface Arduino through Ethernet, which requires another Ethernet Shield
* First packet received from surface includes whether simple buttons are used and joystick deadzone to be used for thruster power calculation (if XBox controller is used for control rather than simple buttons)
