// I2CSlaveDefinitions.h

#ifndef _I2CSLAVEDEFINITIONS_h
#define _I2CSLAVEDEFINITIONS_h

//Basic messaging error tracking is nice, but it can be disabled with this property.
#define I2C_SLAVE_COMMS_ERRORS_ENABLE

//If you don't use a task scheduller, you need to manually the callback whenever possible.
#define I2C_SLAVE_USE_TASK_SCHEDULER

//Enable device reset.
#define I2C_SLAVE_DEVICE_RESET_ENABLE

//Enable device health messages.
#define I2C_SLAVE_HEALTH_ENABLE

#endif

