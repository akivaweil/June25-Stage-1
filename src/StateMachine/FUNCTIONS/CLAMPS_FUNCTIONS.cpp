#include "StateMachine/StateMachine.h"

// External variable declarations for catcher clamp timing
extern unsigned long catcherClampEngageTime;
extern bool catcherClampIsEngaged;

//* ************************************************************************
//* ************************ CLAMPS FUNCTIONS ****************************
//* ************************************************************************
//! Pneumatic clamp control functions
//! Basic pneumatic operations that can be shared across states
//! THIS FILE CONTAINS ONLY CLAMP-RELATED FUNCTIONS

// Simple clamp identifiers for state files
#define POSITION_CLAMP_ID 0
#define WOOD_SECURE_CLAMP_ID 1
#define CATCHER_CLAMP_ID 2

//* ************************************************************************
//* ************************ PNEUMATIC CLAMP FUNCTIONS *******************
//* ************************************************************************

// Internal enum-based functions
void extendClamp(ClampType clamp) {
    switch(clamp) {
        case POSITION_CLAMP_ENUM:
            digitalWrite(POSITION_CLAMP, LOW);
            Serial.println("Position clamp extended");
            break;
        case WOOD_SECURE_CLAMP_ENUM:
            digitalWrite(WOOD_SECURE_CLAMP, LOW);
            Serial.println("Wood secure clamp extended");
            break;
        case CATCHER_CLAMP_ENUM:
            digitalWrite(CATCHER_CLAMP_PIN, LOW);
            catcherClampEngageTime = millis();
            catcherClampIsEngaged = true;
            Serial.println("Catcher clamp extended");
            break;
        default:
            Serial.println("ERROR: Unknown clamp type for extend operation");
            break;
    }
}

void retractClamp(ClampType clamp) {
    switch(clamp) {
        case POSITION_CLAMP_ENUM:
            digitalWrite(POSITION_CLAMP, HIGH);
            Serial.println("Position clamp retracted");
            break;
        case WOOD_SECURE_CLAMP_ENUM:
            digitalWrite(WOOD_SECURE_CLAMP, HIGH);
            Serial.println("Wood secure clamp retracted");
            break;
        case CATCHER_CLAMP_ENUM:
            digitalWrite(CATCHER_CLAMP_PIN, HIGH);
            catcherClampIsEngaged = false;
            Serial.println("Catcher clamp retracted");
            break;
        default:
            Serial.println("ERROR: Unknown clamp type for retract operation");
            break;
    }
}

// Simple wrapper functions for state files to use
void extendClampSimple(int clampId) {
    switch(clampId) {
        case POSITION_CLAMP_ID:
            extendClamp(POSITION_CLAMP_ENUM);
            break;
        case WOOD_SECURE_CLAMP_ID:
            extendClamp(WOOD_SECURE_CLAMP_ENUM);
            break;
        case CATCHER_CLAMP_ID:
            extendClamp(CATCHER_CLAMP_ENUM);
            break;
        default:
            Serial.println("ERROR: Unknown clamp ID for extend operation");
            break;
    }
}

void retractClampSimple(int clampId) {
    switch(clampId) {
        case POSITION_CLAMP_ID:
            retractClamp(POSITION_CLAMP_ENUM);
            break;
        case WOOD_SECURE_CLAMP_ID:
            retractClamp(WOOD_SECURE_CLAMP_ENUM);
            break;
        case CATCHER_CLAMP_ID:
            retractClamp(CATCHER_CLAMP_ENUM);
            break;
        default:
            Serial.println("ERROR: Unknown clamp ID for retract operation");
            break;
    }
}

// Collective Operations
void retractAllCylinders() {
    retractClamp(POSITION_CLAMP_ENUM);
    retractClamp(WOOD_SECURE_CLAMP_ENUM);
    retractClamp(CATCHER_CLAMP_ENUM);
    Serial.println("All cylinders retracted");
}

void extendAllCylinders() {
    extendClamp(POSITION_CLAMP_ENUM);
    extendClamp(WOOD_SECURE_CLAMP_ENUM);
    extendClamp(CATCHER_CLAMP_ENUM);
    Serial.println("All cylinders extended");
} 