#include "odom_tf_generator/odom_tf_generator_core.hpp"

int main(int argc, char** argv){

  // init ros
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<OdomTFGenerator>()->get_node_base_interface());
  rclcpp::shutdown();
}