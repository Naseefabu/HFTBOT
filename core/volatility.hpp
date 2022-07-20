#pragma once

#include<vector>

class InstantVolatilityIndicator
{
public:

    std::vector<int> processing_buffer;
    std::vector<int> sampling_buffer;
    InstantVolatilityIndicator();
private:
}