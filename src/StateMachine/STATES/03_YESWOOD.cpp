#include "StateMachine/StateMachine.h"
#include "Config/Config.h"
#include <AccelStepper.h>
#include <Bounce2.h>

// External variable declarations
extern AccelStepper cutMotor;
extern AccelStepper positionMotor;
extern Bounce cutHomingSwitch;
extern Bounce startCycleSwitch;
extern SystemState currentState;

//* ************************************************************************
//* ************************ YESWOOD FUNCTIONS ***************************
//* ************************************************************************
//! YESWOOD-specific functions for handling successful wood cutting
//! These functions manage the complex sequence of returning motors,
//! advancing wood position, and preparing for the next cycle

//! ************************************************************************
//! YESWOOD STATE SEQUENCE:
//! ************************************************************************
//! STEP 1: START CUT MOTOR RETURN (ONE TIME)
//!    - Set cut motor return speed
//!    - Move cut motor to home position (0)
//!    - Begin simultaneous return sequence
//!
//! STEP 2: RETRACT SECURE CLAMP (ONE TIME)
//!    - Retract wood secure clamp to release wood
//!    - Allow wood movement for advancement
//!
//! STEP 3: ADVANCE POSITION MOTOR (ONE TIME)
//!    - Move position motor to POSITION_TRAVEL_DISTANCE - 0.1 inches
//!    - Advance wood to optimal positioning
//!
//! STEP 4: SWAP CLAMP POSITIONS (WHEN POSITION MOTOR ADVANCE COMPLETE)
//!    - Extend wood secure clamp to hold wood
//!    - Retract position clamp for motor return
//!    - Transfer wood control between clamps
//!
//! STEP 5: START POSITION MOTOR RETURN (WHEN CLAMPS SWAPPED)
//!    - Move position motor back to home position (0)
//!    - Prepare for next positioning cycle
//!
//! STEP 6: EXTEND POSITION CLAMP (WHEN POSITION MOTOR AT HOME)
//!    - Extend position clamp when motor reaches home
//!    - Secure position for wood advancement
//!
//! STEP 6.5: VERIFY CUT MOTOR HOME (CONTINUOUS CHECK)
//!    - Verify cut motor at home position
//!    - Check cut homing switch for confirmation
//!    - Ensure motor position accuracy
//!
//! STEP 7: START FINAL ADVANCE (WHEN POSITION CLAMP EXTENDED AND CUT MOTOR VERIFIED)
//!    - Move position motor to final travel position
//!    - Complete wood positioning sequence
//!
//! STEP 8: RUN MOTORS (CONTINUOUS)
//!    - Execute motor movements continuously
//!    - Maintain motion toward targets
//!
//! STEP 9: CHECK CYCLE CONTINUATION (WHEN SEQUENCE COMPLETE)
//!    - Monitor start cycle switch state
//!    - If HIGH: transition to CUTTING for next cycle
//!    - If LOW: transition to IDLE state
//!    - Reset all state variables
//! ************************************************************************

//* ************************************************************************
//* ************************ MOTOR RETURN OPERATIONS FOR YESWOOD *********
//* ************************************************************************

void returnCutMotorToHomeForYeswood() {
    moveCutMotorToHome();
    Serial.println("YESWOOD: Cut motor returning to home position");
}

void retractSecureClampForYeswood() {
    retractClamp(WOOD_SECURE_CLAMP_ENUM);
    Serial.println("YESWOOD: Secure wood clamp retracted");
}

void advancePositionMotorForYeswood() {
    // Move to POSITION_TRAVEL_DISTANCE - 0.1 inches
    float targetPosition = (POSITION_TRAVEL_DISTANCE - 0.1) * POSITION_MOTOR_STEPS_PER_INCH;
    moveMotorTo(POSITION_MOTOR, targetPosition, POSITION_MOTOR_NORMAL_SPEED);
    Serial.print("YESWOOD: Position motor moving to advance position: ");
    Serial.println(targetPosition);
}

//* ************************************************************************
//* ************************ CLAMP SWAP OPERATIONS FOR YESWOOD ***********
//* ************************************************************************

void swapClampPositionsForYeswood() {
    // Use existing cylinder functions from 99_CYLINDER_FUNCTIONS.cpp
    extendClamp(WOOD_SECURE_CLAMP_ENUM);
    Serial.println("YESWOOD: Secure wood clamp extended for wood transfer");
    
    retractClamp(POSITION_CLAMP_ENUM);
    Serial.println("YESWOOD: Position clamp retracted for wood transfer");
    
    Serial.println("YESWOOD: Clamp positions swapped for wood advancement");
}

void returnPositionMotorToHomeForYeswood() {
    moveMotorTo(POSITION_MOTOR, 0, POSITION_MOTOR_RETURN_SPEED);
    Serial.println("YESWOOD: Position motor returning to home position");
}

void extendPositionClampWhenHomeForYeswood() {
    if (positionMotor.distanceToGo() == 0 && positionMotor.currentPosition() == 0) {
        extendClamp(POSITION_CLAMP_ENUM);
        Serial.println("YESWOOD: Position clamp extended - position motor at home");
    }
}

//* ************************************************************************
//* ************************ CUT MOTOR HOME VERIFICATION FOR YESWOOD *****
//* ************************************************************************

bool checkCutMotorHomeAndSensorForYeswood() {
    // Check if cut motor is at home position
    if (cutMotor.distanceToGo() == 0 && cutMotor.currentPosition() == 0) {
        // Wait 10ms then check homing sensor
        delay(10);
        if (readLimitSwitch(CUT_MOTOR_HOMING_SWITCH_TYPE)) {
            Serial.println("YESWOOD: Cut motor confirmed at home position");
            return true;
        } else {
            Serial.println("YESWOOD: WARNING - Cut motor reports home but sensor disagrees");
            return false;
        }
    }
    return false;
}

void advancePositionMotorToTravelForYeswood() {
    moveMotorTo(POSITION_MOTOR, POSITION_MOTOR_TRAVEL_POSITION, POSITION_MOTOR_NORMAL_SPEED);
    Serial.println("YESWOOD: Position motor advancing to travel position for next cycle");
}

//* ************************************************************************
//* ************************ CYCLE CONTINUATION CHECK FOR YESWOOD ********
//* ************************************************************************

bool checkRunCycleSwitchForYeswood() {
    startCycleSwitch.update();
    if (startCycleSwitch.read() == HIGH) {
        Serial.println("YESWOOD: Run cycle switch HIGH - continuing to CUTTING");
        currentState = CUTTING;
        return true;
    } else {
        Serial.println("YESWOOD: Run cycle switch not HIGH - returning to IDLE");
        currentState = IDLE;
        return true;
    }
}

//* ************************************************************************
//* ************************ MAIN YESWOOD EXECUTION FUNCTION *************
//* ************************************************************************

void executeYeswoodSequence() {
    static bool cutMotorReturnStarted = false;
    static bool secureClampRetracted = false;
    static bool positionMotorAdvanced = false;
    static bool clampsSwapped = false;
    static bool positionMotorHomeStarted = false;
    static bool positionClampExtended = false;
    static bool cutMotorHomeVerified = false;
    static bool finalAdvanceStarted = false;
    
    //! ************************************************************************
    //! STEP 1: START CUT MOTOR RETURN (ONE TIME)
    //! ************************************************************************
    if (!cutMotorReturnStarted) {
        returnCutMotorToHomeForYeswood();
        cutMotorReturnStarted = true;
    }
    
    //! ************************************************************************
    //! STEP 2: RETRACT SECURE CLAMP (ONE TIME)
    //! ************************************************************************
    if (!secureClampRetracted) {
        retractSecureClampForYeswood();
        secureClampRetracted = true;
    }
    
    //! ************************************************************************
    //! STEP 3: ADVANCE POSITION MOTOR (ONE TIME)
    //! ************************************************************************
    if (!positionMotorAdvanced) {
        advancePositionMotorForYeswood();
        positionMotorAdvanced = true;
    }
    
    //! ************************************************************************
    //! STEP 4: SWAP CLAMP POSITIONS WHEN POSITION MOTOR ADVANCE COMPLETE
    //! ************************************************************************
    if (positionMotorAdvanced && !clampsSwapped) {
        if (positionMotor.distanceToGo() == 0) {
            swapClampPositionsForYeswood();
            clampsSwapped = true;
        }
    }
    
    //! ************************************************************************
    //! STEP 5: START POSITION MOTOR RETURN WHEN CLAMPS SWAPPED
    //! ************************************************************************
    if (clampsSwapped && !positionMotorHomeStarted) {
        returnPositionMotorToHomeForYeswood();
        positionMotorHomeStarted = true;
    }
    
    //! ************************************************************************
    //! STEP 6: EXTEND POSITION CLAMP WHEN POSITION MOTOR AT HOME
    //! ************************************************************************
    if (positionMotorHomeStarted && !positionClampExtended) {
        extendPositionClampWhenHomeForYeswood();
        if (positionMotor.distanceToGo() == 0 && positionMotor.currentPosition() == 0) {
            positionClampExtended = true;
        }
    }
    
    //! ************************************************************************
    //! STEP 6.5: VERIFY CUT MOTOR HOME (CONTINUOUS CHECK)
    //! ************************************************************************
    if (cutMotorReturnStarted && !cutMotorHomeVerified) {
        cutMotorHomeVerified = checkCutMotorHomeAndSensorForYeswood();
    }
    
    //! ************************************************************************
    //! STEP 7: START FINAL ADVANCE WHEN POSITION CLAMP EXTENDED AND CUT MOTOR HOME VERIFIED
    //! ************************************************************************
    if (positionClampExtended && cutMotorHomeVerified && !finalAdvanceStarted) {
        advancePositionMotorToTravelForYeswood();
        finalAdvanceStarted = true;
    }
    
    //! ************************************************************************
    //! STEP 8: RUN MOTORS TO ENSURE THEY MOVE
    //! ************************************************************************
    cutMotor.run();
    positionMotor.run();
    
    //! ************************************************************************
    //! STEP 9: CHECK FOR CYCLE CONTINUATION
    //! ************************************************************************
    if (finalAdvanceStarted && positionMotor.distanceToGo() == 0) {
        checkRunCycleSwitchForYeswood();
        
        // Reset state variables for next cycle
        cutMotorReturnStarted = false;
        secureClampRetracted = false;
        positionMotorAdvanced = false;
        clampsSwapped = false;
        cutMotorHomeVerified = false;
        positionMotorHomeStarted = false;
        positionClampExtended = false;
        finalAdvanceStarted = false;
    }
}

void reactivateSecureClampForYeswood() {
    extendClamp(WOOD_SECURE_CLAMP_ENUM);
    Serial.println("YESWOOD: Secure wood clamp re-extended");
    
    retractClamp(POSITION_CLAMP_ENUM);
    Serial.println("YESWOOD: Position clamp retracted");
}

void setFinalClampStateForYeswood() {
    extendClamp(POSITION_CLAMP_ENUM);
    Serial.println("YESWOOD: Position clamp extended - final operational state");
} 