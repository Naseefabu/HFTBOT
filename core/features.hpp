#pragma once

#include <iostream>
#include <vector>

// No. of Trades in a given time
class TapeSpeedIndicator
{
public:

    TapeSpeedIndicator(lookback);
    void clear();
    void add();
    int get_tapespeed();
    void remove();

private:
    int lookback_period = 60;
    int tapespeed = 0;
};