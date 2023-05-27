/*
   Configuration parser for car CAN bus simulator
   Copyright(c) 2023 Adhokshaj Mishra <me@adhokshajmishraonline.in>
*/

#include "ConfigurationParser.hpp"

#include <fstream>
#include <iostream>

ConfigurationParser::ConfigurationParser(std::string file_path)
{
    this->configuration_file = file_path;
}

bool ConfigurationParser::parse()
{
    /*
       Before we even attempt reading data, we need to check the following:
       1. that the given path exists
       2. that the given path is a regular file (not special files like sockets, device files etc)
       3. that the given path does not point to an empty file

       If given path is a symlink, we resolve the symlink and run all checks on resolved path.
    */

    try
    {
        if (!std::filesystem::exists(this->configuration_file))
        {
            std::cerr << "Error: configuration file does not exist." << std::endl;
            return false;
        }
        if (std::filesystem::is_symlink(this->configuration_file))
        {
            this->configuration_file = std::filesystem::read_symlink(this->configuration_file);

            if (!std::filesystem::exists(this->configuration_file))
            {
                std::cerr << "Error: configuration file is a symlink resolving to non-existing path." << std::endl;
                return false;
            }
        }
        if (!std::filesystem::is_regular_file(this->configuration_file))
        {
            std::cerr << "Error: configuration file is not a regular file." << std::endl;
            return false;
        }
        if (std::filesystem::is_empty(this->configuration_file))
        {
            std::cerr << "Error: configuration file is empty." << std::endl;
            return false;
        }
    }
    catch (const std::filesystem::filesystem_error& error)
    {
        std::cerr << "Error (" << error.code() << ") while validating configuration file path: " << error.what() << std::endl;
        return false;
    }

    std::ifstream config_file(this->configuration_file);
    std::string str((std::istreambuf_iterator<char>(config_file)), std::istreambuf_iterator<char>());

    try {
        this->config_data = nlohmann::json::parse(str);
    }
    catch (const nlohmann::json::parse_error& error)
    {
        std::cerr << "Error: could not parse JSON. Parse error: " << error.what() << std::endl;
        return false;
    }
    catch (const nlohmann::json::type_error& error)
    {
        std::cerr << "Error: invalid type encountered in JSON. Parse error: " << error.what() << std::endl;
        return false;
    }
    catch(const std::exception& ex)
    {
        std::cerr << "Error: some unknown error occurred. Parse error: " << ex.what() << std::endl;
        return false;
    }

    nlohmann::json car_parameters;
    nlohmann::json canbus_message_parameters;

    if (config_data.contains("car"))
    {
	car_parameters = config_data["car"];
    }
    else
    {
	std::cerr << "Error: Car parameters are missing from configuration file" << std::endl;
	return false;
    }

    if (config_data.contains("canbus"))
    {
	canbus_message_parameters = config_data["canbus"];
    }
    else
    {
	std::cerr << "Error: CAN message parameters are missing from configuration file" << std::endl;
	return false;
    }

    // parse car related configuration
    if (car_parameters.contains("maximum_speed"))
    {
	CarParameters::MaximumSpeed = car_parameters["maximum_speed"].get<float>();
    }
    if (car_parameters.contains("acceleration"))
    {
	CarParameters::AccelerationRate = car_parameters["acceleration"].get<float>();
    }
    if (car_parameters.contains("door_lock"))
    {
	Car::Status::Door::Locked = car_parameters["door_lock"].get<int>();
    }
    if (car_parameters.contains("door_unlock"))
    {
	Car::Status::Door::Unlocked = car_parameters["door_unlock"].get<int>();
    }
    if (car_parameters.contains("turn_signal_enable"))
    {
	Car::Status::TurnSignal::On = car_parameters["turn_signal_enable"].get<int>();
    }
    if (car_parameters.contains("turn_signal_disable"))
    {
	Car::Status::TurnSignal::Off = car_parameters["turn_signal_disable"].get<int>();
    }

    // parse canbus message related configuration
    if (canbus_message_parameters.contains("id"))
    {
	nlohmann::json can_id = canbus_message_parameters["id"];
	if (can_id.contains("door"))
	{
	    CanMessage::ID::Door = can_id["door"].get<int>();
	}
	if (can_id.contains("signal"))
	{
	    CanMessage::ID::Signal = can_id["signal"].get<int>();
	}
	if (can_id.contains("speed"))
	{
	    CanMessage::ID::Speed = can_id["speed"].get<int>();
	}
    }
    if (canbus_message_parameters.contains("position"))
    {
        nlohmann::json can_position = canbus_message_parameters["position"];
        if (can_position.contains("door"))
        {
            CanMessage::Position::Door = can_position["door"].get<int>();
        }
        if (can_position.contains("signal"))
        {
            CanMessage::Position::Signal = can_position["signal"].get<int>();
        }
        if (can_position.contains("speed"))
        {
            CanMessage::Position::Speed = can_position["speed"].get<int>();
        }
    }
    if (canbus_message_parameters.contains("length"))
    {
        nlohmann::json can_length = canbus_message_parameters["length"];
        if (can_length.contains("door"))
        {
            CanMessage::Length::Door = CanMessage::Position::Door + can_length["door"].get<int>();
        }
        if (can_length.contains("signal"))
        {
            CanMessage::Length::Signal = CanMessage::Position::Signal + can_length["signal"].get<int>();
        }
        if (can_length.contains("speed"))
        {
            CanMessage::Length::Speed = CanMessage::Position::Speed + can_length["speed"].get<int>();
        }
    }
    if (canbus_message_parameters.contains("message"))
    {
	nlohmann::json can_message = canbus_message_parameters["message"];
	if (can_message.contains("left_signal"))
	{
	    CanMessage::Equipment::LeftSignal = can_message["left_signal"].get<int>();
	}
	if (can_message.contains("right_signal"))
        {
            CanMessage::Equipment::RightSignal = can_message["right_signal"].get<int>();
        }
	if (can_message.contains("door1"))
        {
            CanMessage::Equipment::Door1 = can_message["door1"].get<int>();
        }
	if (can_message.contains("door2"))
        {
            CanMessage::Equipment::Door2 = can_message["door2"].get<int>();
        }
	if (can_message.contains("door3"))
        {
            CanMessage::Equipment::Door3 = can_message["door3"].get<int>();
        }
	if (can_message.contains("door4"))
        {
            CanMessage::Equipment::Door4 = can_message["door4"].get<int>();
        }
    }

    return true;
}
