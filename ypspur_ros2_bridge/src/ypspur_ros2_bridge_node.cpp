#include "ypspur_ros2_bridge/ypspur_ros2_bridge_core.hpp"

int main(int argc, char** argv){

  // init ros
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<YpspurROS2Bridge>()->get_node_base_interface());
  rclcpp::shutdown();
}