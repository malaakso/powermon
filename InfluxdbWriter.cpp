#include "InfluxdbWriter.h"

namespace PowerMonitor
{

InfluxdbWriter::InfluxdbWriter(const std::string& host, const std::string& db, const std::string& user, const std::string& password) :
        _running(0)
{
    CURLcode ret;
    struct curl_slist *list = NULL;
    ret = curl_global_init(CURL_GLOBAL_ALL);
    if (ret)
    {
        throw std::runtime_error("Could not initialize curl: "
                + std::string(curl_easy_strerror(ret)));
    }
    _handle = curl_easy_init();
    if (!_handle)
    {
        curl_global_cleanup();
        throw std::runtime_error("Unable to obtain curl handle.");
    }
    _multihandle = curl_multi_init();
    if (!_multihandle)
    {
        curl_easy_cleanup(_handle);
        curl_global_cleanup();
        throw std::runtime_error("Unable to obtain curl multi handle.");
    }

    std::stringstream url;
    url << host << "/write?db=" << db;
    curl_easy_setopt(_handle, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(_handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(_handle, CURLOPT_USERNAME, user.c_str());
    curl_easy_setopt(_handle, CURLOPT_PASSWORD, password.c_str());
    list = curl_slist_append(list, "Expect:");
    curl_easy_setopt(_handle, CURLOPT_HTTPHEADER, list);
    curl_multi_add_handle(_multihandle, _handle);
    curl_multi_perform(_multihandle, &_running);
}

InfluxdbWriter::~InfluxdbWriter()
{
    curl_multi_remove_handle(_multihandle, _handle);
    curl_easy_cleanup(_handle);
    curl_multi_cleanup(_multihandle);
    curl_global_cleanup();
}

void InfluxdbWriter::_doSend()
{
    curl_multi_perform(_multihandle, &_running);
    if (!_running)
    {
        std::stringstream payload;
        for (auto&& line : _buffer)
        {
            payload << line << "\n";
        }
        _buffer.clear();
        curl_multi_remove_handle(_multihandle, _handle);
        curl_easy_setopt(_handle, CURLOPT_POSTFIELDS, payload.str().c_str());
        curl_multi_add_handle(_multihandle, _handle);
        curl_multi_perform(_multihandle, &_running);
    }
}

}
