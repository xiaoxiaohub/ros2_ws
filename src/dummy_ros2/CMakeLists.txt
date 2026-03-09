cmake_minimum_required(VERSION 3.8)

project(dummy_ros2)

find_package(ament_cmake REQUIRED)

install(DIRECTORY
  launch
  urdf
  meshes
  config
  rviz
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()