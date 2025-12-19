# NAO Follow BT - Plugin Architecture

## Architecture Overview

The package uses a plugin-based architecture:

### Main Components

1. **main.cpp** - Central orchestrator that:
   - Creates a single ROS 2 node (`nao_follow_bt`)
   - Stores the node in the BehaviorTree blackboard
   - Loads BT nodes as plugins from shared library
   - Ticks the behavior tree periodically
   - Handles ROS callbacks with `rclcpp::spin_some()`

2. **BT Node Plugins** - All BT nodes are now plugins:
   - `FollowAction` - Follows target with obstacle avoidance
   - `SpinSearchAction` - Spins to search for target
   - `IsTargetDetectedCondition` - Checks if target TF exists

3. **Plugin Library** - `libnao_follow_bt_plugin.so`:
   - Contains all BT node implementations
   - Registered via `bt_plugins.cpp` using `BT_REGISTER_NODES` macro

### Keys

- Single shared ROS node in blackboard
- BT nodes access node via `config().blackboard->get<rclcpp::Node::SharedPtr>("node")`
- Nodes loaded dynamically as plugins
- Clean separation of concerns

### Benefits

1. **Resource Efficiency**: Only one ROS node instead of 3+
2. **Modularity**: BT nodes are true plugins, loadable at runtime
3. **Cleaner Code**: Removed `bt_follow_node.cpp` - just `main.cpp` orchestrates
4. **Better Architecture**: Follows BT.CPP best practices for ROS integration
5. **Reusability**: Plugin can be loaded by other BT executors

### File Structure

```
src/nao_follow_bt/
├── src/
│   ├── main.cpp                    # Main orchestrator (NEW)
│   └── bt_nodes/
│       ├── bt_plugins.cpp          # Plugin registration (NEW)
│       ├── follow_action.cpp       # Modified to use blackboard
│       ├── spin_search_action.cpp  # Modified to use blackboard
│       └── is_target_detected_condition.cpp  # Modified to use blackboard
├── include/nao_follow_bt/bt_nodes/
│   ├── follow_action.hpp
│   ├── spin_search_action.hpp
│   └── is_target_detected_condition.hpp
├── config/
│   └── follow_behavior.xml
├── launch/
│   └── bt_follow_nao.launch.py    # Updated for new executable
└── CMakeLists.txt                  # Updated for plugin build

REMOVED:
├── src/bt_follow_node.cpp          # Replaced by main.cpp
```

### Launch Parameters

- `bt_xml` (required): Path to behavior tree XML file
- `bt_loop_duration` (default: 100): Tick rate in milliseconds
- `plugin_list` (default: []): List of plugin library paths to load

### Usage

```bash
ros2 launch nao_follow_bt bt_follow_nao.launch.py
```

The launch file automatically:
- Locates the plugin library
- Passes the BT XML path
- Sets up all dependencies (YOLO, entity tracker, etc.)
