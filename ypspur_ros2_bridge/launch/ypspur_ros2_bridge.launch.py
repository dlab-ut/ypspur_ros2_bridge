import os
import sys

from ament_index_python.packages import get_package_share_directory

import launch_ros.actions
import launch_ros.events
import launch_ros.events.lifecycle
import lifecycle_msgs.msg

import launch
import launch.actions
import launch.events
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument


def generate_launch_description():
    """Run lifecycle nodes via launch."""
    ld = launch.LaunchDescription()

    params_file = LaunchConfiguration('params_file')

    root_dir = get_package_share_directory('ypspur_ros2_bridge')
    declare_params_file_cmd = DeclareLaunchArgument(
        'params_file',
        default_value=os.path.join(root_dir, 'config/tsukutsuku', 'ypspur_ros2_bridge_params.yaml'),
        description='Params file for ypspur ros2 bridge')

    ypspur_ros2_bridge_node = launch_ros.actions.LifecycleNode(
        name="ypspur_ros2_bridge",
        namespace="",
        package="ypspur_ros2_bridge",
        executable="ypspur_ros2_bridge",
        output="screen",
        parameters=[launch_ros.parameter_descriptions.ParameterFile(params_file, allow_substs=True)]
    )

    configure_trans_event = launch.actions.EmitEvent(
        event=launch_ros.events.lifecycle.ChangeState(
            lifecycle_node_matcher=launch.events.matches_action(
                ypspur_ros2_bridge_node
            ),
            transition_id=lifecycle_msgs.msg.Transition.TRANSITION_CONFIGURE,
        )
    )

    activate_trans_event = launch.actions.EmitEvent(
        event=launch_ros.events.lifecycle.ChangeState(
            lifecycle_node_matcher=launch.events.matches_action(
                ypspur_ros2_bridge_node
            ),
            transition_id=lifecycle_msgs.msg.Transition.TRANSITION_ACTIVATE,
        )
    )

    # When the ypspur ros2 bridge node reaches the 'unconfigured' state, transite to inactivate.
    unconfigured_state_handler = launch.actions.RegisterEventHandler(
        launch_ros.event_handlers.OnStateTransition(
            target_lifecycle_node=ypspur_ros2_bridge_node,
            goal_state="unconfigured",
            entities=[
                launch.actions.LogInfo(
                    msg="node 'ypspur_ros2_bridge' reached the 'unconfigured' state, 'inactivating'."
                ),
                configure_trans_event,
            ],
        )
    )

    inactive_state_handler = launch.actions.RegisterEventHandler(
        launch_ros.event_handlers.OnStateTransition(
            target_lifecycle_node=ypspur_ros2_bridge_node,
            goal_state="inactive",
            entities=[
                launch.actions.LogInfo(
                    msg="node 'ypspur_ros2_bridge' reached the 'inactive' state, 'activating'."
                ),
                activate_trans_event,
            ],
        )
    )

    active_state_handler = launch.actions.RegisterEventHandler(
        launch_ros.event_handlers.OnStateTransition(
            target_lifecycle_node=ypspur_ros2_bridge_node,
            goal_state="active",
            entities=[
                launch.actions.LogInfo(
                    msg="node 'ypspur_ros2_bridge' reached the 'active' state."
                ),
            ],
        )
    )

    # Add the actions to the launch description.
    # The order they are added reflects the order in which they will be executed.
    ld.add_action(declare_params_file_cmd)
    ld.add_action(active_state_handler)
    ld.add_action(inactive_state_handler)
    ld.add_action(unconfigured_state_handler)
    ld.add_action(ypspur_ros2_bridge_node)
    ld.add_action(configure_trans_event)

    return ld


if __name__ == "__main__":
    generate_launch_description()
