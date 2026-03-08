/**
 * vehivle_node.cpp
 * 功能：创建一个 ROS2 节点，通过 Publisher 向仿真小车发送速度控制指令，
 *       以 30Hz 的频率持续发布 Twist 消息到 /cmd_vel 话题，使小车匀速前进。
 *       按下 Ctrl+C 后，自动发送零速度指令使小车停下，再退出节点。
 *       配合 ros2 launch wpr_simulation2 wpb_simple.launch.py 使用。
 */

#include <rclcpp/rclcpp.hpp>                    // ROS2 C++ 客户端库
#include <geometry_msgs/msg/twist.hpp>          // 速度消息类型（包含线速度和角速度）

int main(int argc, char **argv)
{
    // 初始化 ROS2
    rclcpp::init(argc, argv);

    // 创建节点，节点名称为 "vehivle_node"
    auto node = rclcpp::Node::make_shared("vehivle_node");

    // 创建发布者：发布 Twist 消息到 /cmd_vel 话题，队列深度为 10
    auto vel_pub = node->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

    // 节点启动提示
    RCLCPP_INFO(node->get_logger(), "vehivle_node 已启动，正在发布速度指令到 /cmd_vel ...");

    // 构造速度消息，设置线速度 x 为 0.1 m/s，其余分量为 0
    auto vel_msg = std::make_shared<geometry_msgs::msg::Twist>();
    vel_msg->linear.x = 0.1;       // 前进方向线速度，单位 m/s
    vel_msg->linear.y = 0.0;
    vel_msg->linear.z = 0.0;
    vel_msg->angular.x = 0.0;
    vel_msg->angular.y = 0.0;
    vel_msg->angular.z = 0.0;      // 角速度为 0，小车直线前进

    // 设置发布频率为 30Hz
    rclcpp::WallRate loop_rate(30);

    // 循环发布速度指令，直到 Ctrl+C 触发 rclcpp::ok() 返回 false
    while (rclcpp::ok())
    {
        vel_pub->publish(*vel_msg);             // 发布速度消息
        rclcpp::spin_some(node);                // 处理回调，保证 Ctrl+C 信号能被正确响应
        loop_rate.sleep();                      // 按频率休眠
    }

    // 退出循环后，发送零速度指令使小车停下来
    auto stop_msg = std::make_shared<geometry_msgs::msg::Twist>();
    stop_msg->linear.x = 0.0;
    stop_msg->linear.y = 0.0;
    stop_msg->linear.z = 0.0;
    stop_msg->angular.x = 0.0;
    stop_msg->angular.y = 0.0;
    stop_msg->angular.z = 0.0;
    vel_pub->publish(*stop_msg);                // 发布停止指令
    RCLCPP_INFO(node->get_logger(), "vehivle_node 已停止，发送零速度指令使小车停下。");

    // 关闭 ROS2
    rclcpp::shutdown();

    return 0;
}
