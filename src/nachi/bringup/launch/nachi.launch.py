# Copyright 2021 Stogl Robotics Consulting UG (haftungsbeschränkt)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import Command, LaunchConfiguration, PathSubstitution

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch_ros.parameter_descriptions import ParameterFile

from launch.substitutions import PathJoinSubstitution


def generate_launch_description():
    gui_arg = DeclareLaunchArgument(
        "gui",
        default_value="true",
        description="Start RViz2 automatically with this launch file.",
    )
    use_fake_hardware_arg = DeclareLaunchArgument(
        "use_fake_hardware",
        default_value="true",
        description="Start robot with fake hardware mirroring command to its states.",
    )
    nachi_ip_arg = DeclareLaunchArgument(
        "nachi_ip",
        default_value="127.0.0.1",
        description="IP address of the Nachi robot simulation CFD.",
    )

    robot_controllers = PathJoinSubstitution(
        [FindPackageShare("nachi_mz04"), "config", "nachi_controllers.yaml"]
    )

    nachi_ip = LaunchConfiguration("nachi_ip")
    use_fake_hardware = LaunchConfiguration("use_fake_hardware")
    return LaunchDescription(
        [
            gui_arg,
            use_fake_hardware_arg,
            nachi_ip_arg,

            # Control node
            Node(
                package="controller_manager",
                executable="ros2_control_node",
                parameters=[robot_controllers],
                output="both",
            ),
            # robot_state_publisher with robot_description from xacro
            Node(
                package="robot_state_publisher",
                executable="robot_state_publisher",
                output="both",
                parameters=[
                    {
                        "robot_description": Command(
                            [
                                "xacro",
                                " ",
                                PathSubstitution(FindPackageShare("nachi_mz04"))
                                / "urdf"
                                / "nachi.urdf.xacro",
                                " ",
                                "use_fake_hardware:=",
                                use_fake_hardware,
                                " ",
                                "nachi_ip:=",
                                nachi_ip,
                            ]
                        )
                    }
                ],
            ),
            Node(
                package="rviz2",
                executable="rviz2",
                name="rviz2",
                output="log",
                arguments=[
                    "-d",
                    PathSubstitution(FindPackageShare("nachi_mz04"))
                    / "rviz/nachi.rviz",
                ],
                condition=IfCondition(LaunchConfiguration("gui")),
            ),
            Node(
                package="controller_manager",
                executable="spawner",
                parameters=[robot_controllers],
                arguments=[
                    "joint_state_broadcaster",
                ],
            ),
            Node(
                package="controller_manager",
                executable="spawner",
                parameters=[robot_controllers],
                arguments=[
                    "forward_position_controller",
                ],
            ),
        ]
    )
