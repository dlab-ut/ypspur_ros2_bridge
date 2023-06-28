#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_broadcaster.h"
#include "tf2_ros/transform_listener.h"

#include <rclcpp/rclcpp.hpp>

#include <nav_msgs/msg/odometry.hpp>

#include <string.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

#include <cmath>


class OdomTFGenerator : public rclcpp::Node
{
public:
  OdomTFGenerator();
  ~OdomTFGenerator(){};

private:
  void odomCallback(const nav_msgs::msg::Odometry::ConstSharedPtr in_odom);
  void declareParams();
  void getParams();

private:
  std::string frame_id_;
  std::string child_frame_id_;

  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  std::shared_ptr<tf2_ros::TransformBroadcaster> odom_broadcaster_;
  tf2_ros::TransformBroadcaster tf2_broadcaster_;
};
