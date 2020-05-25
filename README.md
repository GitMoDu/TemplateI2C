# ArduinoTemplateI2CSlave

Create Arduino I2C slave devices, as well as its I2C Drivers with Template Classes.

Common API file.
  - Easy extension of BaseAPI class.


TemplateI2CSlave.
  - Uses Extended BaseAPI class.
  - Easy extension of I2C device.
  - No work done during RequestOutput.
  - Base features (All optional):
    - Get Device Id.
    - Device Reset.
    - Set Low Power mode (with external pointer function).
  - Optional Async version, based on TaskScheduler (https://github.com/arkhipenko/TaskScheduler).
    - Minimal interrupt disruption, messages aren't processed in interrupt.
    - Double buffered message processing.


Template Driver for using device.
  - Uses Extended BaseAPI class.
  - Easy extension of I2C driver.

