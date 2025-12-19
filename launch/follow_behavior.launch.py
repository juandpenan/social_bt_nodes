from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python import get_package_share_directory
import os


def generate_launch_description():
    """
    Launch file for follow behavior only.
    Requires tracker.launch.py to be running separately.
    """
    
    # Get the path to the behavior tree XML file
    config_dir = os.path.join(
        get_package_share_directory('social_bt_nodes'),
        'config'
    )
    bt_xml_file = os.path.join(config_dir, 'follow_behavior_dynamic.xml')
    
    # Get the path to the BT plugin library
    lib_dir = os.path.join(
        get_package_share_directory('social_bt_nodes'),
        '..', '..', 'lib'
    )
    plugin_lib = os.path.join(lib_dir, 'libsocial_bt_nodes_plugin.so')
    
    return LaunchDescription([
        
        # Launch BehaviorTree follow node
        Node(
            package='social_bt_nodes',
            executable='social_bt_nodes_main',
            name='social_bt_nodes_node',
            output='screen',
            parameters=[{
                'bt_xml': bt_xml_file,
                'bt_loop_duration': 500,  # ms
                'plugin_list': [plugin_lib]
            }]
        ),
    ])
