#!/usr/bin/env python3

# Copyright 2025 Rodrigo Pérez-Rodríguez
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    # Declare arguments
    bt_xml_arg = DeclareLaunchArgument(
        'bt_xml',
        default_value=PathJoinSubstitution([
            FindPackageShare('social_bt_nodes'),
            'config',
            'follow_change_example.xml'
        ]),
        description='Path to the behavior tree XML file'
    )
    
    bt_loop_duration_arg = DeclareLaunchArgument(
        'bt_loop_duration',
        default_value='500',
        description='Behavior tree loop duration in milliseconds'
    )
    
    # Get the package directory
    social_bt_nodes_dir = get_package_share_directory('social_bt_nodes')
    
    # Path to the plugin library
    plugin_lib = os.path.join(
        get_package_share_directory('social_bt_nodes'),
        '..',
        '..',
        'lib',
        'libsocial_bt_nodes_plugin.so'
    )
    
    # Behavior tree node
    bt_node = Node(
        package='social_bt_nodes',
        executable='social_bt_nodes_main',
        name='follow_change_bt',
        output='screen',
        parameters=[{
            'bt_xml': LaunchConfiguration('bt_xml'),
            'bt_loop_duration': LaunchConfiguration('bt_loop_duration'),
            'plugin_list': [plugin_lib]
        }]
    )
    
    return LaunchDescription([
        bt_xml_arg,
        bt_loop_duration_arg,
        bt_node
    ])
