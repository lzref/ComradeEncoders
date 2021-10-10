#include "InfinitePot.h"
#include "Arduino.h"

void InfinitePot::setup(int valuePotA, int valuePotB)
{
    _previousValuePotA = valuePotA;
    _previousValuePotB = valuePotB;
}

int InfinitePot::update(int valuePotA, int valuePotB)
{
    int direction;
    int dirPotA;
    int dirPotB;
    int delta = 0;

    /****************************************************************************
     * Step 1 decode each  individual pot tap's direction
     ****************************************************************************/
    // First check direction for Tap A
    if (valuePotA > (_previousValuePotA + _adcHysteresis)) { // check if new reading is higher (by <debounce value>), if so...
        dirPotA = 1; // ...direction of tap A is up
    } else if (valuePotA < (_previousValuePotA - _adcHysteresis)) { // check if new reading is lower (by <debounce value>), if so...
        dirPotA = -1; // ...direction of tap A is down
    } else {
        dirPotA = 0; // No change
    }

    // then check direction for tap B
    if (valuePotB > (_previousValuePotB + _adcHysteresis)) { // check if new reading is higher (by <debounce value>), if so...
        dirPotB = 1; // ...direction of tap B is up
    } else if (valuePotB < (_previousValuePotB - _adcHysteresis)) { // check if new reading is lower (by <debounce value>), if so...
        dirPotB = -1; // ...direction of tap B is down
    } else {
        dirPotB = 0; // No change
    }

    /****************************************************************************
     * Step 2: Determine actual direction of ENCODER based on each individual
     * potentiometer tap´s direction and the phase
     ****************************************************************************/
    if (dirPotA == -1 and dirPotB == -1) { //If direction of both taps is down
        if (valuePotA > valuePotB) { // If value A above value B...
            direction = 1; // ...direction is up
        } else {
            direction = -1; // otherwise direction is down
        }
    } else if (dirPotA == 1 and dirPotB == 1) { //If direction of both taps is up
        if (valuePotA < valuePotB) { // If value A below value B...
            direction = 1; // ...direction is up
        } else {
            direction = -1; // otherwise direction is down
        }
    } else if (dirPotA == 1 and dirPotB == -1) { // If A is up and B is down
        if ((valuePotA > (_adcMaxValue / 2)) || (valuePotB > (_adcMaxValue / 2))) { //If either pot at upper range A/B = up/down means direction is up
            direction = 1;
        } else { //otherwise if both pots at lower range A/B = up/down means direction is down
            direction = -1;
        }
    } else if (dirPotA == -1 and dirPotB == 1) {
        if ((valuePotA < (_adcMaxValue / 2)) || (valuePotB < (_adcMaxValue / 2))) { //If either pot  at lower range, A/B = down/up means direction is down
            direction = 1;
        } else { //otherwise if bnoth pots at higher range A/B = up/down means direction is down
            direction = -1;
        }
    } else {
        direction = 0; // if any of tap A or B has status unchanged (0), indicate unchanged
    }

    /****************************************************************************
     * Step 3: Calculate value based on direction, how big change in ADC value,
     * and sensitivity. Avoid values around zero and max  as value has flat region
     ****************************************************************************/
    if (dirPotA != 0 && dirPotB != 0) // If both taps indicate movement
    {
        if ((valuePotA < _adcMaxValue * 0.8) && (valuePotA > _adcMaxValue * 0.2)) { // if tap A is not at endpoints
            delta = direction * abs(valuePotA - _previousValuePotA) / _potSensitivity; //increment value
        } else { // If tap A is close to end points, use tap B to calculate value
            delta = direction * abs(valuePotB - _previousValuePotB) / _potSensitivity; //Make sure to add/subtract at least 1, and then additionally the jump in voltage
        }

        _value += delta;

        // Update prev value storage
        _previousValuePotA = valuePotA; // Update previous value variable
        _previousValuePotB = valuePotB; // Update previous value variable
    }

    return delta;
}