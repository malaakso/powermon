/*
 * Meter.h
 *
 *  Created on: Feb 14, 2016
 *      Author: mlaakso
 */

#ifndef METER_H_
#define METER_H_

#include <cmath>
#include <vector>

namespace PowerMonitor
{

class Meter
{
public:
    Meter()
    {

    }
    template<class It> static float getRMS(It begin, It end)
    {
        if (begin == end)
            return 0.f;
        float v, squares = 0.f;
        unsigned int size = 0;
        while (begin != end)
        {
            v = *begin++;
            squares += v * v;
            size++;
        }
        return std::sqrt(squares / size);
    }
    template<class It> static float getAverage(It begin, It end)
    {
        if (begin == end)
            return 0.f;
        float sum = 0.f;
        unsigned int size = 0;
        while (begin != end)
        {
            sum += *begin++;
            size++;
        }
        return sum / size;
    }
    template<class It> static float getFrequency(It begin, It end)
    {
        if (begin == end)
            return 0.f;

        unsigned int n = 0;
        unsigned int before = 0;
        unsigned int after = 0;
        unsigned int size = 0;
        float v, previous = 0.f;
        while (begin != end)
        {
            v = *begin++;
            if (n == 0)
                before++;
            else
                after++;
            if (v >= 0.f && previous < 0.f)
            {
                n++;
                after = 0;
            }
            previous = v;
            size++;
        }
        return (float)(n - 1) / (size - before - after);
    }
    template<class It1, class It2> static float getAveragePower(It1 first, It1 end, It2 second)
    {
        if (first == end)
            return 0.f;
        float sum = 0.f;
        unsigned int size = 0;
        while (first != end)
        {
            sum += (*first++) * (*second++);
            size++;
        }
        return sum / size;
    }
};

} /* namespace PowerMonitor */

#endif /* METER_H_ */
