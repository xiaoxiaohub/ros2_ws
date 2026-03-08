/**
 * vel_node.cpp
 * 功能：创建一个 ROS2 节点，通过 Publisher 向仿真小车发送速度控制指令，
 *       以 30Hz 的频率持续发布 Twist 消息到 /cmd_vel 话题，使小车匀速前进。
 *       按下 Ctrl+C 后，先发送零速度指令使小车停下，再退出。
 *       配合 ros2 launch wpr_simulation2 wpb_simple.launch.py 使用。
 */

#include <rclcpp/rclcpp.hpp>                    // ROS2 C++ 客户端库
#include <geometry_msgs/msg/twist.hpp>          // 速度消息类型（包含线速度和角速度）
#include <signal.h>                             // 信号处理，用于捕获 Ctrl+C

// 全局变量：标记是否收到退出信号
static bool g_running = true;

// 全局指针：指向发布者，供信号处理函数使用
static rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr g_vel_pub = nullptr;

/**
 * 信号处理函数：捕获 Ctrl+C (SIGINT)
 * 在 ROS2 关闭之前，先发送零速度指令使小车停下来
 */
void sigint_handler(int sig)
{
    (void)sig;  // 避免未使用参数警告

    // 发送零速度指令，使小车停下来
    if (g_vel_pub != nullptr)
    {
        geometry_msgs::msg::Twist stop_msg;
        stop_msg.linear.x = 0.0;
        stop_msg.linear.y = 0.0;
        stop_msg.linear.z = 0.0;
        stop_msg.angular.x = 0.0;
        stop_msg.angular.y = 0.0;
        stop_msg.angular.z = 0.0;
        g_vel_pub->publish(stop_msg);
        printf("\nvel_node 已停止，发送零速度指令使小车停下。\n");
    }

    // 标记退出循环
    g_running = false;
}

int main(int argc, char **argv)
{
    // 初始化 ROS2
    rclcpp::init(argc, argv);

    // 创建节点，节点名称为 "vel_node"
    auto node = std::make_shared<rclcpp::Node>("vel_node");

    // 创建发布者：发布 Twist 消息到 /cmd_vel 话题，队列深度为 10
    g_vel_pub = node->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

    // 注册信号处理函数，捕获 Ctrl+C，在关闭前发送停止指令
    signal(SIGINT, sigint_handler);

    // 节点启动提示
    RCLCPP_INFO(node->get_logger(), "vel_node 已启动，正在发布速度指令到 /cmd_vel ...");

    // 构造速度消息，设置线速度 x 为 0.1 m/s，其余分量为 0
    geometry_msgs::msg::Twist vel_msg;
    vel_msg.linear.x = 0.1;        // 前进方向线速度，单位 m/s
    vel_msg.linear.y = 0.0;
    vel_msg.linear.z = 0.0;
    vel_msg.angular.x = 0.0;
    vel_msg.angular.y = 0.0;
    vel_msg.angular.z = 0.0;       // 角速度为 0，小车直线前进

    // 设置发布频率为 30Hz
    rclcpp::Rate loop_rate(30);

    // 循环发布速度指令，直到收到退出信号
    while (g_running && rclcpp::ok())
    {
        g_vel_pub->publish(vel_msg);  // 发布速度消息
        loop_rate.sleep();          // 按频率休眠
    }

    // 关闭 ROS2
    rclcpp::shutdown();

    return 0;
}
