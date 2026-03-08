"""
turtle_ctr_node.py
功能：创建一个 ROS2 节点，通过 Publisher 向 turtlesim 小乌龟发送速度控制指令，
      使小乌龟以固定的线速度和角速度运动（画圆）。
"""

import rclpy                            # ROS2 Python 客户端库
from rclpy.node import Node             # 节点基类
from geometry_msgs.msg import Twist     # 速度消息类型（包含线速度和角速度）


class TurtleControlNode(Node):
    """小乌龟速度控制节点"""

    def __init__(self):
        # 初始化节点，节点名称为 'turtle_ctr_node'
        super().__init__('turtle_ctr_node')

        # 创建发布者：发布 Twist 消息到 /turtle1/cmd_vel 话题，队列深度为 10
        self.publisher_ = self.create_publisher(Twist, '/turtle1/cmd_vel', 10)

        # 创建定时器：每 0.5 秒调用一次 timer_callback 发送速度指令
        self.timer = self.create_timer(0.5, self.timer_callback)

        self.get_logger().info('Turtle control node has been started.')

    def timer_callback(self):
        """定时器回调函数：构造并发布速度指令"""
        msg = Twist()
        msg.linear.x = 1.0      # 线速度（前进方向），单位 m/s
        msg.angular.z = 0.5     # 角速度（绕 z 轴旋转），单位 rad/s
        self.publisher_.publish(msg)    # 发布消息
        self.get_logger().info(
            f'Publishing: linear.x={msg.linear.x}, angular.z={msg.angular.z}'
        )


def main(args=None):
    """主函数：初始化 ROS2、创建节点、保持运行、退出时清理资源"""
    rclpy.init(args=args)           # 初始化 ROS2
    node = TurtleControlNode()      # 创建节点实例
    rclpy.spin(node)                # 保持节点运行，循环处理回调
    node.destroy_node()             # 销毁节点
    rclpy.shutdown()                # 关闭 ROS2


if __name__ == '__main__':
    main()
