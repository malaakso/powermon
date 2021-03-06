#include <algorithm>
#include <csignal>
#include <iostream>
#include <vector>
#include "Adc.h"
#include "Configuration.h"
#include "Delay.h"
#include "HighPassFilter.h"
#include "InfluxdbWriter.h"
#include "Meter.h"

#define BURDEN_RESISTANCE 99.5f // Ohms
#define VOLTAGE_DIVIDER_RATIO 0.138268156f // 9.9 / (61.7 + 9.9)
#define CURRENT_TRANSFORMER_RATIO 0.0005f // 50 mA / 100 A
#define TRANSFORMER_RATIO (0.048869565f * 0.9657f) // 11.24 V / 230 V * calibration factor

// Channel mappings.
#define L1 1
#define L2 3
#define L3 0
#define V 4
#define N 2

using namespace PowerMonitor;

volatile sig_atomic_t terminate = 0;

static void sighandler(int signum)
{
    terminate = 1;
}

float voltageToVoltage(float v)
{
    // Millivolts from ADC to mains voltage in volts.
    return v / 1000.f / VOLTAGE_DIVIDER_RATIO / TRANSFORMER_RATIO;
}

float voltageToCurrent(float v)
{
    // Millivolts from ADC to mains current in amperes.
    return v / 1000.f / BURDEN_RESISTANCE / CURRENT_TRANSFORMER_RATIO;
}

int main(int argc, char **argv)
{
    struct sigaction sa;

    sa.sa_handler = &sighandler;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    try
    {
        Configuration conf("/etc/powermon.json");
        
        InfluxdbWriter influx(
                conf.get<std::string>("PowerMonitor.InfluxDB.host"),
                conf.get<std::string>("PowerMonitor.InfluxDB.database"),
                conf.get<std::string>("PowerMonitor.InfluxDB.username"),
                conf.get<std::string>("PowerMonitor.InfluxDB.password"));
        
        // 2100 divides evenly with 50 and 3.
        unsigned int sample_rate = conf.get("PowerMonitor.samplerate", 2100);
        GalileoGen2Adc adc(5, sample_rate);
        
        unsigned int mains_freq = conf.get("PowerMonitor.mainsfreq", 50);
        unsigned int samples_per_cycle = sample_rate / mains_freq;
        Delay<float> delayL2(2 * samples_per_cycle / 3), delayL3(samples_per_cycle / 3);
        HighPassFilter<float> hpfV(1.f / sample_rate, 1.f), hpfL1(1.f / sample_rate, 1.f);
        HighPassFilter<float> hpfL2(1.f / sample_rate, 1.f), hpfL3(1.f / sample_rate, 1.f);
        std::vector<float> v(sample_rate), l1(sample_rate), l2(sample_rate), l3(sample_rate);

        bool print = conf.get("PowerMonitor.print", false);
        float vRMS, vf;
        float i1RMS, i2RMS, i3RMS;
        float p1, p2, p3;
        float pf1, pf2, pf3;

        while (!terminate)
        {
            // Read ADC.
            adc.refill();

            // Copy data.
            std::copy(adc.cbegin(V), adc.cend(V), v.begin());
            std::copy(adc.cbegin(L1), adc.cend(L1), l1.begin());
            std::copy(adc.cbegin(L2), adc.cend(L2), l2.begin());
            std::copy(adc.cbegin(L3), adc.cend(L3), l3.begin());
            // Transform data.
            // Filtering and delay must be done in order, use for loop.
            for (auto&& value : v)
                value = hpfV(voltageToVoltage(value));
            for (auto&& value : l1)
                value = hpfL1(voltageToCurrent(value));
            for (auto&& value : l2)
                value = delayL2(hpfL2(voltageToCurrent(value)));
            for (auto&& value : l3)
                value = delayL3(hpfL3(voltageToCurrent(value)));
            
            // Perform calculations.
            vRMS = Meter::getRMS(v.cbegin(), v.cend());
            vf = sample_rate * Meter::getFrequency(v.cbegin(), v.cend());
            i1RMS = Meter::getRMS(l1.cbegin(), l1.cend());
            i2RMS = Meter::getRMS(l2.cbegin(), l2.cend());
            i3RMS = Meter::getRMS(l3.cbegin(), l3.cend());
            p1 = Meter::getAveragePower(l1.cbegin(), l1.cend(), v.cbegin());
            p2 = Meter::getAveragePower(l2.cbegin(), l2.cend(), v.cbegin());
            p3 = Meter::getAveragePower(l3.cbegin(), l3.cend(), v.cbegin());
            pf1 = p1 / (i1RMS * vRMS);
            pf2 = p2 / (i2RMS * vRMS);
            pf3 = p3 / (i3RMS * vRMS);
            
            // Print results.
            if (print)
            {
                std::cout << vRMS << " V " << vf << " Hz\n";
                std::cout << i1RMS << " A " << i2RMS << " A " << i3RMS << " A\n";
                std::cout << p1 << " W " << p2 << " W " << p3 << " W\n";
                std::cout << pf1 << " " << pf2 << " " << pf3 << "\n";
            }
            // Send results.
            influx.send<float>("voltage",
                    {
                        { "voltage", vRMS },
                        { "frequency", vf }
                    });
            influx.send<float>("power",
                    {
                        { "l1", p1 },
                        { "l2", p2 },
                        { "l3", p3 },
                        { "pf1", pf1 },
                        { "pf2", pf2 },
                        { "pf3", pf3 }
                    });
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
