// I2CSlaveDefinitions.h

#ifndef _I2CSLAVEDEFINITIONS_h
#define _I2CSLAVEDEFINITIONS_h

//This feature is only useful if you have ROM to spare and want a clean way to composite a message class,
// or just save RAM with variable size template message.
//Otherwise, it's best turned off, as it simplifies the class object into a simples intance with no virtual calls.
//Not using this feature forces your message objects to all be of the same (max) size.
//#define I2C_MESSAGE_IMPLEMENT_INTERFACE

//Basic messaging error tracking is nice, but it can be disabled with this property.
#define I2C_SLAVE_COMMS_ERRORS_ENABLE

//If you don't use a task scheduller, you need to manually the callback whenever possible.
#define I2C_SLAVE_USE_TASK_SCHEDULER

//Enable device reset.
#define I2C_SLAVE_DEVICE_RESET_ENABLE

//Enable device health messages.
#define I2C_SLAVE_HEALTH_ENABLE

#endif

