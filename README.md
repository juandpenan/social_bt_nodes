# social_bt_nodes

ROS 2 package providing reusable **BehaviorTree.CPP** action and condition nodes for social robotics applications. The nodes cover HRI (speech, listening, extraction), motion (following, navigation, searching), perception (target detection and configuration), and utility tasks. They are currently implemented and tested on the **NAO humanoid robot** but are designed to be platform-agnostic and adaptable to other robots.

---

## Package contents

```
social_bt_nodes/
├── include/social_bt_nodes/bt_nodes/
│   ├── interaction/      # Speak, SpeakEnum, Listen, Confirmation, Extract, NaoPosition, NaoSetLeds
│   ├── motion/           # Follow, FollowDynamic, SpinSearch, NavigateTo
│   ├── perception/       # IsTargetDetected, SetPerceptionTarget
│   └── support/          # SetRos2Param
├── src/bt_nodes/         # corresponding .cpp implementations
├── config/               # example BT XML files
├── launch/               # example launch files
└── node_descriptions/
    └── social_bt_nodes.yaml   # descriptions consumed by llm_bt_builder RAG
```

The package builds a single shared library **`libsocial_bt_nodes_plugin.so`** and a standalone executable **`social_bt_nodes_main`**.

---

## Available nodes

### Interaction

| Node | Type | Required ports | Output ports |
|---|---|---|---|
| `Speak` | Action | `text` | — |
| `SpeakEnum` | Action | `text` | — |
| `Listen` | Action | — | `transcribed_text` |
| `Confirmation` | Action | `text` | — |
| `Extract` | Action | `interest`, `text` | `extracted_info` |
| `NaoPosition` | Action | `action_name` | `success` |
| `NaoSetLeds` | Action | `led_ids`, `mode` | `success` |

### Motion

| Node | Type | Required ports |
|---|---|---|
| `Follow` | Action | — (all optional, reads TF) |
| `FollowDynamic` | Action | — (all optional, reads TF) |
| `SpinSearch` | Action | — |
| `NavigateTo` | Action | `x`+`y` **or** `target_frame` |

### Perception

| Node | Type | Required ports |
|---|---|---|
| `IsTargetDetected` | Condition | `target_frame`, `base_frame` |
| `SetPerceptionTarget` | Action | `target_class` |

### Support

| Node | Type | Required ports |
|---|---|---|
| `SetRos2Param` | Action | `node_name`, `param_name`, `param_value` |

---

## Node reference

### `Speak`
Synthesizes and speaks the specified text using a TTS service.

| Port | Dir | Type | Default | Description |
|---|---|---|---|---|
| `text` | In | string | **required** | Text to speak |
| `service_name` | In | string | `/tts_service` | TTS service name |
| `timeout` | In | int | `5000` | Max wait (ms) |

Returns `SUCCESS` when speech completes, `RUNNING` while speaking, `FAILURE` if service unavailable or `text` missing.

---

### `SpeakEnum`
Enumerates and speaks a list of items with proper language conjunction (e.g. "a, b and c").

| Port | Dir | Type | Default | Description |
|---|---|---|---|---|
| `text` | In | string | **required** | Items separated by `separator` |
| `separator` | In | string | `,` | Item separator |
| `language` | In | string | `en` | `en` or `es` |
| `service_name` | In | string | `/tts_service` | TTS service name |
| `timeout` | In | int | `5000` | Max wait (ms) |

---

### `Listen`
Listens and transcribes speech to text using an STT service.

| Port | Dir | Type | Default | Description |
|---|---|---|---|---|
| `service_name` | In | string | `/stt_service` | STT service name |
| `timeout` | In | int | `10000` | Max wait (ms) |
| `transcribed_text` | Out | string | — | Transcribed text |

---

### `Confirmation`
Analyzes input text for yes/no responses. Returns `SUCCESS` only if the user confirms (YES).

| Port | Dir | Type | Default | Description |
|---|---|---|---|---|
| `text` | In | string | **required** | Text to analyze |
| `service_name` | In | string | `/yesno_service` | Service name |
| `timeout` | In | int | `10000` | Max wait (ms) |

---

### `Extract`
Extracts a specific piece of information from a text using an LLM service.

| Port | Dir | Type | Default | Description |
|---|---|---|---|---|
| `interest` | In | string | **required** | What to extract (single value) |
| `text` | In | string | **required** | Source text |
| `service_name` | In | string | `/extract_service` | Service name |
| `timeout` | In | int | `5000` | Max wait (ms) |
| `extracted_info` | Out | string | — | Extracted result |

---

### `NaoPosition`
Executes a predefined movement action on the NAO robot via action server.

| Port | Dir | Type | Default | Description |
|---|---|---|---|---|
| `action_name` | In | string | **required** | Movement name (e.g. `hello`, `sit`) |
| `action_server` | In | string | `/nao_pos_server` | Action server name |
| `timeout` | In | float | — | Max wait (s) |
| `success` | Out | bool | — | Whether action succeeded |

---

### `NaoSetLeds`
Controls NAO LED groups via action server.

| Port | Dir | Type | Default | Description |
|---|---|---|---|---|
| `led_ids` | In | string | **required** | LED IDs (e.g. `"0,1"`) |
| `mode` | In | int | **required** | `0`=steady, `1`=blink |
| `color_r/g/b` | In | float | — | RGB components (0.0–1.0) |
| `intensity` | In | float | — | LED intensity |
| `frequency` | In | float | — | Blink frequency |
| `duration` | In | float | — | Duration (s) |
| `action_server` | In | string | `leds_play` | Action server name |
| `timeout` | In | float | — | Max wait (s) |
| `success` | Out | bool | — | Whether action succeeded |

---

### `IsTargetDetected`
Condition: checks whether a target TF frame exists and is recent.

| Port | Dir | Type | Default | Description |
|---|---|---|---|---|
| `target_frame` | In | string | **required** | TF frame to check |
| `base_frame` | In | string | **required** | Reference frame |
| `timeout` | In | float | `0.5` | Max age of the transform (s) |

Returns `SUCCESS` if the transform exists and is fresh, `FAILURE` otherwise.

---

### `SetPerceptionTarget`
Configures the perception system to detect a specific object class. When detected, a TF frame `perception_target` is published.

| Port | Dir | Type | Default | Description |
|---|---|---|---|---|
| `target_class` | In | string | **required** | Class to detect (e.g. `person`, `bottle`) |
| `service_name` | In | string | `/set_perception_target` | Service name |
| `timeout` | In | int | — | Max wait (ms) |
| `success` | Out | bool | — | Whether call succeeded |
| `message` | Out | string | — | Status message |

---

### `Follow`
Follows a target TF frame using proportional control with sonar-based obstacle avoidance and two-phase motion (rotate then advance).

| Port | Dir | Type | Default |
|---|---|---|---|
| `target_frame` | In | string | `target` |
| `base_frame` | In | string | `base_link` |
| `min_distance` | In | float | `1.0` m |
| `avoidance_distance` | In | float | `0.5` m |
| `max_linear_speed` | In | float | `0.5` m/s |
| `max_angular_speed` | In | float | `1.0` rad/s |
| `succeed_on_reach` | In | bool | `false` |
| `cmd_vel_topic` | In | string | `/cmd_vel` |
| `sonar_topic` | In | string | `/sensors/sonar` |
| `touch_topic` | In | string | `/sensors/touch` |
| `rotation_stop_threshold` | In | float | `0.087` rad |
| `linear_stop_threshold` | In | float | `0.26` rad |

Returns `RUNNING` while following, `SUCCESS` if `succeed_on_reach=true` and target reached, `FAILURE` if TF lost.

---

### `FollowDynamic`
Like `Follow` but uses PID control for both linear and angular velocities, with a danger distance that stops the robot immediately.

Extra ports vs `Follow`:

| Port | Dir | Type | Default |
|---|---|---|---|
| `danger_distance` | In | float | `0.3` m |
| `linear_vel_strategy` | In | string | `proportional` (`proportional`\|`pid`) |

---

### `SpinSearch`
Spins the robot in place continuously until the behavior tree halts it (typically when `IsTargetDetected` succeeds in the parent).

| Port | Dir | Type | Default |
|---|---|---|---|
| `angular_speed` | In | float | `0.5` rad/s |
| `cmd_vel_topic` | In | string | `/cmd_vel` |
| `touch_topic` | In | string | `/sensors/touch` |

Always returns `RUNNING`. Stops rotation when touch sensor is pressed but continues returning `RUNNING`.

---

### `NavigateTo`
Navigates to a goal pose using Nav2. Accepts explicit coordinates or a TF frame as destination.

| Port | Dir | Type | Default |
|---|---|---|---|
| `x` | In | float | — |
| `y` | In | float | — |
| `yaw` | In | float | `0.0` |
| `target_frame` | In | string | — (alternative to x/y/yaw) |
| `frame_id` | In | string | `map` |
| `action_name` | In | string | `navigate_to_pose` |
| `timeout` | In | float | `300.0` s |
| `error_msg` | Out | string | — |

---

### `SetRos2Param`
Sets a parameter on any running ROS 2 node using the parameter service.

| Port | Dir | Type | Default |
|---|---|---|---|
| `node_name` | In | string | **required** |
| `param_name` | In | string | **required** |
| `param_value` | In | string | **required** |
| `param_type` | In | string | `string` (`string`\|`int`\|`double`\|`bool`) |
| `timeout` | In | int | — |
| `success` | Out | bool | — |

---

## `social_bt_nodes_main` executable

Standalone executor that loads BT plugins and runs an XML-defined behavior tree.

**Parameters:**
- `bt_xml` (string, **required**) — path to the behavior tree XML file
- `bt_loop_duration` (int, default `100`) — tick period in ms
- `plugin_list` (string array, default `[]`) — plugin `.so` paths to load

```bash
ros2 run social_bt_nodes social_bt_nodes_main \
  --ros-args \
  -p bt_xml:=$(ros2 pkg prefix social_bt_nodes)/share/social_bt_nodes/config/follow_behavior.xml \
  -p plugin_list:=[libsocial_bt_nodes_plugin.so]
```

---

## Example behavior trees

| XML file | Description |
|---|---|
| `follow_behavior.xml` | Follow target (proportional control) |
| `follow_behavior_dynamic.xml` | Follow target (PID control) |
| `follow_change_example.xml` | Follow with dynamic target class change |
| `restaurant_order.xml` | Take a table order (Speak → Listen → Confirm loop) |
| `nao_hello_example.xml` | Make NAO perform the "hello" pose |
| `nao_leds_demo.xml` | LED control demo |
| `navigate_to_example.xml` | Navigate to a fixed pose |

---

## Example launches

| Launch file | Description |
|---|---|
| `follow_behavior.launch.py` | Full follow pipeline (tracker + BT) |
| `restaurant_demo.launch.py` | Restaurant order-taking demo |
| `nao_hello_demo.launch.py` | NAO hello gesture demo |
| `nao_leds_demo.launch.py` | NAO LED demo |
| `tracker.launch.py` | Object tracker only (entity_tracker_fake_3d) |

---

## Building

```bash
colcon build --packages-select social_bt_nodes
source install/setup.bash
```

Key dependencies: `rclcpp`, `rclcpp_action`, `behaviortree_cpp`, `tf2_ros`, `nav2_msgs`, `nao_lola_sensor_msgs`, `simple_hri_interfaces`, `simple_perception_interfaces`, `nao_pos_interfaces`, `nao_led_interfaces`.

---

## Using the plugin in another package

Load dynamically from a BehaviorTree.CPP factory:

```cpp
factory.registerFromPlugin("libsocial_bt_nodes_plugin.so");
```

Or declare it in a `behavior_architecture` YAML config:

```yaml
plugin_libraries:
  - "libsocial_bt_nodes_plugin.so"
```

The `node_descriptions/social_bt_nodes.yaml` is installed under
`share/social_bt_nodes/node_descriptions/` and auto-resolved by
`llm_bt_builder` when you set:

```yaml
bt_nodes_package: "social_bt_nodes"
```

---

## Notes

- **Depth:** `entity_tracker_fake_3d` uses a fixed 1 m depth with accurate angular direction from camera intrinsics. Full depth integration (e.g. via `depth_anything_v2_ros2`) is planned.
- **Platform:** NAO-specific nodes (`NaoPosition`, `NaoSetLeds`, sonar/touch topics) require the corresponding NAO ROS 2 drivers. All other nodes are platform-agnostic.

---

## License

Apache License 2.0

## Author

Rodrigo Pérez-Rodríguez (rodrigo.perez@urjc.es)
