#include "StateMachine/StateMachine.h"
#include "Config/Config.h"
#include <AccelStepper.h>
#include <Bounce2.h>

// External variable declarations
extern AccelStepper cutMotor;
extern AccelStepper positionMotor;
extern Bounce woodSensor;
extern Bounce wasWoodSuctionedSensor;
extern SystemState currentState;

//* ************************************************************************
//* ************************ CUTTING FUNCTIONS ***************************
//* ************************************************************************
//! CUTTING-specific functions for wood cutting operations
//! These functions handle all aspects of the cutting sequence with clear,
//! readable implementations specific to the CUTTING state

//! ************************************************************************
//! CUTTING STATE SEQUENCE:
//! ************************************************************************
//! STEP 1: EXTEND BOTH CLAMPS (ONE TIME)
//!    - Extend position clamp to secure wood position
//!    - Extend wood secure clamp to hold wood firmly
//!    - Both clamps activated simultaneously
//!
//! STEP 2: START CUT MOTOR MOVEMENT (ONE TIME)
//!    - Set cut motor speed to CUT_MOTOR_CUTTING_SPEED
//!    - Move cut motor to CUT_MOTOR_CUT_POSITION
//!    - Begin cutting sequence
//!
//! STEP 3: CONTINUOUS SAFETY AND ACTIVATION CHECKS
//!    - Run cut motor continuously toward target
//!    - Safety check at 0.3 inches: monitor wasWoodSuctionedSensor
//!    - If safety violation: stop motor and enter error state
//!    - Catcher clamp activation at early offset position
//!    - Catcher servo activation at early offset position
//!
//! STEP 4: CHECK CUT COMPLETION AND ROUTE TO NEXT STATE
//!    - Monitor cut motor distance to go
//!    - When cut complete: check wood sensor state
//!    - If wood detected (LOW): transition to YESWOOD state
//!    - If no wood detected (HIGH): transition to NOWOOD state
//!    - Reset all state variables for next cycle
//! ************************************************************************

//* ************************************************************************
//* ************************ CLAMP OPERATIONS FOR CUTTING ****************
//* ************************************************************************

void activateClampingForCutting() {
    // Use individual clamp functions
    extendPositionClamp();
    extendWoodSecureClamp();
    Serial.println("CUTTING: Position and wood secure clamps activated");
}

//* ************************************************************************
//* ************************ MOTOR OPERATIONS FOR CUTTING ****************
//* ************************************************************************

void startCutMotorMovementForCutting() {
    moveMotorTo(CUT_MOTOR, CUT_MOTOR_CUT_POSITION, CUT_MOTOR_CUTTING_SPEED);
    Serial.print("CUTTING: Cut motor started - moving to position ");
    Serial.println(CUT_MOTOR_CUT_POSITION);
}

bool checkCutMotorSafetyAt03Inches() {
    // Calculate 0.3 inch position
    float safetyCheckPosition = 0.3 * CUT_MOTOR_STEPS_PER_INCH;
    
    // Check if motor has reached 0.3 inches
    if (cutMotor.currentPosition() >= safetyCheckPosition) {
        if (readSensor(WOOD_SUCTION_SENSOR_TYPE)) {
            Serial.println("CUTTING: SAFETY VIOLATION - Wood suctioned sensor activated at 0.3 inches");
            cutMotor.stop();
            // TODO: Enter waswoodsuctioned error state
            return false;
        }
        Serial.println("CUTTING: Safety check passed at 0.3 inches");
    }
    return true;
}

bool checkCatcherClampActivationPoint() {
    // Calculate activation point
    float activationPosition = CUT_MOTOR_CUT_POSITION - (CATCHER_CLAMP_EARLY_ACTIVATION_OFFSET_INCHES * CUT_MOTOR_STEPS_PER_INCH);
    
    if (cutMotor.currentPosition() >= activationPosition) {
        // Use individual clamp function
        extendCatcherClamp();
        Serial.println("CUTTING: Catcher clamp activated at early activation offset");
        return true;
    }
    return false;
}

bool checkCatcherServoActivationPoint() {
    // Calculate activation point
    float activationPosition = CUT_MOTOR_CUT_POSITION - (CATCHER_SERVO_EARLY_ACTIVATION_OFFSET_INCHES * CUT_MOTOR_STEPS_PER_INCH);
    
    if (cutMotor.currentPosition() >= activationPosition) {
        // Activate catcher servo
        // TODO: Add servo activation code
        Serial.println("CUTTING: Catcher servo activated at early activation offset");
        return true;
    }
    return false;
}

//* ************************************************************************
//* ************************ WOOD SENSOR CHECKING FOR CUTTING ************
//* ************************************************************************

bool checkWoodSensorForStateTransition() {
    bool sensorReading = readSensor(WOOD_SENSOR_TYPE);
    Serial.print("CUTTING: Wood sensor reading: ");
    Serial.print(sensorReading ? "DETECTED (LOW)" : "NOT DETECTED (HIGH)");
    Serial.print(" - Raw pin reading: ");
    woodSensor.update();
    Serial.println(woodSensor.read());
    
    if (sensorReading) {
        Serial.println("CUTTING: Wood detected - transitioning to YESWOOD");
        currentState = YESWOOD;
        return true;
    } else {
        Serial.println("CUTTING: No wood detected - transitioning to NOWOOD");
        currentState = NOWOOD;
        return true;
    }
}

//* ************************************************************************
//* ************************ MAIN CUTTING EXECUTION FUNCTION *************
//* ************************************************************************

void executeCuttingSequence() {
    static bool clampsExtended = false;
    static bool cutMotorStarted = false;
    static bool safetyChecked = false;
    static bool catcherClampActivated = false;
    static bool catcherServoActivated = false;
    
    //! ************************************************************************
    //! STEP 1: EXTEND BOTH CLAMPS (ONE TIME)
    //! ************************************************************************
    if (!clampsExtended) {
        activateClampingForCutting();
        clampsExtended = true;
    }
    
    //! ************************************************************************
    //! STEP 2: START CUT MOTOR MOVEMENT (ONE TIME)
    //! ************************************************************************
    if (!cutMotorStarted) {
        startCutMotorMovementForCutting();
        cutMotorStarted = true;
    }
    
    //! ************************************************************************
    //! STEP 3: CONTINUOUS SAFETY AND ACTIVATION CHECKS
    //! ************************************************************************
    if (cutMotor.distanceToGo() != 0) {
        cutMotor.run(); // Make the motor move
        
        // Safety check at 0.3 inches
        if (!safetyChecked) {
            if (!checkCutMotorSafetyAt03Inches()) {
                // Reset state variables on error
                clampsExtended = false;
                cutMotorStarted = false;
                safetyChecked = false;
                return;
            }
            safetyChecked = true;
        }
        
        // Catcher clamp activation check
        if (!catcherClampActivated) {
            catcherClampActivated = checkCatcherClampActivationPoint();
        }
        
        // Catcher servo activation check
        if (!catcherServoActivated) {
            catcherServoActivated = checkCatcherServoActivationPoint();
        }
    }
    
    //! ************************************************************************
    //! STEP 4: CHECK IF CUT IS COMPLETE AND ROUTE TO NEXT STATE
    //! ************************************************************************
    if (cutMotor.distanceToGo() == 0) {
        Serial.println("CUTTING: Cut motor movement complete - checking wood sensor");
        checkWoodSensorForStateTransition();
        
        // Reset state variables for next cycle
        clampsExtended = false;
        cutMotorStarted = false;
        safetyChecked = false;
        catcherClampActivated = false;
        catcherServoActivated = false;
    }
}