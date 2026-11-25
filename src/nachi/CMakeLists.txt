if(MSVC)
    add_compile_options(/utf-8)
endif()
cmake_minimum_required(VERSION 3.16)
project(nachi_mz04 LANGUAGES CXX)


find_package(ros2_control_cmake REQUIRED)
set_compiler_options()
export_windows_symbols()



# find dependencies
set(THIS_PACKAGE_INCLUDE_DEPENDS
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
)
#Setting path to OpenNR-IF 
set(OpenNRDir_x64 "C:/Program Files/nachi/OpenNR-IF")

# Specify the required version of ros2_control
find_package(controller_manager 5.0.0)
# Handle the case where the required version is not found
if(NOT controller_manager_FOUND)
  message(FATAL_ERROR "ros2_control version 5.0.0 or higher is required. "
  "Are you using the correct branch of the ros2_control_demos repository?")
endif()

# find dependencies
find_package(backward_ros REQUIRED)
find_package(ament_cmake REQUIRED)
foreach(Dependency IN ITEMS ${THIS_PACKAGE_INCLUDE_DEPENDS})
  find_package(${Dependency} REQUIRED)
endforeach()

# complie my driver
add_library(
  nachi_mz04
  SHARED
  hardware/nachi.cpp
  hardware/nachi_comm.cpp
)

add_executable(logger 
  hardware/logger.cpp
)

target_link_libraries(logger
  PUBLIC
    rclcpp::rclcpp
    sensor_msgs::sensor_msgs__rosidl_typesupport_cpp
)



target_include_directories(nachi_mz04 PRIVATE
  "${OpenNRDir_x64}"
)
target_link_directories(nachi_mz04 PRIVATE
  "${OpenNRDir_x64}"
)


target_include_directories(nachi_mz04 PUBLIC
$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/hardware/include>
$<INSTALL_INTERFACE:include/nachi_mz04>
)

target_link_libraries(nachi_mz04 PUBLIC
  hardware_interface::hardware_interface
  pluginlib::pluginlib
  rclcpp::rclcpp
  rclcpp_lifecycle::rclcpp_lifecycle
  "${OpenNRDir_x64}/OpenNR-IF.lib"
)

# Export hardware plugins
pluginlib_export_plugin_description_file(hardware_interface nachi_mz04.xml)

# INSTALL
install(
  DIRECTORY hardware/include/
  DESTINATION include/nachi_mz04
)
install(
  DIRECTORY rviz
  DESTINATION share/nachi_mz04
)
install(
  DIRECTORY meshes
  DESTINATION share/nachi_mz04
)

install(
  DIRECTORY description/launch description/ros2_control description/urdf
  DESTINATION share/nachi_mz04
)
install(
  DIRECTORY bringup/launch bringup/config
  DESTINATION share/nachi_mz04
)
install(TARGETS nachi_mz04
  EXPORT export_nachi_mz04
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(TARGETS logger
  DESTINATION lib/nachi_mz04
)




#[[
if(BUILD_TESTING)
  find_package(ament_cmake_pytest REQUIRED)
  ament_add_pytest_test(example_1_urdf_xacro test/test_urdf_xacro.py)

  # Integration (launch) tests
  find_package(ament_cmake_ros REQUIRED)
  find_package(launch_testing_ament_cmake REQUIRED)
  function(add_ros_isolated_launch_test path)
    set(RUNNER "${ament_cmake_ros_DIR}/run_test_isolated.py")
    add_launch_test("${path}" RUNNER "${RUNNER}" ${ARGN})
  endfunction()
  add_ros_isolated_launch_test(test/test_view_robot_launch.py)
  add_ros_isolated_launch_test(test/test_rrbot_launch.py)
  add_ros_isolated_launch_test(test/test_rrbot_launch_cli_direct.py)
  add_ros_isolated_launch_test(test/test_rrbot_movement.py)
endif()
#]]

## EXPORTS
ament_export_targets(export_nachi_mz04 HAS_LIBRARY_TARGET)
ament_export_dependencies(${THIS_PACKAGE_INCLUDE_DEPENDS})
ament_package()
