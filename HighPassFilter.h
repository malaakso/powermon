/*
 * HighPassFilter.h
 *
 *  Created on: Nov 6, 2016
 *      Author: mlaakso
 */

#ifndef HIGHPASSFILTER_H_
#define HIGHPASSFILTER_H_

namespace PowerMonitor
{

//! Simple IIR high-pass filter.
template<class T> class HighPassFilter
{
public:
    //! Constructor.
    //! \param f crossover frequency in Hz.
    //! \param dt time step in s.
    HighPassFilter(T dt, T f) :
            _alpha(1.0 / (1.0 + 6.283185307 * f * dt)),
            _initialized(false)
    {
    }

    T operator()(T sample)
    {
        if (!_initialized)
        {
            _previousResult = sample;
            _previousSample = sample;
            _initialized = true;
            return sample;
        }
        T result = _alpha * (_previousResult + sample - _previousSample);
        _previousResult = result;
        _previousSample = sample;
        return result;
    }

private:
    T _alpha;               //!< Filter coefficient.
    T _previousSample;      //!< Previous sample.
    T _previousResult;      //!< Previous filtered sample.
    bool _initialized;      //!< Previous samples initialized?
};

}

#endif /* HIGHPASSFILTER_H_ */
