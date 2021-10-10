#ifndef INFINITE_POT_H
#define INFINITE_POT_H

class InfinitePot
{
    int _previousValuePotA = 0;
    int _previousValuePotB = 0;
    int _value = 0;

    int _adcHysteresis = 50;
    int _potSensitivity = 20;
    int _adcMaxValue = 4096;

public:
    InfinitePot() {};
    InfinitePot(int adcHysteresis, int potSensitivity, int adcMaxValue):
        _adcHysteresis(adcHysteresis), _potSensitivity(potSensitivity), _adcMaxValue(adcMaxValue) {};
    
    void setup(int valuePotA, int valuePotB);
    int update(int valuePotA, int valuePotB);

    int getValue() {
        return _value;
    }
};

#endif // INFINITE_POT_H