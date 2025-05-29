#include "StateMachine/StateMachine.h"
#include "Config/Config.h"
#include <AccelStepper.h>

// External variable declarations
extern AccelStepper positionMotor;
extern SystemState currentState;

//* ************************************************************************
//* ************************ PUSHWOODFORWARDONE FUNCTIONS ****************
//* ************************************************************************
//! PUSHWOODFORWARDONE-specific functions for manual wood advancement
//! These functions handle the precise sequence for manually pushing wood forward

//! ************************************************************************
//! PUSHWOODFORWARDONE STATE SEQUENCE:
//! ************************************************************************
//! STEP 1: RETRACT POSITION CLAMP (ONE TIME)
//!    - Retract position clamp to release wood
//!    - Prepare for manual wood advancement
//!
//! STEP 2: MOVE POSITION MOTOR TO TRAVEL POSITION (ONE TIME)
//!    - Extend position clamp for wood control
//!    - Retract wood secure clamp
//!    - Move position motor to advance position
//!    - Prepare for wood pushing sequence
//!
//! STEP 3: WAIT FOR POSITION MOTOR TO REACH TRAVEL
//!    - Run position motor until it reaches target
//!    - When complete: retract position clamp
//!    - Extend wood secure clamp for wood control
//!    - Transfer control to secure clamp
//!
//! STEP 4: WAIT 300MS (ONE TIME)
//!    - Allow mechanical settling time
//!    - Ensure proper clamp engagement
//!    - Stabilize wood position
//!
//! STEP 5: MOVE POSITION MOTOR TO ADVANCE POSITION (ONE TIME)
//!    - Move motor to POSITION_TRAVEL_DISTANCE - 0.1 inches
//!    - Advance wood to optimal position
//!    - Prepare for final positioning
//!
//! STEP 6: WAIT FOR ADVANCE, THEN SWAP TO POSITION CONTROL (ONE TIME)
//!    - Wait for motor to reach advance position
//!    - Retract position clamp
//!    - Extend wood secure clamp
//!    - Transfer control back to position clamp
//!
//! STEP 7: WAIT 50MS (ONE TIME)
//!    - Brief settling delay
//!    - Ensure clamp engagement
//!
//! STEP 8: MOVE TO FINAL TRAVEL POSITION (ONE TIME)
//!    - Move position motor to final travel position
//!    - Complete wood advancement sequence
//!
//! STEP 8.5: RUN MOTOR (CONTINUOUS)
//!    - Execute motor movement continuously
//!    - Maintain motion toward target
//!
//! STEP 9: WAIT FOR COMPLETION AND TRANSITION TO IDLE
//!    - Monitor position motor for completion
//!    - When motor reaches final position: transition to IDLE
//!    - Reset all state variables for next cycle
//!    - Wood advancement complete
//! ************************************************************************

//* ************************************************************************
//* ************************ CLAMP OPERATIONS FOR PUSHWOODFORWARDONE *****
//* ************************************************************************

void retractPositionClampForPushWood() {
    retractClampSimple(POSITION_CLAMP_ID);
    Serial.println("PUSHWOOD: Position clamp retracted");
}

void swapToSecureControlForPushWood() {
    // Use existing cylinder functions from 99_CYLINDER_FUNCTIONS.cpp
    extendClampSimple(POSITION_CLAMP_ID);
    Serial.println("PUSHWOOD: Position clamp extended for wood control");
    
    retractClampSimple(WOOD_SECURE_CLAMP_ID);
    Serial.println("PUSHWOOD: Secure wood clamp retracted - position clamp taking control");
}

void swapToPositionControlForPushWood() {
    // Use existing cylinder functions from 99_CYLINDER_FUNCTIONS.cpp
    retractClampSimple(POSITION_CLAMP_ID);
    Serial.println("PUSHWOOD: Position clamp retracted");
    
    extendClampSimple(WOOD_SECURE_CLAMP_ID);
    Serial.println("PUSHWOOD: Secure wood clamp extended - securing wood for final positioning");
}

//* ************************************************************************
//* ************************ MOTOR OPERATIONS FOR PUSHWOODFORWARDONE *****
//* ************************************************************************

void movePositionMotorToHomeForPushWood() {
    moveMotorTo(POSITION_MOTOR, 0, POSITION_MOTOR_NORMAL_SPEED);
    Serial.println("PUSHWOOD: Position motor moving to home position (0)");
}

void advancePositionMotorForPushWood() {
    // Move to POSITION_TRAVEL_DISTANCE - 0.1 inches
    float targetPosition = (POSITION_TRAVEL_DISTANCE - 0.1) * POSITION_MOTOR_STEPS_PER_INCH;
    moveMotorTo(POSITION_MOTOR, targetPosition, POSITION_MOTOR_NORMAL_SPEED);
    Serial.print("PUSHWOOD: Position motor moving to advance position: ");
    Serial.println(targetPosition);
}

void movePositionMotorToFinalForPushWood() {
    moveMotorTo(POSITION_MOTOR, POSITION_MOTOR_TRAVEL_POSITION, POSITION_MOTOR_NORMAL_SPEED);
    Serial.println("PUSHWOOD: Position motor moving to final travel position");
}

//* ************************************************************************
//* ************************ TIMING FUNCTIONS FOR PUSHWOODFORWARDONE *****
//* ************************************************************************

void waitForPushWoodSwapDelay() {
    delay(300);
    Serial.println("PUSHWOOD: 300ms delay completed");
}

void waitForPushWoodFinalDelay() {
    delay(50);
    Serial.println("PUSHWOOD: 50ms delay completed");
}

//* ************************************************************************
//* ************************ STATE TRANSITION FOR PUSHWOODFORWARDONE *****
//* ************************************************************************

void transitionFromPushWoodToIdle() {
    Serial.println("PUSHWOODFORWARDONE -> IDLE: Wood advancement complete - returning to idle");
    currentState = IDLE;
}

//* ************************************************************************
//* ************************ MAIN PUSHWOODFORWARDONE EXECUTION FUNCTION **
//* ************************************************************************

void executePushWoodForwardSequence() {
    static bool positionClampRetracted = false;
    static bool positionMotorToHome = false;
    static bool clampsSwappedToSecure = false;
    static bool swapDelayCompleted = false;
    static bool positionMotorAdvanced = false;
    static bool clampsSwappedToPosition = false;
    static bool finalDelayCompleted = false;
    static bool positionMotorToFinal = false;
    
    //! ************************************************************************
    //! STEP 1: RETRACT POSITION CLAMP (ONE TIME)
    //! ************************************************************************
    if (!positionClampRetracted) {
        retractPositionClampForPushWood();
        positionClampRetracted = true;
    }
    
    //! ************************************************************************
    //! STEP 2: MOVE POSITION MOTOR TO TRAVEL POSITION
    //! ************************************************************************
    if (positionClampRetracted && !positionMotorToHome) {
        extendClampSimple(POSITION_CLAMP_ID);
        Serial.println("PUSHWOODFORWARD: Position clamp extended");
        
        retractClampSimple(WOOD_SECURE_CLAMP_ID);
        Serial.println("PUSHWOODFORWARD: Wood secure clamp retracted");
        
        advancePositionMotorForPushWood();
        positionMotorToHome = true;
    }
    
    //! ************************************************************************
    //! STEP 3: WAIT FOR POSITION MOTOR TO REACH TRAVEL
    //! ************************************************************************
    if (positionMotorToHome && !clampsSwappedToSecure) {
        positionMotor.run();
        if (positionMotor.distanceToGo() == 0) {
            retractClampSimple(POSITION_CLAMP_ID);
            Serial.println("PUSHWOODFORWARD: Position clamp retracted when motor reaches travel");
            
            extendClampSimple(WOOD_SECURE_CLAMP_ID);
            Serial.println("PUSHWOODFORWARD: Wood secure clamp extended");
            clampsSwappedToSecure = true;
        }
    }
    
    //! ************************************************************************
    //! STEP 4: WAIT 300MS (ONE TIME)
    //! ************************************************************************
    if (clampsSwappedToSecure && !swapDelayCompleted) {
        waitForPushWoodSwapDelay();
        swapDelayCompleted = true;
    }
    
    //! ************************************************************************
    //! STEP 5: MOVE POSITION MOTOR TO ADVANCE POSITION (ONE TIME)
    //! ************************************************************************
    if (swapDelayCompleted && !positionMotorAdvanced) {
        advancePositionMotorForPushWood();
        positionMotorAdvanced = true;
    }
    
    //! ************************************************************************
    //! STEP 6: WAIT FOR ADVANCE, THEN SWAP TO POSITION CONTROL (ONE TIME)
    //! ************************************************************************
    if (positionMotorAdvanced && !clampsSwappedToPosition) {
        if (positionMotor.distanceToGo() == 0) {
            swapToPositionControlForPushWood();
            clampsSwappedToPosition = true;
        }
    }
    
    //! ************************************************************************
    //! STEP 7: WAIT 50MS (ONE TIME)
    //! ************************************************************************
    if (clampsSwappedToPosition && !finalDelayCompleted) {
        waitForPushWoodFinalDelay();
        finalDelayCompleted = true;
    }
    
    //! ************************************************************************
    //! STEP 8: MOVE TO FINAL TRAVEL POSITION (ONE TIME)
    //! ************************************************************************
    if (finalDelayCompleted && !positionMotorToFinal) {
        movePositionMotorToFinalForPushWood();
        positionMotorToFinal = true;
    }
    
    //! ************************************************************************
    //! STEP 8.5: RUN MOTOR TO ENSURE IT MOVES
    //! ************************************************************************
    positionMotor.run();
    
    //! ************************************************************************
    //! STEP 9: WAIT FOR COMPLETION AND TRANSITION TO IDLE
    //! ************************************************************************
    if (positionMotorToFinal) {
        if (positionMotor.distanceToGo() == 0) {
            Serial.println("PUSHWOOD: Position motor at final position - transitioning to IDLE");
            currentState = IDLE;
            
            // Reset state variables for next cycle
            positionClampRetracted = false;
            positionMotorToHome = false;
            clampsSwappedToSecure = false;
            swapDelayCompleted = false;
            positionMotorAdvanced = false;
            clampsSwappedToPosition = false;
            finalDelayCompleted = false;
            positionMotorToFinal = false;
        }
    }
} 