#include "StateMachine/StateMachine.h"
#include "Config/Config.h"
#include <Bounce2.h>

// External variable declarations
extern Bounce reloadSwitch;
extern SystemState currentState;
extern bool isReloadMode;

//* ************************************************************************
//* ************************ RELOAD FUNCTIONS ***************************
//* ************************************************************************
//! RELOAD-specific functions for safe manual wood loading/unloading
//! These functions provide a safe mode for manual access to the machine

//* ************************************************************************
//* ************************ CLAMP OPERATIONS FOR RELOAD *****************
//* ************************************************************************

void retractAllClampsForReload() {
    // Use existing cylinder functions
    retractClampSimple(POSITION_CLAMP_ID);
    retractClampSimple(WOOD_SECURE_CLAMP_ID);
    retractClampSimple(CATCHER_CLAMP_ID);
    Serial.println("RELOAD: All clamps retracted");
}

void setOperationalClampsForReload() {
    // Use existing cylinder functions 
    extendClampSimple(POSITION_CLAMP_ID);
    extendClampSimple(WOOD_SECURE_CLAMP_ID);
    Serial.println("RELOAD: Operational clamps set (position and wood secure extended)");
}

//* ************************************************************************
//* ************************ RELOAD MODE MANAGEMENT **********************
//* ************************************************************************

void enterReloadMode() {
    isReloadMode = true;
    Serial.println("RELOAD: Entering reload mode - safe for manual wood handling");
}

void exitReloadMode() {
    isReloadMode = false;
    Serial.println("RELOAD: Exiting reload mode - returning to operational state");
}

//* ************************************************************************
//* ************************ SWITCH MONITORING FOR RELOAD ****************
//* ************************************************************************

bool checkReloadSwitchForExit() {
    reloadSwitch.update();
    if (reloadSwitch.read() == LOW) {
        Serial.println("RELOAD: Reload switch turned OFF - preparing to exit reload mode");
        return true;
    }
    return false;
}

//* ************************************************************************
//* ************************ STATE TRANSITION FOR RELOAD *****************
//* ************************************************************************

void transitionFromReloadToIdle() {
    Serial.println("RELOAD -> IDLE: Reload complete - returning to idle state");
    currentState = IDLE;
}

//* ************************************************************************
//* ************************ MAIN RELOAD EXECUTION FUNCTION **************
//* ************************************************************************

void executeReloadSequence() {
    static bool clampsRetracted = false;
    static bool reloadModeSet = false;
    static bool exitConditionMet = false;
    static bool clampsReengaged = false;
    
    //! ************************************************************************
    //! STEP 1: RETRACT ALL CLAMPS FOR SAFE ACCESS (ONE TIME)
    //! ************************************************************************
    if (!clampsRetracted) {
        retractAllClampsForReload();
        clampsRetracted = true;
    }
    
    //! ************************************************************************
    //! STEP 2: SET RELOAD MODE FLAG (ONE TIME)
    //! ************************************************************************
    if (clampsRetracted && !reloadModeSet) {
        enterReloadMode();
        reloadModeSet = true;
    }
    
    //! ************************************************************************
    //! STEP 3: MONITOR RELOAD SWITCH FOR EXIT CONDITION (CONTINUOUS)
    //! ************************************************************************
    if (reloadModeSet && !exitConditionMet) {
        if (checkReloadSwitchForExit()) {
            exitConditionMet = true;
        }
    }
    
    //! ************************************************************************
    //! STEP 4: RE-ENGAGE CLAMPS AND EXIT RELOAD MODE (ONE TIME)
    //! ************************************************************************
    if (exitConditionMet && !clampsReengaged) {
        exitReloadMode();
        setOperationalClampsForReload();
        clampsReengaged = true;
    }
    
    //! ************************************************************************
    //! STEP 5: TRANSITION TO IDLE STATE (ONE TIME)
    //! ************************************************************************
    if (clampsReengaged) {
        transitionFromReloadToIdle();
        
        // Reset state variables for next cycle
        clampsRetracted = false;
        reloadModeSet = false;
        exitConditionMet = false;
        clampsReengaged = false;
    }
} 