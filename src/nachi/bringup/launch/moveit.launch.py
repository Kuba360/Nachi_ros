import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition
from launch.substitutions import Command, LaunchConfiguration, PathSubstitution
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import PathJoinSubstitution
from ament_index_python.packages import get_package_share_directory

from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch_ros.parameter_descriptions import ParameterFile
from moveit_configs_utils import MoveItConfigsBuilder

from launch.substitutions import (
    Command,
    FindExecutable,
    LaunchConfiguration,
    PathJoinSubstitution,
)

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

    nachi_ip = LaunchConfiguration("nachi_ip")
    use_fake_hardware = LaunchConfiguration("use_fake_hardware")


    robot_description_content=Command(
        [
            PathJoinSubstitution([FindExecutable(name="xacro")]),
            " ",
            PathJoinSubstitution(
                [FindPackageShare("nachi_mz04"), "urdf", "nachi.urdf.xacro"]
            ),
            " ",
            "nachi_ip:=",
            nachi_ip,
            " ",
            "use_fake_hardware:=",
            use_fake_hardware,
            " ",
        ]
    )
    robot_description = {"robot_description": robot_description_content}




    robot_controllers = PathJoinSubstitution(
        [FindPackageShare("nachi_mz04"), "config", "nachi_controllers.yaml"]
    )



    world_to_base_tf = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        name="world_to_base",
        arguments=[
            "--x", "0",
            "--y", "0",
            "--z", "0",
            "--roll", "0",
            "--pitch", "0",
            "--yaw", "0",
            "--frame-id", "world",
            "--child-frame-id", "base_link",
        ],
        output="screen",
    )

    moveit_config = (
    MoveItConfigsBuilder("mz04", package_name="nachi_mz04")
        .robot_description(
            file_path="urdf/nachi.urdf.xacro",
            mappings={
            "use_fake_hardware": use_fake_hardware,
            "nachi_ip": nachi_ip,
            }
        )
        .robot_description_semantic(
            file_path="config/nachi.srdf"
        )
        .robot_description_kinematics(
            file_path="config/kinematics.yaml"
        )
        .joint_limits(
            file_path="config/joint_limits.yaml"
        )
        .trajectory_execution(
            file_path="config/moveit_controllers.yaml"
        )
        .planning_pipelines(
            pipelines=["ompl"]
        )
        .to_moveit_configs()
    )

    moveit_group_node = Node(
        package="moveit_ros_move_group",
        executable="move_group",
        output="both",
        parameters=[
            moveit_config.to_dict()
        ],
    )


            # Control node
    ros2_control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[robot_controllers, robot_description],
        # remappings=[
        #    ("~/robot_description", "/robot_description"),
        # ],
    )

    # robot_state_publisher with robot_description from xacro
    robot_state_publisher_node=Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="both",
        parameters=[robot_description],
    )


    rviz_node=Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="log",
        arguments=[
            "-d",
            PathSubstitution(FindPackageShare("nachi_mz04"))
            / "rviz/moveit.rviz",
        ],
        parameters=[
            moveit_config.robot_description,
            moveit_config.robot_description_semantic,
            moveit_config.robot_description_kinematics,
        ],
        condition=IfCondition(LaunchConfiguration("gui")),
    )



    joint_state_broadcaster_node=Node(
        package="controller_manager",
        executable="spawner",
        parameters=[robot_controllers],
        arguments=[
            "joint_state_broadcaster",
            "--controller-manager",
            "/controller_manager",
        ],
    )
    joint_trajectory_controller_node=Node(
        package="controller_manager",
        executable="spawner",
        parameters=[robot_controllers],
        arguments=[
            "joint_trajectory_controller",
            "--controller-manager",
            "/controller_manager",
        ],
    )

    gripper_controller_node=Node(
        package="controller_manager",
        executable="spawner",
        parameters=[robot_controllers],
        arguments=[
            "gripper_controller",
            "--controller-manager",
            "/controller_manager",
        ],
    )
    
    return LaunchDescription(
        [
            gui_arg,
            use_fake_hardware_arg,
            nachi_ip_arg,


            robot_state_publisher_node,
            world_to_base_tf,

            ros2_control_node,
            joint_state_broadcaster_node,
            joint_trajectory_controller_node,
            #gripper_controller_node,

            moveit_group_node,



            rviz_node,

        ]
    )
