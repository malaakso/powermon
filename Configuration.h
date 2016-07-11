/*
 * Configuration.h
 *
 *  Created on: Jul 11, 2016
 *      Author: mlaakso
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <string>
#include <utility>
#include <vector>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace PowerMonitor
{

class Configuration
{
public:
    //! Creates a new configuration object.
    //! \param configFile path to configuration file to read settings from.
    Configuration(const std::string& configFile)
    {
        boost::property_tree::read_json(configFile, _tree);
    }
    //! Gets a configuration option.
    //! \param option option name.
    //! \return Option value.
    template<class T> T get(const std::string& option)
    {
        return _tree.get<T>(option);
    }
    //! Gets a configuration option.
    //! \param option option name.
    //! \param def default value to return if option is not set.
    //! \return Option value.
    template<class T> T get(const std::string& option, T def)
    {
        return _tree.get(option, def);
    }
    //! Gets a configuration option array.
    //! \param option option name.
    //! \return Option values as vector.
    template<class T> std::vector<T> getArray(const std::string& option)
    {
        std::vector<T> r;
        for (auto&& item : _tree.get_child(option))
            r.push_back(item.second.get_value<T>());
        return r;
    }
private:
    boost::property_tree::ptree _tree;
};

}

#endif /* CONFIGURATION_H_ */
