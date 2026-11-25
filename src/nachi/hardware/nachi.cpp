// Copyright 2020 ros2_control Development Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "nachi_mz04/nachi.hpp"


#include <chrono>
#include <cmath>
#include <numbers>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>


#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"


/*
#include <windows.h>
#include <OpenNR-IF.h>
#ifdef ERROR
#undef ERROR
#endif
*/



namespace nachi_mz04
{

static const rclcpp::Logger LOGGER = rclcpp::get_logger("MZ04Hardware");

CallbackReturn RobotSystem::on_init(const hardware_interface::HardwareComponentInterfaceParams & params)
{
    if (hardware_interface::SystemInterface::on_init(params) != CallbackReturn::SUCCESS)
    {
        return CallbackReturn::ERROR;
    }
  auto info_ = params.hardware_info;

  communication_.set_logger(LOGGER);
  
  std::string nachi_ip=info_.hardware_parameters["nachi_ip"];



  joint_position_.assign(6, 0);
  joint_position_command_.assign(6, 0);

 

  for (int i = 0; i < 6; i++)
    jnts_com[i] = (double) joint_position_command_[i];

  for (const auto & joint : info_.joints)
  {
    for (const auto & inter_face : joint.state_interfaces)
    {
      joint_interfaces[inter_face.name].push_back(joint.name);
    }
  }


  config=communication_.connect(nachi_ip);
  
  if(!config)return CallbackReturn::ERROR;
    
  return CallbackReturn::SUCCESS;
}



CallbackReturn RobotSystem::on_configure(const rclcpp_lifecycle::State & /*previous_state*/)
{
    
  std::fill_n(fValue,6,0.0f);
  config=false;

  config=communication_.read_axis(fValue);
  if(!config)return CallbackReturn::ERROR;

  for (size_t i = 0; i < 6; i++) {
    joint_position_[i]=fValue[i] * std::numbers::pi/180.0;
    joint_position_command_[i]=fValue[i] * std::numbers::pi/180.0;
  }


  config=communication_.activate_motors();
  if(!config)return CallbackReturn::ERROR;

  config=communication_.set_motors_speed(5,5);  //max is 5 , 3 is preferred
  if(!config)return CallbackReturn::ERROR;

  config=communication_.configure_recordings();
  if(!config)return CallbackReturn::ERROR;


  return CallbackReturn::SUCCESS;
}



CallbackReturn RobotSystem::on_deactivate(const rclcpp_lifecycle::State & /*previous_state*/)
{
  communication_.disconnect();
  
  return CallbackReturn::SUCCESS;
}






return_type RobotSystem::read(const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  std::fill_n(fValue,6,0.0f);
  config=false;

  config=communication_.read_axis(fValue);
  if(!config)return return_type::ERROR;

  for (size_t i = 0; i < 6; i++) {
    joint_position_[i]=fValue[i] * std::numbers::pi/180.0;
  }

  return return_type::OK;
}





return_type RobotSystem::write(const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  bool diff=false;
  config=false;
  std::fill_n(fValue,6,0.0f);

  for(int i=0;i<6;i++){
    if(jnts_com[i]!=joint_position_command_[i]){
      diff=true;
      break;
    }
  }

  if(diff){
    for(int i=0;i<6;i++){
      jnts_com[i]=joint_position_command_[i];
      fValue[i]=jnts_com[i]*(180.0/std::numbers::pi);
    }

    config=communication_.move1(fValue);

    if(!config)return return_type::ERROR;
  }


  return return_type::OK;
}






std::vector<hardware_interface::StateInterface> RobotSystem::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;

  int ind = 0;
  for (const auto & joint_name : joint_interfaces["position"])
  {
    state_interfaces.emplace_back(joint_name, "position", &joint_position_[ind++]);
  }

  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> RobotSystem::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;

  int ind = 0;
  for (const auto & joint_name : joint_interfaces["position"])
  {
    command_interfaces.emplace_back(joint_name, "position", &joint_position_command_[ind++]);
  }

  return command_interfaces;
}








}  






#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(
  nachi_mz04::RobotSystem, hardware_interface::SystemInterface)

