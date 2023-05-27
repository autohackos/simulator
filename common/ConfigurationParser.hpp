/*
   Configuration parser for car CAN bus simulator
   Copyright(c) 2023 Adhokshaj Mishra <me@adhokshajmishraonline.in>
*/

#ifndef CONFIGURATION_PARSER
#define CONFIGURATION_PARSER

#include "../3rdparty/json.hpp"
#include "car.hpp"
#include "can.hpp"

#include <filesystem>
#include <string>

/*
struct CarConfiguration
{
    float maximum_speed;
    float acceleration;
    int door_lock;
    int door_unlock;
    int turn_signal_enable;
    int turn_signal_disable;
};

struct CANBusConfiguration
{
    int door_id;
    int signal_id;
    int speed_id;
};
*/

class ConfigurationParser
{
private:
    std::filesystem::path configuration_file;
    nlohmann::json config_data;

    //CarConfiguration car;
protected:
public:
    ConfigurationParser(std::string file_path = "./config.json");
    bool parse();
};

#endif
