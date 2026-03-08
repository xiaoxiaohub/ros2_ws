cmake_minimum_required(VERSION 3.8)
project(vel_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# 添加可执行文件 vel_node
add_executable(vel_node src/vel_node.cpp)
# 链接依赖
ament_target_dependencies(vel_node rclcpp geometry_msgs)

# 添加可执行文件 vel_gui_ctrl_node（Qt5 图形界面控制节点）
add_executable(vel_gui_ctrl_node src/vel_gui_ctrl_node.cpp)
ament_target_dependencies(vel_gui_ctrl_node rclcpp geometry_msgs)
target_link_libraries(vel_gui_ctrl_node Qt5::Widgets)

# 添加可执行文件 vel_advice_ctrl_node（Qt5 图形界面控制节点 + 里程计订阅）
add_executable(vel_advice_ctrl_node src/vel_advice_ctrl_node.cpp)
ament_target_dependencies(vel_advice_ctrl_node rclcpp geometry_msgs nav_msgs)
target_link_libraries(vel_advice_ctrl_node Qt5::Widgets)

# 安装可执行文件到 lib 目录，供 ros2 run 调用
install(TARGETS
  vel_node
  vel_gui_ctrl_node
  vel_advice_ctrl_node
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
