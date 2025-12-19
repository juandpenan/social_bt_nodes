# social_bt_nodes

A ROS 2 package providing reusable BehaviorTree.CPP nodes for social robotics applications. This package offers a plugin-based library of behavior tree nodes for common social robotics tasks including object tracking, following, searching, and obstacle avoidance. While currently implemented and tested on the NAO humanoid robot, the nodes are designed to be generic and adaptable to different robot platforms.

## Overview

The `social_bt_nodes` package provides a plugin-based library of BehaviorTree nodes for social robotics applications. It implements intelligent behaviors for object tracking, autonomous search, and following using **BehaviorTree.CPP**. The package integrates with YOLO-based object detection and uses sensor data (sonar, touch) for obstacle avoidance and interaction control.

**Current Implementation:** The nodes are currently implemented for the NAO humanoid robot but are designed with modularity and reusability in mind for adaptation to other platforms.

**Note:** This README will be updated as new behavior tree nodes are added to the library.

### Key Features

- **Plugin-Based Architecture**: BT nodes are dynamically loadable plugins for maximum reusability across projects
- **Generic and Modular**: Designed for social robotics applications, adaptable to different robot platforms
- **BehaviorTree.CPP Integration**: Industry-standard behavior tree framework for complex robot behaviors
- **Autonomous Search**: Generic search patterns (currently: spinning search for visual targets)
- **Object Detection Integration**: Works with YOLO object detectors via the `yolo_ros` package
- **PID Control**: Advanced motion control with PID controllers for smooth following (FollowDynamic)
- **Multi-Level Obstacle Avoidance**: Configurable obstacle detection and avoidance strategies
- **TF-Based Tracking**: Uses ROS TF frames for target tracking, enabling integration with various perception systems
- **Sensor Integration**: Flexible sensor input handling (currently: sonar for obstacles, touch for control)
- **XML-Configurable**: Behavior trees can be modified via XML without recompiling
- **Multiple Control Strategies**: Proportional and PID control options for different motion requirements

### Current Status & Limitations

**Platform:** Currently implemented and tested on NAO humanoid robot. Future work will focus on making nodes more platform-agnostic.

**Depth Information:** Real depth information has not been fully integrated. The `entity_tracker_fake_3d` helper node uses a fixed distance approach (1 meter) with accurate angular direction from camera intrinsics. This is a temporary solution until proper depth estimation is integrated.

**Node Library:** The package currently provides core motion and perception nodes. Additional nodes for HRI (speech, gestures), navigation, and manipulation will be added as the library expands.

## Architecture

The package uses **BehaviorTree.CPP** for intelligent behavior management:

### BehaviorTree Architecture

```
Root: ReactiveFallback
├── ReactiveSequence (Follow if detected)
│   ├── IsTargetDetected? (Condition)
│   └── Follow/FollowDynamic (Action with obstacle avoidance)
└── SpinSearch (Action - continuous spinning)
```

**Behavior Logic:**
1. **IsTargetDetected**: Checks if target TF frame exists
2. If **TRUE** → **Follow/FollowDynamic**: Robot follows target with integrated obstacle avoidance
3. If **FALSE** → **SpinSearch**: Robot spins in place searching for target (returns RUNNING continuously)

### System Architecture

The complete system workflow:

```
┌──────────────┐
│  YOLO ROS    │ ──> DetectionArray (custom YOLO format)
└──────────────┘
       │
       v
┌──────────────────┐
│ yolo_to_standard │ ──> Detection2DArray
└──────────────────┘
       │
       v
┌───────────────────────┐
│ Entity Tracker Fake3D │ ──> TF: base_link -> target
└───────────────────────┘
       │
       v
┌────────────────────────────────┐
│  social_bt_nodes_main          │
│  ┌──────────────────────────┐  │
│  │ IsTargetDetected         │  │ ──> Checks TF
│  └──────────────────────────┘  │
│  ┌──────────────────────────┐  │       ┌──────────────┐
│  │ SpinSearch               │  │ <──── │ Touch Sensors│
│  └──────────────────────────┘  │       └──────────────┘
│  ┌──────────────────────────┐  │       ┌──────────────┐
│  │ Follow / FollowDynamic   │  │ <──── │ Sonar Sensors│
│  └──────────────────────────┘  │       └──────────────┘
└────────────────────────────────┘
       │
       v
   Twist (cmd_vel)
```

## Nodes

### Main Executable

#### social_bt_nodes_main

Main behavior tree executor that loads BT plugins and runs the XML-defined behavior tree.

**Parameters:**
- `bt_xml` (string, **required**) - Path to the behavior tree XML file
- `bt_loop_duration` (int, default: `100`) - Behavior tree tick period in milliseconds
- `plugin_list` (string array, default: `[]`) - List of plugin library paths to load

**Usage:**
```bash
ros2 run social_bt_nodes social_bt_nodes_main \
  --ros-args -p bt_xml:=/path/to/behavior.xml \
  -p plugin_list:=[/path/to/libsocial_bt_nodes_plugin.so]
```

### BehaviorTree Node Plugins (C++)

All BT nodes are implemented as plugins in `libsocial_bt_nodes_plugin.so` and registered via `BT_REGISTER_NODES` macro:

#### 1. IsTargetDetected (Condition)
Checks if the target TF frame is available.

**XML Name:** `IsTargetDetected`

**Ports:**
- `target_frame` (string, default: `"target"`) - Target TF frame to check
- `base_frame` (string, default: `"base_link"`) - Base TF frame
- `timeout` (double, default: `0.5`) - Time to wait for transform (seconds)

**Returns:**
- `SUCCESS` if target TF is available
- `FAILURE` if target TF is not available

#### 2. SpinSearch (Action)
Rotates the robot in place while searching for the target. Always returns RUNNING (continuous spinning).

**XML Name:** `SpinSearch`

**Ports:**
- `angular_speed` (double, default: `0.5`) - Rotation speed (rad/s)
- `cmd_vel_topic` (string, default: `"/cmd_vel"`) - Velocity command topic
- `touch_topic` (string, default: `"/sensors/touch"`) - Touch sensor topic for stop control

**Returns:**
- `RUNNING` while searching (spinning)

**Note:** Can be stopped via touch sensor input (implementation depends on robot platform).

#### 3. Follow (Action)
Follows the target TF frame with integrated obstacle avoidance using sonar sensors.

**XML Name:** `Follow`

**Ports:**
- `target_frame` (string, default: `"target"`) - Target TF frame to follow
- `base_frame` (string, default: `"base_link"`) - Base TF frame
- `min_distance` (double, default: `1.0`) - Desired distance to target (m)
- `avoidance_distance` (double, default: `0.5`) - Minimum distance to obstacles (m)
- `max_linear_speed` (double, default: `0.5`) - Maximum forward speed (m/s)
- `max_angular_speed` (double, default: `1.0`) - Maximum rotation speed (rad/s)
- `succeed_on_reach` (bool, default: `false`) - Return SUCCESS when target is reached
- `cmd_vel_topic` (string, default: `"/cmd_vel"`) - Velocity command topic
- `sonar_topic` (string, default: `"/sensors/sonar"`) - Sonar sensor topic
- `touch_topic` (string, default: `"/sensors/touch"`) - Touch sensor topic
- `rotation_stop_threshold` (double, default: `0.087`) - Angle threshold to stop rotating (rad, ~5 deg)
- `linear_stop_threshold` (double, default: `0.26`) - Angle threshold to stop linear motion (rad, ~15 deg)

**Returns:**
- `RUNNING` while following
- `SUCCESS` if `succeed_on_reach` is true and target reached
- `FAILURE` if target is lost

#### 4. FollowDynamic (Action)
Advanced follow behavior with PID control and multi-level obstacle avoidance.

**XML Name:** `FollowDynamic`

**Ports:**
- `target_frame` (string, default: `"target"`) - Target TF frame to follow
- `base_frame` (string, default: `"base_link"`) - Base TF frame
- `min_distance` (double, default: `1.0`) - Desired distance to target (m)
- `avoidance_distance` (double, default: `0.5`) - Obstacle avoidance distance (m)
- `danger_distance` (double, default: `0.3`) - Critical obstacle distance - robot stops (m)
- `max_linear_speed` (double, default: `0.5`) - Maximum forward speed (m/s)
- `max_angular_speed` (double, default: `1.0`) - Maximum rotation speed (rad/s)
- `succeed_on_reach` (bool, default: `false`) - Return SUCCESS when target is reached
- `linear_vel_strategy` (string, default: `"proportional"`) - Linear velocity control: "proportional" or "pid"
- `cmd_vel_topic` (string, default: `"/cmd_vel"`) - Velocity command topic
- `sonar_topic` (string, default: `"/sensors/sonar"`) - Sonar sensor topic
- `touch_topic` (string, default: `"/sensors/touch"`) - Touch sensor topic

**Returns:**
- `RUNNING` while following
- `SUCCESS` if `succeed_on_reach` is true and target reached
- `FAILURE` if target is lost

**Features:**
- PID control for smoother motion (when `linear_vel_strategy="pid"`)
- Two-level obstacle avoidance (avoidance + danger zones)
- Better handling of dynamic obstacles

### Python Support Nodes

#### 1. yolo_to_standard

Converts YOLO's custom `DetectionArray` messages to standard ROS `Detection2DArray` and `Detection3DArray` messages.

**Subscribed Topics:**
- `input_detection_2d` (`yolo_msgs/DetectionArray`) - YOLO 2D detections
- `input_detection_3d` (`yolo_msgs/DetectionArray`) - YOLO 3D detections

**Published Topics:**
- `output_detection_2d` (`vision_msgs/Detection2DArray`) - Standard 2D detections
- `output_detection_3d` (`vision_msgs/Detection3DArray`) - Standard 3D detections

#### 2. entity_tracker_fake_3d

Tracks a target object from 2D detections and publishes its position as a TF frame. Uses camera intrinsics to compute the angular direction and places the target at a fixed 1m distance.

**Subscribed Topics:**
- `input_detection_2d` (`vision_msgs/Detection2DArray`) - 2D object detections
- `camera_info` (`sensor_msgs/CameraInfo`) - Camera intrinsics

**Published TF:**
- `source_frame` → `target_frame` (default: `base_link` → `target`)

**Parameters:**
- `target_class` (string, default: `"person"`) - Class name to track
- `source_frame` (string, default: `"base_link"`) - Parent frame for the TF
- `target_frame` (string, default: `"target"`) - Child frame name for the TF
- `optical_frame` (string, default: `"CameraTop_optical_frame"`) - Camera optical frame

**Note:** This node is a Python helper and is NOT a BehaviorTree node. It provides target TF frames that the BT nodes can use.

## Installation

### Prerequisites

This package requires the following dependencies:
- ROS 2 (tested on Humble)
- `behaviortree_cpp` (version 4.x)
- `yolo_ros` package ([mgonzs13/yolo_ros](https://github.com/mgonzs13/yolo_ros)) - for object detection
- `nao_lola_sensor_msgs` package (required for NAO robot; adapt for other platforms)
- Standard ROS 2 packages: `rclcpp`, `rclpy`, `geometry_msgs`, `vision_msgs`, `tf2_ros`

### Build

1. Clone this repository into your ROS 2 workspace:
```bash
cd ~/ros2_ws/src
git clone <repository_url> social_bt_nodes
```

2. Install dependencies:
```bash
cd ~/ros2_ws
rosdep install --from-paths src --ignore-src -r -y
```

3. Build the package:
```bash
colcon build --packages-select social_bt_nodes
source install/setup.bash
```

## Usage

### Launch Files

#### 1. tracker.launch.py

Launches the object detection and tracking pipeline:

```bash
ros2 launch social_bt_nodes tracker.launch.py
```

This launches:
- YOLO detector (`yolo_bringup`)
- `yolo_to_standard` converter
- `entity_tracker_fake_3d` TF publisher

**Default Configuration:**
- Target class: `tv`
- Target frame: `target`
- Source frame: `base_link`
- Optical frame: `CameraTop_optical_frame`

#### 2. follow_behavior.launch.py

Launches the BehaviorTree-based following system:

```bash
ros2 launch social_bt_nodes follow_behavior.launch.py
```

This launches:
- `social_bt_nodes_main` with the behavior tree plugin
- Uses `follow_behavior_dynamic.xml` by default
- Loop duration: 500ms

### Complete System Example

To run a complete following system:

```bash
# Terminal 1: Launch object detection and tracking
ros2 launch social_bt_nodes tracker.launch.py

# Terminal 2: Launch behavior tree
ros2 launch social_bt_nodes follow_behavior.launch.py
```

### Using Different Behavior Trees

Two XML configurations are provided:

1. **follow_behavior.xml** - Basic Follow node
2. **follow_behavior_dynamic.xml** - Advanced FollowDynamic node with PID control

To use a different behavior tree:
```bash
ros2 launch social_bt_nodes follow_behavior.launch.py bt_xml:=/path/to/custom_behavior.xml
```

## Obstacle Avoidance

The motion control nodes support sensor-based obstacle avoidance:

- **Left Sensor**: Detects obstacles on the left; robot turns right to avoid
- **Right Sensor**: Detects obstacles on the right; robot turns left to avoid
- **Both Sensors**: When both detect obstacles, the robot turns away from the closer one
- **Avoidance Distance**: Configurable via the `avoidance_distance` parameter
- **Danger Distance** (FollowDynamic only): Critical distance where robot stops completely

**Current Implementation:** Uses sonar sensors on NAO robot. The nodes subscribe to sonar topics and can be adapted to different sensor modalities (ultrasonic, IR, laser, depth cameras) by providing appropriate sensor data on the configured topics.

The avoidance behavior is combined with the target-following behavior, ensuring the robot doesn't collide with obstacles while pursuing its target.

## Touch/Button Control

The behavior tree nodes support touch/button sensor input for user control:
- **Active**: Robot executes the behavior (following, searching, etc.)
- **Stopped**: Robot stops all motion (publishes zero velocities)

**Current Implementation:** Uses NAO's head touch sensors. Can be adapted to physical buttons, capacitive sensors, or other input methods on different platforms.

## Configuration

### Changing Target Class

Modify the `tracker.launch.py` or pass parameters:

```bash
ros2 launch social_bt_nodes tracker.launch.py target_class:=person
```

Or edit the launch file directly to change the default target class.

### Adjusting Behavior Tree Parameters

Edit the XML files in the `config/` directory:
- `follow_behavior.xml` - Basic following
- `follow_behavior_dynamic.xml` - Advanced following with PID

Example parameters to adjust:
```xml
<Follow 
  name="FollowTarget"
  min_distance="0.5"           <!-- Desired distance to target -->
  avoidance_distance="0.2"     <!-- Obstacle avoidance distance -->
  max_linear_speed="1.0"       <!-- Max forward speed -->
  max_angular_speed="0.5"      <!-- Max rotation speed -->
  cmd_vel_topic="/target"/>
```

### Custom Frames

If using different robot or camera frames, modify the tracker launch file:

```python
parameters=[{
    'source_frame': 'base_footprint',
    'target_frame': 'person_target',
    'optical_frame': 'camera_optical'
}]
```

## Visualization

To visualize the target TF frame and behavior tree execution:

**RViz Visualization:**
```bash
ros2 run rviz2 rviz2
```
Add a TF display to see the `target` frame relative to `base_link`.

**Groot2 Monitoring (optional):**
Uncomment the Groot2 section in `main.cpp` to enable live BehaviorTree monitoring:
```cpp
std::unique_ptr<BT::Groot2Publisher> groot_publisher;
groot_publisher = std::make_unique<BT::Groot2Publisher>(tree);
```
Then rebuild and connect with Groot2 to visualize tree execution in real-time.

## Troubleshooting

### No detections received
- Check that YOLO is running: `ros2 topic echo /yolo/detections`
- Verify topic remappings match your system
- Ensure the target class exists in YOLO's class list
- Check if `yolo_to_standard` is converting properly: `ros2 topic echo /detections_2d`

### Robot not moving
- Check touch sensors: Touch NAO's head to toggle start/stop
- Verify velocity commands: `ros2 topic echo /target`
- Check sonar values: `ros2 topic echo /sensors/sonar`
- Check BT execution: Look for "Tree status" messages in the node logs

### TF errors
- Ensure target TF is published: `ros2 run tf2_ros tf2_echo base_link target`
- Ensure camera frames exist: `ros2 run tf2_ros tf2_echo base_link CameraTop_optical_frame`
- Check frame names match your robot's TF tree
- Verify camera_info is being published: `ros2 topic echo /camera_rgb_info`

### Plugin loading errors
- Verify plugin library exists: `ls -l install/social_bt_nodes/lib/libsocial_bt_nodes_plugin.so`
- Check that `plugin_list` parameter includes the correct path
- Ensure all dependencies are built and sourced

### Behavior tree not ticking
- Check `bt_loop_duration` parameter (in milliseconds)
- Look for error messages in the node logs
- Verify the XML file path is correct and accessible

### Fake depth inaccuracies
- The 1m fixed distance is a workaround; angular direction is accurate
- For precise distance control, integrate real depth information
- Consider using monocular depth estimation (e.g., Depth Anything V2) for better results

## Future Improvements

- [ ] Integration of real depth information from depth cameras
- [ ] Monocular depth estimation using neural networks (e.g., Depth Anything V2)
- [x] PID controller for smoother motion (implemented in FollowDynamic)
- [ ] Multiple target tracking and selection
- [ ] Dynamic obstacle avoidance using additional sensors
- [ ] Path planning integration for complex environments
- [ ] More BT nodes: approach, retreat, orbit, etc.
- [ ] Speech interaction nodes for social HRI
- [ ] Gesture recognition and response nodes

## License

This package is licensed under the Apache License 2.0.

## Authors and Contributors

- Rodrigo Pérez-Rodríguez (rodrigo.perez@urjc.es)

## Acknowledgments

- Developed as part of social robotics research at Universidad Rey Juan Carlos
- Currently implemented and tested on NAO humanoid robot by SoftBank Robotics
- Uses [yolo_ros](https://github.com/mgonzs13/yolo_ros) for object detection
- Built with [BehaviorTree.CPP](https://github.com/BehaviorTree/BehaviorTree.CPP) framework
