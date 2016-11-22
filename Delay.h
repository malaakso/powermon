/*
 * Delay.h
 *
 *  Created on: Oct 21, 2015
 *      Author: mlaakso
 */

#ifndef DELAY_H_
#define DELAY_H_

#include <queue>

namespace PowerMonitor
{
//! Delay line for delaying a signal by a specific number of samples.
//!
//! Delay line can be used to compensate for the group delay of another
//! signal caused by signal processing when the two signals should be
//! analyzed simultaneously. For low-pass filtering the group delay at
//! low frequencies is n = f_s / (pi * f_c), where f_c is the cutoff frequency
//! of the low-pass filter and f_s the sampling rate.
//!
template<class T> class Delay
{
public:
    //! Constructor.
    //! \param n delay in number of samples.
    Delay(unsigned int n)
    {
        for (unsigned int i = 0; i < n; ++i)
            _buffer.emplace();
    }

    //! Delays signal.
    //! \param sample next sample in the signal.
    //! \return Delayed sample.
    T operator()(const T sample)
    {
        T value = _buffer.front();
        _buffer.pop();
        _buffer.push(sample);
        return value;
    }
private:
    std::queue<T> _buffer; //!< Buffer to hold the delayed values.
};

} /* namespace PowerMonitor */

#endif /* DELAY_H_ */
