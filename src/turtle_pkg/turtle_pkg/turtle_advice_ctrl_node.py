"""
turtle_advice_ctrl_node.py
功能：创建一个带 PyQt5 图形界面的 ROS2 节点，集成 Publisher 和 Subscriber：
      - Publisher：用户输入线速度和角速度（角度值），点击"发送"后发布 Twist 控制指令
      - Subscriber：订阅 /turtle1/pose 话题，实时显示小乌龟的位姿信息
        （当前X坐标、Y坐标、线速度、角速度、角度）
"""

import sys
import math                                 # 数学库，用于角度与弧度互转
import rclpy                                # ROS2 Python 客户端库
from rclpy.node import Node                 # 节点基类
from geometry_msgs.msg import Twist         # 速度消息类型（包含线速度和角速度）
from turtlesim.msg import Pose              # 小乌龟位姿消息类型
from PyQt5.QtWidgets import (               # PyQt5 界面组件
    QApplication, QWidget, QLabel,
    QLineEdit, QPushButton,
    QVBoxLayout, QHBoxLayout
)
from PyQt5.QtCore import QTimer             # Qt 定时器，用于处理 ROS2 回调


class TurtleAdviceCtrlNode(Node):
    """带图形界面的小乌龟控制节点（含位姿订阅）"""

    def __init__(self):
        # 初始化节点，节点名称为 'turtle_advice_ctrl_node'
        super().__init__('turtle_advice_ctrl_node')

        # 创建发布者：发布 Twist 消息到 /turtle1/cmd_vel 话题，队列深度为 10
        self.publisher_ = self.create_publisher(Twist, '/turtle1/cmd_vel', 10)

        # 创建订阅者：订阅 /turtle1/pose 话题，获取小乌龟实时位姿
        self.subscription_ = self.create_subscription(
            Pose, '/turtle1/pose', self.pose_callback, 10
        )

        # 保存最新的位姿数据，供界面显示
        self.current_pose = Pose()

        self.get_logger().info('Turtle advice control node has been started.')

    def pose_callback(self, msg):
        """位姿订阅回调：保存最新的位姿数据"""
        self.current_pose = msg

    def publish_velocity(self, linear_x, angular_z):
        """构造并发布速度指令"""
        msg = Twist()
        msg.linear.x = linear_x        # 线速度（前进方向），单位 m/s
        msg.angular.z = angular_z       # 角速度（绕 z 轴旋转），单位 rad/s
        self.publisher_.publish(msg)    # 发布消息
        self.get_logger().info(
            f'Publishing: linear.x={linear_x}, angular.z={angular_z}'
        )


class TurtleAdviceWindow(QWidget):
    """小乌龟控制器的 PyQt5 图形界面窗口（含位姿显示）"""

    def __init__(self, node):
        super().__init__()
        self.node = node                # 保存 ROS2 节点引用
        self.init_ui()                  # 初始化界面
        self.init_ros_timer()           # 初始化 ROS2 回调定时器

    def init_ui(self):
        """初始化界面布局：线速度/角速度输入、位姿显示、发送按钮"""
        self.setWindowTitle('小乌龟控制器')     # 设置窗口标题
        self.setFixedSize(350, 300)             # 固定窗口大小

        # ---- 线速度输入行 ----
        label_linear = QLabel('线速度')              # 标签
        self.input_linear = QLineEdit('0.0')        # 输入框，默认值 0.0

        layout_linear = QHBoxLayout()
        layout_linear.addWidget(label_linear)
        layout_linear.addWidget(self.input_linear)

        # ---- 角速度输入行 ----
        label_angular = QLabel('角速度')
        self.input_angular = QLineEdit('0.0')

        layout_angular = QHBoxLayout()
        layout_angular.addWidget(label_angular)
        layout_angular.addWidget(self.input_angular)

        # ---- 位姿信息显示标签 ----
        self.label_x = QLabel('当前X坐标  0.000000')           # X 坐标
        self.label_y = QLabel('当前Y坐标  0.000000')           # Y 坐标
        self.label_linear_vel = QLabel('当前线速度  0.000000')  # 线速度
        self.label_angular_vel = QLabel('当前角速度  0.000000') # 角速度
        self.label_theta = QLabel('当前角度    0.000000')       # 角度（度）

        # ---- 发送按钮 ----
        self.btn_send = QPushButton('发送')
        self.btn_send.clicked.connect(self.on_send_clicked)     # 绑定点击事件

        # ---- 主布局（垂直排列） ----
        main_layout = QVBoxLayout()
        main_layout.addLayout(layout_linear)            # 线速度输入
        main_layout.addLayout(layout_angular)           # 角速度输入
        main_layout.addWidget(self.label_x)             # 当前X坐标
        main_layout.addWidget(self.label_y)             # 当前Y坐标
        main_layout.addWidget(self.label_linear_vel)    # 当前线速度
        main_layout.addWidget(self.label_angular_vel)   # 当前角速度
        main_layout.addWidget(self.label_theta)         # 当前角度
        main_layout.addWidget(self.btn_send)            # 发送按钮
        self.setLayout(main_layout)

    def init_ros_timer(self):
        """创建 Qt 定时器，定期处理 ROS2 回调并刷新界面显示"""
        self.ros_timer = QTimer(self)
        self.ros_timer.timeout.connect(self.spin_and_update)
        self.ros_timer.start(50)        # 每 50ms 处理一次

    def spin_and_update(self):
        """处理一次 ROS2 回调，并刷新界面上的位姿信息"""
        rclpy.spin_once(self.node, timeout_sec=0)       # 非阻塞处理回调
        self.update_pose_display()                       # 刷新位姿显示

    def update_pose_display(self):
        """根据节点中最新的位姿数据，更新界面上的显示标签"""
        pose = self.node.current_pose
        self.label_x.setText(f'当前X坐标  {pose.x:f}')
        self.label_y.setText(f'当前Y坐标  {pose.y:f}')
        self.label_linear_vel.setText(f'当前线速度  {pose.linear_velocity:f}')
        self.label_angular_vel.setText(f'当前角速度  {pose.angular_velocity:f}')
        # 将弧度转为角度显示，与用户输入保持一致
        theta_deg = math.degrees(pose.theta)
        self.label_theta.setText(f'当前角度    {theta_deg:f}')

    def on_send_clicked(self):
        """发送按钮点击事件：读取输入框的值，将角度转为弧度后发布速度指令"""
        try:
            linear_x = float(self.input_linear.text())      # 获取线速度，单位 m/s
            angular_deg = float(self.input_angular.text())   # 获取角速度，单位 度/s
        except ValueError:
            # 输入无效时，记录警告并跳过
            self.node.get_logger().warn('请输入有效的数字！')
            return

        # 将角度值转换为弧度值（ROS2 Twist 消息要求弧度）
        angular_rad = math.radians(angular_deg)

        # 调用节点发布速度指令
        self.node.publish_velocity(linear_x, angular_rad)


def main(args=None):
    """主函数：初始化 ROS2 和 Qt，启动图形界面"""
    rclpy.init(args=args)                           # 初始化 ROS2

    node = TurtleAdviceCtrlNode()                   # 创建 ROS2 节点

    app = QApplication(sys.argv)                    # 创建 Qt 应用
    window = TurtleAdviceWindow(node)               # 创建图形界面窗口，传入节点
    window.show()                                   # 显示窗口

    exit_code = app.exec_()                         # 进入 Qt 事件循环（阻塞）

    node.destroy_node()                             # 窗口关闭后，销毁节点
    rclpy.shutdown()                                # 关闭 ROS2
    sys.exit(exit_code)


if __name__ == '__main__':
    main()
