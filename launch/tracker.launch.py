from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
import os

def generate_launch_description():
    """
    Launch file for object tracking components only.
    Includes entity tracker, YOLO converter, and YOLO detector.
    """
    
    return LaunchDescription([
        
        # Launch entity tracker to publish target TF
        Node(
            package='social_bt_nodes',
            executable='entity_tracker_fake_3d.py',
            name='entity_tracker_node',
            output='screen',
            parameters=[{
                'target_class': 'person',
                'source_frame': 'base_link',
                'target_frame': 'target',
                'optical_frame': 'CameraTop_optical_frame'
            }],
            remappings=[
                ('/input_detection_2d', '/detections_2d'),
                ('/camera_info', '/camera_rgb_info')
            ]
        ),
        
        # Launch YOLO to standard converter
        Node(
            package='social_bt_nodes',
            executable='yolo_to_standard.py',
            name='yolo_to_standard_node',
            output='screen',
            remappings=[
                ('input_detection_3d', '/yolo/detections_3d'),
                ('input_detection_2d', '/yolo/detections'),
                ('output_detection_3d', '/detections_3d'),
                ('output_detection_2d', '/detections_2d')
            ]
        ),
        
        # Launch YOLO detector
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(
                os.path.join(
                    get_package_share_directory('yolo_bringup'),
                    'launch',
                    'yolo.launch.py'
                )
            ),
            launch_arguments={
                'input_image_topic': '/image_rgb',
                'input_depth_topic': '/image_depth',
                'input_depth_info_topic': '/camera_rgb_info',
                'target_frame': 'CameraTop_frame',
                'depth_image_reliability': '1'
            }.items()
        ),
    ])
