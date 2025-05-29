#include "StateMachine/StateMachine.h"

//* ************************************************************************
//* ************************ SENSOR FUNCTIONS ***************************
//* ************************************************************************
//! Sensor and switch reading functions
//! Functions for reading all sensors and limit switches

// External Bounce objects declared in main.cpp
extern Bounce woodSensor;
extern Bounce wasWoodSuctionedSensor;
extern Bounce cutHomingSwitch;
extern Bounce positionHomingSwitch;

//* ************************************************************************
//* ************************ UNIFIED SENSOR READING FUNCTIONS ************
//* ************************************************************************

bool readSensor(SensorType sensor) {
    switch(sensor) {
        case WOOD_SENSOR_TYPE:
            // Wood sensor is active LOW with input pullup
            woodSensor.update();
            return woodSensor.read() == LOW;
        case WOOD_SUCTION_SENSOR_TYPE:
            // Wood suction sensor is active LOW with input pullup
            wasWoodSuctionedSensor.update();
            return wasWoodSuctionedSensor.read() == LOW;
        default:
            Serial.println("ERROR: Unknown sensor type for readSensor operation");
            return false;
    }
}

bool readLimitSwitch(SwitchType switchType) {
    switch(switchType) {
        case CUT_MOTOR_HOMING_SWITCH_TYPE:
            // Cut motor homing switch is active HIGH with input pulldown
            cutHomingSwitch.update();
            return cutHomingSwitch.read() == HIGH;
        case POSITION_MOTOR_HOMING_SWITCH_TYPE:
            // Position motor homing switch is active HIGH with input pulldown
            positionHomingSwitch.update();
            return positionHomingSwitch.read() == HIGH;
        default:
            Serial.println("ERROR: Unknown switch type for readLimitSwitch operation");
            return false;
    }
}

bool readHomeSwitch(SwitchType switchType) {
    // For now, home switches are the same as limit switches
    // This function can be expanded if separate home switches are added
    return readLimitSwitch(switchType);
}

//* ************************************************************************
//* ************************ LEGACY SENSOR READING FUNCTIONS *************
//* ************************************************************************
//! Legacy functions maintained for compatibility

bool readWoodSensor() {
    return readSensor(WOOD_SENSOR_TYPE);
}

bool readWoodSuctionSensor() {
    return readSensor(WOOD_SUCTION_SENSOR_TYPE);
}

bool readCutMotorHomingSwitch() {
    return readLimitSwitch(CUT_MOTOR_HOMING_SWITCH_TYPE);
}

bool readPositionMotorHomingSwitch() {
    return readLimitSwitch(POSITION_MOTOR_HOMING_SWITCH_TYPE);
} 