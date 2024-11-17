# TemplateI2C

Create Arduino I2C slave devices, as well as its I2C Drivers with Template Classes.

TemplateI2CSlave:
  - Class-templated extension of I2C device.
  - No work done during RequestOutput, except copying a pre-set buffer to the I2C buffer.
  - Very light RequestEvent processing with optional async receiver.
  - Base features:
    - Get Device Id.
    - Device Reset.

TemplateI2CDriver:
  - Class-templated extension of I2C driver.
  - Async requests enable any I2C slow-to-respond device to work with a co-op delay between request write and read.
