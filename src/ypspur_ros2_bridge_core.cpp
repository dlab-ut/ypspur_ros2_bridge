#include "ypspur_ros2_bridge/ypspur_ros2_bridge_core.hpp"

YpspurROS2Bridge::YpspurROS2Bridge()
: rclcpp_lifecycle::LifecycleNode("ypspur_ros2_bridge"), tf2_broadcaster_(*this)
{
 declareParams(); 
}

void YpspurROS2Bridge::declareParams()
{
  serial_port_ = this->declare_parameter<std::string>("serial_port", "/dev/serial/by-id/usb-T-frog_project_T-frog_Driver-if00");
  param_file_ = this->declare_parameter<std::string>("spur_params_file", "spur_params_file");
  spur_args_ = this->declare_parameter<std::string>("spur_args", "spur_args");
  left_wheel_joint_ = this->declare_parameter<std::string>("left_wheel_joint", "left_wheel_joint");
  right_wheel_joint_ =
    this->declare_parameter<std::string>("right_wheel_joint", "right_wheel_joint");
  frame_id_ = this->declare_parameter<std::string>("frame_id", "odom");
  child_frame_id_ = this->declare_parameter<std::string>("child_frame_id", "base_link");
  linear_vel_max_ = this->declare_parameter<double>("linear_vel_max", 1.1);
  angular_vel_max_ = this->declare_parameter<double>("angular_vel_max", M_PI);
  linear_acc_max_ = this->declare_parameter<double>("linear_acc_max", 1.0);
  angular_acc_max_ = this->declare_parameter<double>("angular_acc_max", M_PI);
  pub_hz_ = this->declare_parameter<double>("pub_hz_", 25.0);
}

void YpspurROS2Bridge::getParams()
{
  this->get_parameter("serial_port", serial_port_);
  this->get_parameter("spur_params_file", param_file_);
  this->get_parameter("spur_args", spur_args_);
  this->get_parameter("left_wheel_joint", left_wheel_joint_);
  this->get_parameter("right_wheel_joint", right_wheel_joint_);
  this->get_parameter("frame_id", frame_id_);
  this->get_parameter("child_frame_id", child_frame_id_);
  this->get_parameter("linear_vel_max", linear_vel_max_);
  this->get_parameter("angular_vel_max", angular_vel_max_);
  this->get_parameter("linear_acc_max", linear_acc_max_);
  this->get_parameter("angular_acc_max", angular_acc_max_);
  this->get_parameter("pub_hz_", pub_hz_);
}

CallbackReturn YpspurROS2Bridge::on_configure(const rclcpp_lifecycle::State & previous_state)
{
  getParams();
  auto ret = system(nullptr);
  if (ret != 0) {
    ret =
      system(("ypspur-coordinator -p " + param_file_ + " " + "-d " + serial_port_  + " " + spur_args_ + " &").c_str());
    rclcpp::sleep_for(std::chrono::milliseconds(1000));
    RCLCPP_INFO_STREAM(get_logger(), "param file = " << param_file_);
    RCLCPP_INFO_STREAM(get_logger(), "serial port = " << serial_port_);
    RCLCPP_INFO_STREAM(get_logger(), "spur args = " << spur_args_);
  }
  if (ret == -1 || Spur_init() < 0 || YP_get_error_state() != 0) {
    RCLCPP_WARN_STREAM(get_logger(), "ret = " << ret);
    RCLCPP_WARN_STREAM(get_logger(), "Spur_init() = " << Spur_init());
    RCLCPP_WARN_STREAM(get_logger(), "YP_get_error_state() = " << YP_get_error_state());

    RCLCPP_WARN(this->get_logger(), "Can't open ypspur");
    system(("ps aux | grep ypspur-coordinator | grep -v grep | awk \'{ print \"kill -9\", $2 }\' | sh"));
    return CallbackReturn::FAILURE;
  }

  Spur_set_vel(linear_vel_max_);
  Spur_set_accel(linear_acc_max_);
  Spur_set_angvel(angular_vel_max_);
  Spur_set_angaccel(angular_acc_max_);

  js_.name.push_back(left_wheel_joint_);
  js_.name.push_back(right_wheel_joint_);
  js_.position.resize(2);

  double rate = 1.0 / pub_hz_;
  auto timer_control_callback = std::bind(&YpspurROS2Bridge::timerCallback, this);
  auto period_control =
    std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(rate));
  timer_control_ = std::make_shared<rclcpp::GenericTimer<decltype(timer_control_callback)>>(
    get_clock(), period_control, std::move(timer_control_callback),
    this->get_node_base_interface()->get_context());
  this->get_node_timers_interface()->add_timer(timer_control_, nullptr);

  //cmd_vel topic name is on global name space 
  cmd_vel_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
    "/cmd_vel", 1, std::bind(&YpspurROS2Bridge::CmdVelCallback, this, std::placeholders::_1));
  odom_pub_ = this->create_publisher<nav_msgs::msg::Odometry>("odom", rclcpp::QoS{10});
  twist_pub_ = this->create_publisher<geometry_msgs::msg::TwistStamped>("twist", rclcpp::QoS{10});
  pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("pose", rclcpp::QoS{10});
  js_pub_ = this->create_publisher<sensor_msgs::msg::JointState>("joint_states", rclcpp::QoS{10});
  odom_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);

  return CallbackReturn::SUCCESS;
}

CallbackReturn YpspurROS2Bridge::on_cleanup(const rclcpp_lifecycle::State & previous_state)
{
  RCLCPP_INFO(get_logger(), "Cleaning up ypspur");
  Spur_stop();
  rclcpp::sleep_for(std::chrono::seconds(1));
  Spur_free();
  RCLCPP_INFO(get_logger(), "Completed Cleaning up ypspur");
  return CallbackReturn::SUCCESS;
}

CallbackReturn YpspurROS2Bridge::on_deactivate(const rclcpp_lifecycle::State & previous_state)
{
  return CallbackReturn::SUCCESS;
}

CallbackReturn YpspurROS2Bridge::on_shutdown(const rclcpp_lifecycle::State & previous_state)
{
  return CallbackReturn::SUCCESS;
}

CallbackReturn YpspurROS2Bridge::on_activate(const rclcpp_lifecycle::State & previous_state)
{
  if (YP_get_error_state() == 0) {
    odom_pub_->on_activate();
    twist_pub_->on_activate();
    pose_pub_->on_activate();
    js_pub_->on_activate();
    return CallbackReturn::SUCCESS;
  } else {
    RCLCPP_WARN(get_logger(), "Ypspur got some error states");
  }
}

void YpspurROS2Bridge::CmdVelCallback(const geometry_msgs::msg::Twist::ConstSharedPtr cmd_vel)
{
  Spur_vel(cmd_vel->linear.x, cmd_vel->angular.z);
}

void YpspurROS2Bridge::timerCallback()
{
  geometry_msgs::msg::Quaternion odom_quat;
  geometry_msgs::msg::TransformStamped odom_trans;
  nav_msgs::msg::Odometry odom;
  geometry_msgs::msg::TwistStamped twist;
  geometry_msgs::msg::PoseStamped pose;
  rclcpp::Clock ros_clock(RCL_ROS_TIME);

  Spur_get_pos_GL(&x_, &y_, &th_);

  Spur_get_vel(&vx_, &vth_);

  YP_get_wheel_ang(&l_ang_, &r_ang_);

  tf2::Quaternion yp_quat;
  yp_quat.setRPY(0, 0, th_);
  odom_quat = tf2::toMsg(yp_quat);

  odom_trans.header.stamp = ros_clock.now();
  odom_trans.header.frame_id = frame_id_;
  odom_trans.child_frame_id = child_frame_id_;
  odom_trans.transform.translation.x = x_;
  odom_trans.transform.translation.y = y_;
  odom_trans.transform.translation.z = 0.0;
  odom_trans.transform.rotation = odom_quat;

  tf2_broadcaster_.sendTransform(odom_trans);

  odom.header.stamp = ros_clock.now();
  odom.header.frame_id = frame_id_;
  odom.child_frame_id = child_frame_id_;
  odom.pose.pose.position.x = x_;
  odom.pose.pose.position.y = y_;
  odom.pose.pose.position.z = 0.0;
  odom.pose.pose.orientation = odom_quat;

  odom.twist.twist.linear.x = vx_;
  odom.twist.twist.linear.y = vy_;
  odom.twist.twist.angular.z = vth_;

  odom_pub_->publish(odom);

  twist.header.stamp = ros_clock.now();
  twist.header.frame_id = child_frame_id_;
  twist.twist.linear.x = vx_;
  twist.twist.linear.y = vy_;
  twist.twist.angular.z = vth_;
  twist_pub_->publish(twist);

  pose.header.stamp = ros_clock.now();
  pose.header.frame_id = frame_id_;
  pose.pose.position = odom.pose.pose.position;
  pose.pose.orientation = odom.pose.pose.orientation;
  pose_pub_->publish(pose);

  js_.header.stamp = ros_clock.now();
  js_.name.push_back(left_wheel_joint_);
  js_.name.push_back(right_wheel_joint_);
  js_.position.push_back(l_ang_);
  js_.position.push_back(r_ang_);
  js_pub_->publish(js_);
}
