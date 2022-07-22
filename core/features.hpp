#pragma once

#include <iostream>
#include <vector>
#include "utils.hpp"

// No. of Trades in a given time
class TapeSpeedIndicator
{
public:

    TapeSpeedIndicator(int &lookback);
    void clear();
    void add(double &volume);
    int get_tapespeed();
    void remove();

private:
    int lookback_period = 60;
    int tapespeed = 0;
    std::vector<float> times;
};