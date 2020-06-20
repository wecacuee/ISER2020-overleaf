cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(gazebo_wifi_plugin)

find_package(gazebo REQUIRED)
find_package(catkin REQUIRED COMPONENTS roscpp rospy std_msgs genmsg geometry_msgs)

add_message_files(
  DIRECTORY msg
  FILES ReceiverOracleReport.msg ReceiverOracleRouterInfo.msg
  ReceiverReport.msg ReceiverRouterInfo.msg)

generate_messages(DEPENDENCIES std_msgs geometry_msgs)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES gazebo_wifi_plugin
#  CATKIN_DEPENDS other_catkin_pkg
#  DEPENDS system_lib
)

include_directories(${GAZEBO_INCLUDE_DIRS} ${catkin_INCLUDE_DIRS} src)
link_directories(${GAZEBO_LIBRARY_DIRS} ${catkin_LIBRARY_DIRS})
list(APPEND CMAKE_CXX_FLAGS "${GAZEBO_CXX_FLAGS}")

add_library(wifirouter SHARED src/WifiRouterPlugin.cc)
target_link_libraries(wifirouter ${GAZEBO_libraries} ${catkin_LIBRARIES})

add_library(wifireceiver SHARED src/WifiReceiverPlugin.cc)
target_link_libraries(wifireceiver ${GAZEBO_libraries} ${catkin_LIBRARIES})
