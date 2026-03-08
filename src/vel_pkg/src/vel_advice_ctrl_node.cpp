/**
 * vel_advice_ctrl_node.cpp
 * 功能：创建一个带 Qt5 图形界面的 ROS2 节点，集成 Publisher 和 Subscriber：
 *       - Publisher：用户输入线速度和角速度，点击"启动"后持续发布 Twist 控制指令；
 *         点击"停止"后发送零速度停车；关闭窗口时也自动停车。
 *       - Subscriber：订阅 /odom 话题，实时显示小车的位姿信息
 *         （当前X坐标、Y坐标、线速度、角速度、角度）
 *       配合 ros2 launch wpr_simulation2 wpb_simple.launch.py 使用。
 */

#include <rclcpp/rclcpp.hpp>                        // ROS2 C++ 客户端库
#include <geometry_msgs/msg/twist.hpp>              // 速度消息类型
#include <nav_msgs/msg/odometry.hpp>                // 里程计消息类型（含位姿和速度）
#include <QApplication>                             // Qt 应用
#include <QWidget>                                  // Qt 窗口基类
#include <QLabel>                                   // Qt 标签
#include <QLineEdit>                                // Qt 输入框
#include <QPushButton>                              // Qt 按钮
#include <QVBoxLayout>                              // Qt 垂直布局
#include <QHBoxLayout>                              // Qt 水平布局
#include <QTimer>                                   // Qt 定时器
#include <QCloseEvent>                              // Qt 窗口关闭事件
#include <cmath>                                    // 数学函数

/**
 * 从四元数中提取绕 Z 轴的偏航角（yaw），单位为度
 */
static double quaternion_to_yaw_deg(double qx, double qy, double qz, double qw)
{
    // 从四元数计算 yaw 角（弧度）
    double siny_cosp = 2.0 * (qw * qz + qx * qy);
    double cosy_cosp = 1.0 - 2.0 * (qy * qy + qz * qz);
    double yaw_rad = std::atan2(siny_cosp, cosy_cosp);
    // 转换为角度
    return yaw_rad * 180.0 / M_PI;
}

/**
 * 小车 GUI 控制窗口类（含里程计显示）
 * 集成 ROS2 节点和 Qt 界面，点击启动持续发送速度，点击停止发送零速度，
 * 同时实时显示小车当前位姿信息
 */
class VelAdviceCtrlWindow : public QWidget
{
public:
    VelAdviceCtrlWindow(std::shared_ptr<rclcpp::Node> node, QWidget *parent = nullptr)
        : QWidget(parent), node_(node), is_running_(false)
    {
        // 创建发布者：发布 Twist 消息到 /cmd_vel 话题，队列深度为 10
        vel_pub_ = node_->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

        // 创建订阅者：订阅 /odom 话题，获取小车实时里程计数据
        odom_sub_ = node_->create_subscription<nav_msgs::msg::Odometry>(
            "/odom", 10,
            [this](const nav_msgs::msg::Odometry::SharedPtr msg) {
                odom_msg_ = *msg;   // 保存最新的里程计数据
            }
        );

        init_ui();          // 初始化界面
        init_timers();      // 初始化定时器
    }

private:
    /**
     * 初始化界面布局：线速度/角速度输入、位姿显示标签、启动/停止按钮
     */
    void init_ui()
    {
        setWindowTitle("小车控制器");       // 窗口标题
        setFixedSize(350, 320);            // 固定窗口大小

        // ---- 线速度输入行 ----
        auto *label_linear = new QLabel("线速度");
        input_linear_ = new QLineEdit("0.0");

        auto *layout_linear = new QHBoxLayout();
        layout_linear->addWidget(label_linear);
        layout_linear->addWidget(input_linear_);

        // ---- 角速度输入行 ----
        auto *label_angular = new QLabel("角速度");
        input_angular_ = new QLineEdit("0.0");

        auto *layout_angular = new QHBoxLayout();
        layout_angular->addWidget(label_angular);
        layout_angular->addWidget(input_angular_);

        // ---- 位姿信息显示标签 ----
        label_x_           = new QLabel("当前X坐标   0.000000");
        label_y_           = new QLabel("当前Y坐标   0.000000");
        label_linear_vel_  = new QLabel("当前线速度  0.000000");
        label_angular_vel_ = new QLabel("当前角速度  0.000000");
        label_theta_       = new QLabel("当前角度    0.000000");

        // ---- 启动 / 停止按钮（水平排列） ----
        auto *btn_start = new QPushButton("启动");
        auto *btn_stop  = new QPushButton("停止");
        connect(btn_start, &QPushButton::clicked, this, &VelAdviceCtrlWindow::on_start_clicked);
        connect(btn_stop,  &QPushButton::clicked, this, &VelAdviceCtrlWindow::on_stop_clicked);

        auto *layout_buttons = new QHBoxLayout();
        layout_buttons->addWidget(btn_start);
        layout_buttons->addWidget(btn_stop);

        // ---- 主布局（垂直排列） ----
        auto *main_layout = new QVBoxLayout();
        main_layout->addLayout(layout_linear);          // 线速度输入
        main_layout->addLayout(layout_angular);          // 角速度输入
        main_layout->addWidget(label_x_);               // 当前X坐标
        main_layout->addWidget(label_y_);               // 当前Y坐标
        main_layout->addWidget(label_linear_vel_);      // 当前线速度
        main_layout->addWidget(label_angular_vel_);     // 当前角速度
        main_layout->addWidget(label_theta_);           // 当前角度
        main_layout->addLayout(layout_buttons);          // 启动 / 停止按钮
        setLayout(main_layout);
    }

    /**
     * 初始化定时器：
     * - ros_timer_：每 50ms 处理 ROS2 回调并刷新位姿显示
     * - pub_timer_：每 33ms（约30Hz）发布速度指令（启动后才开始）
     */
    void init_timers()
    {
        // ROS2 回调 + 界面刷新定时器
        ros_timer_ = new QTimer(this);
        connect(ros_timer_, &QTimer::timeout, this, &VelAdviceCtrlWindow::spin_and_update);
        ros_timer_->start(50);

        // 速度发布定时器，启动后以 30Hz 持续发送速度指令
        pub_timer_ = new QTimer(this);
        connect(pub_timer_, &QTimer::timeout, this, &VelAdviceCtrlWindow::publish_velocity);
    }

    /**
     * 处理一次 ROS2 回调，并刷新界面上的位姿信息
     */
    void spin_and_update()
    {
        rclcpp::spin_some(node_);       // 处理 ROS2 回调（含 odom 订阅）

        // 从 odom 消息中提取位姿数据
        double x = odom_msg_.pose.pose.position.x;
        double y = odom_msg_.pose.pose.position.y;
        double linear_vel  = odom_msg_.twist.twist.linear.x;
        double angular_vel = odom_msg_.twist.twist.angular.z;
        double theta_deg = quaternion_to_yaw_deg(
            odom_msg_.pose.pose.orientation.x,
            odom_msg_.pose.pose.orientation.y,
            odom_msg_.pose.pose.orientation.z,
            odom_msg_.pose.pose.orientation.w
        );

        // 更新界面标签
        label_x_->setText(QString("当前X坐标   %1").arg(x, 0, 'f', 6));
        label_y_->setText(QString("当前Y坐标   %1").arg(y, 0, 'f', 6));
        label_linear_vel_->setText(QString("当前线速度  %1").arg(linear_vel, 0, 'f', 6));
        label_angular_vel_->setText(QString("当前角速度  %1").arg(angular_vel, 0, 'f', 6));
        label_theta_->setText(QString("当前角度    %1").arg(theta_deg, 0, 'f', 6));
    }

    /**
     * 启动按钮点击事件：读取输入框的值，开始持续发布速度指令
     */
    void on_start_clicked()
    {
        bool ok_linear = false, ok_angular = false;
        linear_x_    = input_linear_->text().toDouble(&ok_linear);
        angular_deg_ = input_angular_->text().toDouble(&ok_angular);

        if (!ok_linear || !ok_angular)
        {
            RCLCPP_WARN(node_->get_logger(), "请输入有效的数字！");
            return;
        }

        is_running_ = true;
        pub_timer_->start(33);      // 约 30Hz 持续发布
        RCLCPP_INFO(node_->get_logger(), "小车启动：linear.x=%.2f, angular=%.1f°",
                     linear_x_, angular_deg_);
    }

    /**
     * 停止按钮点击事件：停止持续发布，并发送零速度指令使小车停下
     */
    void on_stop_clicked()
    {
        pub_timer_->stop();
        is_running_ = false;
        publish_stop();
        RCLCPP_INFO(node_->get_logger(), "小车已停止");
    }

    /**
     * 定时器回调：按照设定的速度值发布 Twist 消息
     */
    void publish_velocity()
    {
        double angular_rad = angular_deg_ * M_PI / 180.0;  // 角度转弧度

        geometry_msgs::msg::Twist vel_msg;
        vel_msg.linear.x  = linear_x_;
        vel_msg.angular.z = angular_rad;
        vel_pub_->publish(vel_msg);
    }

    /**
     * 发送零速度指令，使小车停下来
     */
    void publish_stop()
    {
        geometry_msgs::msg::Twist stop_msg;
        stop_msg.linear.x  = 0.0;
        stop_msg.angular.z = 0.0;
        vel_pub_->publish(stop_msg);
    }

    /**
     * 重写窗口关闭事件：关闭窗口时自动停车
     */
    void closeEvent(QCloseEvent *event) override
    {
        pub_timer_->stop();
        publish_stop();
        RCLCPP_INFO(node_->get_logger(), "窗口关闭，小车已停止");
        event->accept();
    }

    std::shared_ptr<rclcpp::Node> node_;                                        // ROS2 节点
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr vel_pub_;            // 速度发布者
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;         // 里程计订阅者
    nav_msgs::msg::Odometry odom_msg_;                                          // 最新里程计数据
    QLineEdit *input_linear_;                                                   // 线速度输入框
    QLineEdit *input_angular_;                                                  // 角速度输入框
    QLabel *label_x_;                                                           // 当前X坐标标签
    QLabel *label_y_;                                                           // 当前Y坐标标签
    QLabel *label_linear_vel_;                                                  // 当前线速度标签
    QLabel *label_angular_vel_;                                                 // 当前角速度标签
    QLabel *label_theta_;                                                       // 当前角度标签
    QTimer *ros_timer_;                                                         // ROS2 回调定时器
    QTimer *pub_timer_;                                                         // 速度发布定时器
    bool is_running_;                                                           // 是否正在运行
    double linear_x_    = 0.0;                                                  // 当前设定线速度
    double angular_deg_ = 0.0;                                                  // 当前设定角速度（度）
};

/**
 * 主函数：初始化 ROS2 和 Qt，启动图形界面
 */
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);                                           // 初始化 ROS2

    auto node = rclcpp::Node::make_shared("vel_advice_ctrl_node");      // 创建 ROS2 节点

    QApplication app(argc, argv);                                       // 创建 Qt 应用
    VelAdviceCtrlWindow window(node);                                   // 创建 GUI 窗口
    window.show();                                                      // 显示窗口

    RCLCPP_INFO(node->get_logger(), "vel_advice_ctrl_node 已启动");

    int ret = app.exec();                                               // 进入 Qt 事件循环

    rclcpp::shutdown();                                                 // 关闭 ROS2
    return ret;
}
