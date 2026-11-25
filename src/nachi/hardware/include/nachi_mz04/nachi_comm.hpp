#pragma once 
#include <array>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "rclcpp/logging.hpp"
#include "rclcpp/logger.hpp"

#include <windows.h>

#ifdef ERROR
#undef ERROR
#endif

#define NUM_OF_AXES (6)

namespace nachi_mz04{

enum class LogLevel{Info,Warn,Error};

struct LogEntry{
    LogLevel level;
    std::string text;
};

class OpenNRIF{

    public:
        void set_logger(const rclcpp::Logger& logger){logger_=logger;}
        bool connect(std::string ip);
        void disconnect();
        bool read_axis(float *fValue);
        bool activate_motors();
        bool set_motors_speed(int SetManSpeed, int SetChkSpeed);
        bool configure_recordings();
        bool move1(float *fValue);

      
    private:
        rclcpp::Logger logger_=rclcpp::get_logger("nachi");
        bool connected_=false;

        int nErr=0;
        int nErr2 = 0;
        int nXmlOpenId = 0;
        BOOL bVal;

};


}