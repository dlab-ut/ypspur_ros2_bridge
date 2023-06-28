#include "odom_tf_generator/odom_tf_generator_core.hpp"

OdomTFGenerator::OdomTFGenerator() : rclcpp::Node("odom_tf_generator"), tf2_broadcaster_(*this)
{
  odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
    "odom", 1, std::bind(&OdomTFGenerator::odomCallback, this, std::placeholders::_1));
  odom_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);

  declareParams();
  getParams();
}

void OdomTFGenerator::declareParams()
{
  frame_id_ = this->declare_parameter<std::string>("frame_id", "odom");
  child_frame_id_ = this->declare_parameter<std::string>("child_frame_id", "base_link");
}

void OdomTFGenerator::getParams()
{
  this->get_parameter("frame_id", frame_id_);
  this->get_parameter("child_frame_id", child_frame_id_);
}

void OdomTFGenerator::odomCallback(const nav_msgs::msg::Odometry::ConstSharedPtr in_odom)
{
  geometry_msgs::msg::TransformStamped odom_trans;
  rclcpp::Clock ros_clock(RCL_ROS_TIME);

  odom_trans.header.stamp = ros_clock.now();
  odom_trans.header.frame_id = frame_id_;
  odom_trans.child_frame_id = child_frame_id_;
  odom_trans.transform.translation.x = in_odom->pose.pose.position.x;
  odom_trans.transform.translation.y = in_odom->pose.pose.position.y;
  odom_trans.transform.translation.z = 0.0;
  odom_trans.transform.rotation = in_odom->pose.pose.orientation;

  tf2_broadcaster_.sendTransform(odom_trans);
}
