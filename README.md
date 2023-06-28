# ypspur_ros2_bridge
## About
This package provides the bridge to use ypspur with ROS 2.  
With this package, you can send `cmd_vel` message and get `odom` message.

## Installation
### 1. git clone
```bash
$ cd <colcon_ws>/src
$ git clone https://github.com/dlab-ut/ypspur_ros2_bridge.git
```

### 2. rosdep
```bash
$ cd <colcon_ws>
$ rosdep install -i --from-paths src
```

### 3. build
```bash
$ cd <colcon_ws>
$ colcon build
$ source install/local_setup.bash
```

## Usage
launch ypspur_ros_bridge
```bash
$ ros2 launch ypspur_ros2_bridge ypspur_ros2_bridge.launch.py
```

See `ypspur_ros_bridge/launch/ypspur_ros_bridge.launch` file, for more detail.  
And the parameter file is inside of `ypspur_ros_bridge/config/` directory.

### Parameters

```
/**:
  ros__parameters:
    serial_port: "/dev/serial/by-id/usb-T-frog_project_T-frog_Driver-if00"
    spur_params_file: "/home/tsukutsuku/tsukutsuku_ws/src/roboken/yp_robot_config/ypspur-robot-params/tsukutsuku.param"
    spur_args: "--admask 10000000 --enable-get-digital-io"
    frame_id: "odom"
    child_frame_id: "base_link"
    left_wheel_joint: "left_wheel_joint"
    right_wheel_joint: "right_wheel_joint"
    linear_vel_max: 2.0 #[m/s] 4[km/h] ~= 1.1[m/s]
    linear_acc_max: 1.0 #[m/s]
    angular_vel_max: 3.14 #[rad/s]
    angular_acc_max: 3.14 #[rad/s]
    pub_hz_: 50.0
```
### publish Topics

#### joint_states(sensor_msgs::msg::JointState)
Publish left/right wheel joint state
so that this package is used with joint_state_publisher

#### odom(nav_msgs::msg::Odometry)
#### twist(geometry_msgs::msg::TwistStamped)
#### pose(geometry_msgs::msg::PoseStamped)

### subscribe Topics

#### /cmd_vel(geometry_msgs::msg::Twist)