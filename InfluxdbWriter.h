#ifndef INFLUXDBWRITER_H_
#define INFLUXDBWRITER_H_

#include <chrono>
#include <map>
#include <deque>
#include <sstream>
#include <string>
#include <type_traits>
#include <curl/curl.h>

namespace PowerMonitor
{
//! InfluxDB client.

//!
//! Sends measurements to InfluxDB. Supports integers, floats and strings,
//! and string tags. Time stamps can be provided or generated on the fly.
//!
class InfluxdbWriter
{
public:
    //! Constructor
    InfluxdbWriter(const std::string& host, const std::string& db, const std::string& user, const std::string& password);
    //! Destructor
    ~InfluxdbWriter();
    //! Send a measurement to InfluxDB.
    //! \param measurement measurement to send to.
    //! \param values map of field-value-pairs.
    //! \param timestamp timestamp in nanoseconds since Unix epoch.
    template<class T> void send(
            const std::string& measurement,
            const std::map<std::string, T>& values,
            long long timestamp = std::chrono::system_clock::now().time_since_epoch().count())
    {
        std::stringstream ss;
        ss << measurement << " ";
        for (auto&& p : values)
        {
            if (!std::is_arithmetic<T>::value)
                ss << p.first << "=\"" << p.second << "\",";
            else if (std::is_integral<T>::value)
                ss << p.first << "=" << p.second << "i,";
            else
                ss << p.first << "=" << p.second << ",";
        }
        // Replace the last comma with a whitespace.
        ss.seekp(-1, ss.cur);
        ss << " " << timestamp;
        _buffer.push_back(ss.str());
        _doSend();
    }
    //! Send a measurement to InfluxDB.
    //! \param measurement measurement to send to.
    //! \param values map of field-value-pairs.
    //! \param tags map of key-value-pairs.
    //! \param timestamp timestamp in nanoseconds since Unix epoch.
    template<class T> void send(
            const std::string& measurement,
            const std::map<std::string, T>& values,
            const std::map<std::string, std::string>& tags,
            long long timestamp = std::chrono::system_clock::now().time_since_epoch().count())
    {
        std::stringstream ss;
        ss << measurement << ",";
        for (auto&& p : tags)
        {
            ss << p.first << "=" << p.second << ",";
        }
        // Replace the last comma with whitespace.
        ss.seekp(-1, ss.cur);
        ss << " ";
        for (auto&& p : values)
        {
            if (!std::is_arithmetic<T>::value)
                ss << p.first << "=\"" << p.second << "\",";
            else if (std::is_integral<T>::value)
                ss << p.first << "=" << p.second << "i,";
            else
                ss << p.first << "=" << p.second << ",";
        }
        // Replace the last comma with whitespace.
        ss.seekp(-1, ss.cur);
        ss << " " << timestamp;
        _buffer.push_back(ss.str());
        _doSend();
    }

private:
    CURLM*                          _multihandle;
    CURL*                           _handle;
    std::deque<std::string>         _buffer;
    int                             _running;

    void _doSend();

};

}

#endif /* INFLUXDBWRITER_H_ */
