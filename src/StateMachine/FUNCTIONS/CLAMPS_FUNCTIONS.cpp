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
//* ************************ INDIVIDUAL CLAMP FUNCTIONS ******************
//* ************************************************************************
//! Individual functions for each clamp - PREFERRED USAGE

// Position Clamp Functions
void extendPositionClamp() {
    digitalWrite(POSITION_CLAMP, LOW);
    Serial.println("Position clamp extended");
}

void retractPositionClamp() {
    digitalWrite(POSITION_CLAMP, HIGH);
    Serial.println("Position clamp retracted");
}

// Wood Secure Clamp Functions
void extendWoodSecureClamp() {
    digitalWrite(WOOD_SECURE_CLAMP, LOW);
    Serial.println("Wood secure clamp extended");
}

void retractWoodSecureClamp() {
    digitalWrite(WOOD_SECURE_CLAMP, HIGH);
    Serial.println("Wood secure clamp retracted");
}

// Catcher Clamp Functions
void extendCatcherClamp() {
    digitalWrite(CATCHER_CLAMP_PIN, LOW);
    catcherClampEngageTime = millis();
    catcherClampIsEngaged = true;
    Serial.println("Catcher clamp extended");
}

void retractCatcherClamp() {
    digitalWrite(CATCHER_CLAMP_PIN, HIGH);
    catcherClampIsEngaged = false;
    Serial.println("Catcher clamp retracted");
}

//* ************************************************************************
//* ************************ LEGACY PNEUMATIC CLAMP FUNCTIONS ************
//* ************************************************************************
//! Legacy enum-based functions - KEPT FOR COMPATIBILITY

// Internal enum-based functions
void extendClamp(ClampType clamp) {
    switch(clamp) {
        case POSITION_CLAMP_ENUM:
            extendPositionClamp();
            break;
        case WOOD_SECURE_CLAMP_ENUM:
            extendWoodSecureClamp();
            break;
        case CATCHER_CLAMP_ENUM:
            extendCatcherClamp();
            break;
        default:
            Serial.println("ERROR: Unknown clamp type for extend operation");
            break;
    }
}

void retractClamp(ClampType clamp) {
    switch(clamp) {
        case POSITION_CLAMP_ENUM:
            retractPositionClamp();
            break;
        case WOOD_SECURE_CLAMP_ENUM:
            retractWoodSecureClamp();
            break;
        case CATCHER_CLAMP_ENUM:
            retractCatcherClamp();
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
            extendPositionClamp();
            break;
        case WOOD_SECURE_CLAMP_ID:
            extendWoodSecureClamp();
            break;
        case CATCHER_CLAMP_ID:
            extendCatcherClamp();
            break;
        default:
            Serial.println("ERROR: Unknown clamp ID for extend operation");
            break;
    }
}

void retractClampSimple(int clampId) {
    switch(clampId) {
        case POSITION_CLAMP_ID:
            retractPositionClamp();
            break;
        case WOOD_SECURE_CLAMP_ID:
            retractWoodSecureClamp();
            break;
        case CATCHER_CLAMP_ID:
            retractCatcherClamp();
            break;
        default:
            Serial.println("ERROR: Unknown clamp ID for retract operation");
            break;
    }
}

// Collective Operations
void retractAllCylinders() {
    retractPositionClamp();
    retractWoodSecureClamp();
    retractCatcherClamp();
    Serial.println("All cylinders retracted");
}

void extendAllCylinders() {
    extendPositionClamp();
    extendWoodSecureClamp();
    extendCatcherClamp();
    Serial.println("All cylinders extended");
} 