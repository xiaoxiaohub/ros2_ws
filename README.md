# ROS2 学习工作空间 (ros2_ws)

基于 **ROS2 Humble** 的学习与实践工作空间，包含基础入门、小乌龟控制、仿真小车控制、机械臂 URDF 可视化等多个功能包。

---

## 目录

- [环境要求](#环境要求)
- [编译](#编译)
- [功能包总览](#功能包总览)
- [使用方法](#使用方法)
  - [1. my_package — Python 入门节点](#1-my_package--python-入门节点)
  - [2. my_c_package — C++ 入门节点](#2-my_c_package--c-入门节点)
  - [3. turtle_pkg — 小乌龟控制](#3-turtle_pkg--小乌龟控制)
  - [4. vehicle_pkg — 仿真小车速度控制 (C++)](#4-vehicle_pkg--仿真小车速度控制-c)
  - [5. vel_pkg — 仿真小车进阶控制 (C++ / Qt5 GUI)](#5-vel_pkg--仿真小车进阶控制-c--qt5-gui)
  - [6. dummy_ros2 — 6DOF 机械臂 URDF 可视化](#6-dummy_ros2--6dof-机械臂-urdf-可视化)
  - [7. wpr_simulation2 — WPB 机器人 Gazebo 仿真环境](#7-wpr_simulation2--wpb-机器人-gazebo-仿真环境)

---

## 环境要求

| 项目 | 版本 |
|------|------|
| 操作系统 | Ubuntu 22.04 |
| ROS2 | Humble Hawksbill |
| Gazebo | Gazebo Classic (11) |
| Qt5 | libqt5-dev (`sudo apt install qtbase5-dev`) |
| PyQt5 | `pip install PyQt5` 或 `sudo apt install python3-pyqt5` |

## 编译

```bash
# 1. 进入工作空间
cd ~/ros2_ws

# 2. 加载 ROS2 环境
source /opt/ros/humble/setup.bash

# 3. 编译所有功能包
colcon build

# 4. 加载本工作空间环境（每次新开终端都需要执行）
source install/setup.bash
```

> **提示：** 如需仅编译单个包，可使用：
> ```bash
> colcon build --packages-select <包名>
> ```

---

## 功能包总览

| 包名 | 语言 | 说明 |
|------|------|------|
| `my_package` | Python | ROS2 入门：Hello World 节点 |
| `my_c_package` | C++ | ROS2 入门：C++ Hello World 节点 |
| `turtle_pkg` | Python | 小乌龟 turtlesim 控制（命令行 / GUI / GUI+位姿显示） |
| `vehicle_pkg` | C++ | 仿真小车匀速前进控制 |
| `vel_pkg` | C++ | 仿真小车 GUI 控制（Qt5 界面，含里程计显示） |
| `dummy_ros2` | URDF | 6DOF 机械臂模型可视化（rviz2 / Gazebo） |
| `wpr_simulation2` | C++/Python | WPB 机器人 Gazebo 仿真环境（第三方） |

---

## 使用方法

> ⚠️ **以下所有命令均需先在终端中执行：**
> ```bash
> source /opt/ros/humble/setup.bash
> source ~/ros2_ws/install/setup.bash
> ```

---

### 1. my_package — Python 入门节点

最基础的 ROS2 Python 包，包含两个简单的打印节点。

```bash
# 运行 hello_node（输出 "Hello, ROS2!"）
ros2 run my_package hello_node

# 运行 my_node（输出 "Hi I am a students."）
ros2 run my_package my_node
```

---

### 2. my_c_package — C++ 入门节点

最基础的 ROS2 C++ 包，输出一句问候语。

```bash
ros2 run my_c_package my_c_node
```

---

### 3. turtle_pkg — 小乌龟控制

提供三种方式控制 turtlesim 小乌龟，功能逐级递进。

#### 3.1 命令行控制（自动画圆）

```bash
# 终端1：启动小乌龟仿真
ros2 run turtlesim turtlesim_node

# 终端2：运行控制节点，小乌龟会以固定线速度和角速度画圆
ros2 run turtle_pkg turtle_ctr_node
```

#### 3.2 PyQt5 GUI 控制

```bash
# 终端1：启动小乌龟仿真
ros2 run turtlesim turtlesim_node

# 终端2：启动 GUI 控制界面，输入线速度和角速度（度/s），点击"发送"
ros2 run turtle_pkg turtle_gui_ctrl_node
```

#### 3.3 PyQt5 GUI 控制 + 实时位姿显示

```bash
# 终端1：启动小乌龟仿真
ros2 run turtlesim turtlesim_node

# 终端2：启动带位姿反馈的 GUI，界面实时显示 X/Y 坐标、速度和角度
ros2 run turtle_pkg turtle_advice_ctrl_node
```

---

### 4. vehicle_pkg — 仿真小车速度控制 (C++)

向 `/cmd_vel` 话题发布速度指令，使仿真小车匀速前进（0.1 m/s），Ctrl+C 自动停车。

```bash
# 终端1：启动 WPB 仿真小车（需要 wpr_simulation2）
ros2 launch wpr_simulation2 wpb_simple.launch.py

# 终端2：运行速度控制节点，小车开始前进
ros2 run vehicle_pkg vehivle_node
# 按 Ctrl+C 停止，小车会自动刹车
```

---

### 5. vel_pkg — 仿真小车进阶控制 (C++ / Qt5 GUI)

提供三个节点，功能逐级递进，均配合 `wpr_simulation2` 仿真环境使用。

#### 5.1 命令行匀速前进

```bash
# 终端1：启动仿真环境
ros2 launch wpr_simulation2 wpb_simple.launch.py

# 终端2：小车以 0.1m/s 匀速前进，Ctrl+C 自动停车
ros2 run vel_pkg vel_node
```

#### 5.2 Qt5 GUI 速度控制

```bash
# 终端1：启动仿真环境
ros2 launch wpr_simulation2 wpb_simple.launch.py

# 终端2：弹出 Qt 窗口，输入线速度和角速度（度/s），点击"启动"/"停止"
ros2 run vel_pkg vel_gui_ctrl_node
```

#### 5.3 Qt5 GUI 速度控制 + 里程计实时显示

```bash
# 终端1：启动仿真环境
ros2 launch wpr_simulation2 wpb_simple.launch.py

# 终端2：弹出 Qt 窗口，控制小车运动，同时实时显示 X/Y 坐标、速度和角度
ros2 run vel_pkg vel_advice_ctrl_node
```

---

### 6. dummy_ros2 — 6DOF 机械臂 URDF 可视化

SolidWorks 导出的 6 自由度机械臂 URDF 模型，支持在 rviz2 中可视化和关节交互控制。

#### 6.1 在 rviz2 中查看并控制关节

```bash
ros2 launch dummy_ros2 display.launch.py
```

启动后会打开：
- **rviz2**：显示机械臂 3D 模型和 TF 坐标系
- **Joint State Publisher GUI**：拖动滑条实时控制各关节角度

#### 6.2 在 Gazebo 中仿真

```bash
ros2 launch dummy_ros2 gazebo.launch.py
```

---

### 7. wpr_simulation2 — WPB 机器人 Gazebo 仿真环境

第三方仿真包，为 `vehicle_pkg` 和 `vel_pkg` 提供 Gazebo 仿真场景。

```bash
# 启动基础仿真场景（空世界 + WPB 小车）
ros2 launch wpr_simulation2 wpb_simple.launch.py
```

其他可用的仿真场景：

```bash
# 带桌子的场景
ros2 launch wpr_simulation2 wpb_table.launch.py

# 带物体的场景
ros2 launch wpr_simulation2 wpb_objects.launch.py

# SLAM 建图场景
ros2 launch wpr_simulation2 slam.launch.py

# 导航场景
ros2 launch wpr_simulation2 navigation.launch.py
```

---

## 常见问题

### Q: 编译时报错 `find_package(Qt5 ...) FAILED`
```bash
sudo apt install qtbase5-dev
```

### Q: 运行 turtle_pkg 的 GUI 节点报错 `No module named 'PyQt5'`
```bash
sudo apt install python3-pyqt5
```

### Q: 新终端运行 `ros2 run` 提示找不到包
每次打开新终端都需要加载环境：
```bash
source /opt/ros/humble/setup.bash
source ~/ros2_ws/install/setup.bash
```

或将以上两行添加到 `~/.bashrc` 中实现自动加载：
```bash
echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
echo "source ~/ros2_ws/install/setup.bash" >> ~/.bashrc
```

### Q: Gazebo 启动后黑屏或很慢
首次启动 Gazebo 需要下载模型文件，可能需要等待较长时间。可尝试：
```bash
# 单独启动 Gazebo 测试
gazebo --verbose
```
