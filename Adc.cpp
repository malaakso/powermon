/*
 * Adc.cc
 *
 *  Created on: Mar 5, 2015
 *      Author: mlaakso
 */

#include <cerrno>
#include <cstring>
#include <string>
#include <stdexcept>
#include "Adc.h"

namespace PowerMonitor
{

GalileoGen2Adc::GalileoGen2Adc(unsigned int nChannels, unsigned int freq) :
        _nChannels(nChannels)
{
    int ret;
    _iioctx = iio_create_local_context();
    if (_iioctx == nullptr)
    {
        throw std::runtime_error("Could not create iio context.");
    }
    _adc = iio_context_find_device(_iioctx, "adc1x8s102");
    if (_adc == nullptr)
    {
        iio_context_destroy(_iioctx);
        throw std::runtime_error("Could not find iio device.");
    }
    _trigger = iio_context_find_device(_iioctx, "hrtimer_trig0");
    if (_trigger == nullptr)
    {
        iio_context_destroy(_iioctx);
        throw std::runtime_error("Could not find iio trigger device.");
    }
    ret = iio_device_attr_write_longlong(_trigger, "frequency", freq);
    if (ret < 0)
    {
        iio_context_destroy(_iioctx);
        throw std::runtime_error("Could not set sampling frequency: " + std::string(strerror(ret)));
    }
    // Associate trigger with ADC.
    ret = iio_device_set_trigger(_adc, _trigger);
    if (ret < 0)
    {
        iio_context_destroy(_iioctx);
        throw std::runtime_error("Could not associate trigger: " + std::string(strerror(ret)));
    }
    // Enable ADC channels.
    unsigned int n = iio_device_get_channels_count(_adc);
    for (unsigned int i = 0; i < n; ++i)
    {
        if (i < nChannels)
            iio_channel_enable(iio_device_get_channel(_adc, i));
        else
            iio_channel_disable(iio_device_get_channel(_adc, i));
    }
    // Create a buffer holding 1 second worth of values.
    _adcbuf = iio_device_create_buffer(_adc, freq, false);
    if (_adcbuf == nullptr)
    {
        iio_context_destroy(_iioctx);
        throw std::runtime_error("Could not create iio buffer: " + std::string(strerror(errno)));
    }
}

GalileoGen2Adc::~GalileoGen2Adc()
{
    iio_buffer_destroy(_adcbuf);
    iio_context_destroy(_iioctx);
}

void GalileoGen2Adc::refill()
{
    ssize_t bytes = iio_buffer_refill(_adcbuf);
    if (bytes < 0)
    {
        throw std::runtime_error("Could not read iio buffer: " + std::string(strerror(bytes)));
    }
}

GalileoGen2Adc::const_iterator GalileoGen2Adc::cbegin(unsigned int channel) const
{
    struct iio_channel* chn = iio_device_get_channel(_adc, channel);
    if (chn == nullptr)
    {
        throw std::runtime_error("Invalid channel index.");
    }
    char* ptr = (char*) iio_buffer_first(_adcbuf, chn);
    return const_iterator(ptr, iio_buffer_step(_adcbuf), chn);
}

GalileoGen2Adc::const_iterator GalileoGen2Adc::cend(unsigned int channel) const
{
    struct iio_channel* chn = iio_device_get_channel(_adc, channel);
    if (chn == nullptr)
    {
        throw std::runtime_error("Invalid channel index.");
    }
    char* ptr = (char*) iio_buffer_first(_adcbuf, chn);
    char* end = (char*) iio_buffer_end(_adcbuf);
    while (ptr < end)
        ptr += iio_buffer_step(_adcbuf);
    return const_iterator(ptr, iio_buffer_step(_adcbuf), chn);
}

}
