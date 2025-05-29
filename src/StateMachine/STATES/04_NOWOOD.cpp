#include "StateMachine/StateMachine.h"
#include "Config/Config.h"
#include <AccelStepper.h>
#include <Bounce2.h>

// External variable declarations
extern AccelStepper cutMotor;
extern AccelStepper positionMotor;
extern SystemState currentState;

//* ************************************************************************
//* ************************ NOWOOD FUNCTIONS ****************************
//* ************************************************************************
//! NOWOOD-specific functions for handling when no wood is detected
//! These functions handle the sequence when wood is not caught by the catcher

//! ************************************************************************
//! NOWOOD STATE SEQUENCE:
//! ************************************************************************
//! STEP 1: RETRACT SECURE CLAMP (ONE TIME)
//!    - Retract wood secure clamp immediately
//!    - Release any remaining wood fragments
//!    - Prepare for system reset
//!
//! STEP 2: MOVE POSITION MOTOR TO -1 (ONE TIME)
//!    - Move position motor to -1 position (negative 1 step)
//!    - Clear any mechanical interference
//!    - Prepare for clamp reset sequence
//!
//! STEP 3: START CUT MOTOR RETURN (ONE TIME)
//!    - Set cut motor return speed
//!    - Move cut motor back to home position (0)
//!    - Begin motor return sequence
//!
//! STEP 4: RESET CLAMPS WHEN POSITION MOTOR AT -1
//!    - Wait for position motor to reach -1
//!    - Retract position clamp (reset sequence)
//!    - Extend position clamp (restore operational state)
//!    - Ensure proper clamp positioning
//!
//! STEP 5: MOVE POSITION MOTOR TO TRAVEL POSITION (ONE TIME)
//!    - Move position motor to travel position
//!    - Restore operational positioning
//!    - Prepare for next cycle
//!
//! STEP 6: RUN MOTORS (CONTINUOUS)
//!    - Execute motor movements continuously
//!    - Maintain motion toward targets
//!
//! STEP 7: CHECK FOR COMPLETION AND TRANSITION TO IDLE
//!    - Monitor both motors for completion
//!    - When both motors reach targets: transition to IDLE
//!    - Reset all state variables for next cycle
//!    - System ready for new operation
//! ************************************************************************

//* ************************************************************************
//* ************************ CLAMP OPERATIONS FOR NOWOOD *****************
//* ************************************************************************

void retractSecureClampForNowood() {
    retractClampSimple(WOOD_SECURE_CLAMP_ID);
    Serial.println("NOWOOD: Secure wood clamp retracted");
}

void resetClampPositionsForNowood() {
    // Use existing cylinder functions from 99_CYLINDER_FUNCTIONS.cpp
    retractClampSimple(POSITION_CLAMP_ID);
    Serial.println("NOWOOD: Position clamp retracted");
    
    extendClampSimple(POSITION_CLAMP_ID);
    Serial.println("NOWOOD: Position clamp extended - reset to operational position");
}

//* ************************************************************************
//* ************************ MOTOR OPERATIONS FOR NOWOOD *****************
//* ************************************************************************

void movePositionMotorToNegOneForNowood() {
    // Move to -1 position (negative 1 step)
    moveMotorTo(POSITION_MOTOR, -1, POSITION_MOTOR_NORMAL_SPEED);
    Serial.println("NOWOOD: Position motor moving to -1 position");
}

void returnCutMotorToHomeForNowood() {
    moveMotorTo(CUT_MOTOR, 0, CUT_MOTOR_RETURN_SPEED);
    Serial.println("NOWOOD: Cut motor returning to home position");
}

void advancePositionMotorToTravelForNowood() {
    moveMotorTo(POSITION_MOTOR, POSITION_MOTOR_TRAVEL_POSITION, POSITION_MOTOR_NORMAL_SPEED);
    Serial.println("NOWOOD: Position motor moving to travel position");
}

//* ************************************************************************
//* ************************ STATE TRANSITION FOR NOWOOD *****************
//* ************************************************************************

void transitionFromNowoodToIdle() {
    Serial.println("NOWOOD -> IDLE: Returning to idle state - ready for next cycle");
    currentState = IDLE;
}

//* ************************************************************************
//* ************************ MAIN NOWOOD EXECUTION FUNCTION **************
//* ************************************************************************

void executeNowoodSequence() {
    static bool secureClampRetracted = false;
    static bool positionMotorToNegOne = false;
    static bool cutMotorReturnStarted = false;
    static bool clampsReset = false;
    static bool positionMotorToTravel = false;
    
    //! ************************************************************************
    //! STEP 1: RETRACT SECURE CLAMP (ONE TIME)
    //! ************************************************************************
    if (!secureClampRetracted) {
        retractSecureClampForNowood();
        secureClampRetracted = true;
    }
    
    //! ************************************************************************
    //! STEP 2: MOVE POSITION MOTOR TO -1 (ONE TIME)
    //! ************************************************************************
    if (!positionMotorToNegOne) {
        movePositionMotorToNegOneForNowood();
        positionMotorToNegOne = true;
    }
    
    //! ************************************************************************
    //! STEP 3: START CUT MOTOR RETURN (ONE TIME)
    //! ************************************************************************
    if (!cutMotorReturnStarted) {
        returnCutMotorToHomeForNowood();
        cutMotorReturnStarted = true;
    }
    
    //! ************************************************************************
    //! STEP 4: RESET CLAMPS WHEN POSITION MOTOR AT -1
    //! ************************************************************************
    if (positionMotorToNegOne && !clampsReset) {
        if (positionMotor.distanceToGo() == 0) {
            resetClampPositionsForNowood();
            clampsReset = true;
        }
    }
    
    //! ************************************************************************
    //! STEP 5: MOVE POSITION MOTOR TO TRAVEL POSITION (ONE TIME)
    //! ************************************************************************
    if (clampsReset && !positionMotorToTravel) {
        advancePositionMotorToTravelForNowood();
        positionMotorToTravel = true;
    }
    
    //! ************************************************************************
    //! STEP 6: RUN MOTORS TO ENSURE THEY MOVE
    //! ************************************************************************
    cutMotor.run();
    positionMotor.run();
    
    //! ************************************************************************
    //! STEP 7: CHECK FOR COMPLETION AND TRANSITION TO IDLE
    //! ************************************************************************
    if (positionMotorToTravel && cutMotorReturnStarted) {
        bool cutMotorDone = (cutMotor.distanceToGo() == 0);
        bool positionMotorDone = (positionMotor.distanceToGo() == 0);
        
        if (cutMotorDone && positionMotorDone) {
            Serial.println("NOWOOD: Both motors complete - transitioning to IDLE");
            currentState = IDLE;
            
            // Reset state variables for next cycle
            positionMotorToNegOne = false;
            clampsReset = false;
            cutMotorReturnStarted = false;
            positionMotorToTravel = false;
        }
    }
} 