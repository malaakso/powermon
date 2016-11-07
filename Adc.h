/*
 * Adc.h
 *
 *  Created on: Mar 5, 2015
 *      Author: mlaakso
 */

#ifndef ADC_H_
#define ADC_H_

#include <iterator>
#include <iio.h>

namespace PowerMonitor
{

class GalileoGen2Adc
{
public:
    GalileoGen2Adc(unsigned int nChannels, unsigned int freq);
    ~GalileoGen2Adc();
    void refill();

    class const_iterator: public std::iterator<std::input_iterator_tag, float, ptrdiff_t, const float*, const float&>
    {
    public:
        explicit const_iterator(char* first, ptrdiff_t step, struct iio_channel* chn) :
            _ptr(first),
            _step(step),
            _chn(chn)
        {
        }
        value_type operator*() const
        {
            uint16_t tmp;
            iio_channel_convert(_chn, &tmp, _ptr);
            // TODO: Dynamically set these.
            return tmp * 5000.0 / 4096;
        }
        const_iterator& operator++()
        {
            _ptr += _step;
            return *this;
        }
        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            _ptr += _step;
            return tmp;
        }
        bool operator==(const const_iterator& other) const
        {
            return _ptr == other._ptr;
        }
        bool operator!=(const const_iterator& other) const
        {
            return _ptr != other._ptr;
        }
    protected:
        const char*         _ptr;
        ptrdiff_t           _step;
    private:
        struct iio_channel* _chn;
    };

    const_iterator cbegin(unsigned int channel) const;
    const_iterator cend(unsigned int channel) const;

private:
    struct iio_context*                                 _iioctx;
    struct iio_device*                                  _adc;
    struct iio_device*                                  _trigger;
    struct iio_buffer*                                  _adcbuf;
    unsigned int                                        _nChannels;
};

}

#endif /* ADC_H_ */
