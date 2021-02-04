# ArduinoTemplateI2CSlave

Create Arduino I2C slave devices, as well as its I2C Drivers with Template Classes.

Common API:
  - Easy extension of BaseAPI class.


TemplateI2CSlave:
  - Uses Extended BaseAPI class.
  - Easy extension of I2C device.
  - No work done during RequestOutput, except copying a pre-set buffer to the I2C buffer.
  - Very light RequestEvent processing, depends mostly on slave implementation complexity.
  - Optional features (set by #defines):
    - Get Device Id.
    - Device Reset.
    - Device Low Power mode (with external pointer function).
	- Track last event timestamp (millis timestamp).


TemplateI2CDriver:
  - Uses the same Extended BaseAPI class.
  - Easy extension of I2C driver.
