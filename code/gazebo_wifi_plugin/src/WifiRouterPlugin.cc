#include "WifiRouterPlugin.hh"

#include <iostream>
#include <memory>
#include <ros/ros.h>
#include "std_msgs/String.h"

using namespace gazebo;
GZ_REGISTER_SENSOR_PLUGIN(WifiRouterPlugin)

WifiRouterPlugin::WifiRouterPlugin() : SensorPlugin()
{
}

WifiRouterPlugin::~WifiRouterPlugin()
{
}

void WifiRouterPlugin::Load(
    sensors::SensorPtr sensor, sdf::ElementPtr sdf) {
  // Get the parent sensor.
  this->parent_sensor_ =
      std::dynamic_pointer_cast<sensors::WirelessTransmitter>(sensor);

  // Make sure the parent sensor is valid.
  if (!this->parent_sensor_) {
    gzerr << "WifiRouterPlugin requires a Wireless Transmitter Sensor.\n";
    return;
  }

  // Connect to the sensor update event.
  this->update_connection_ = this->parent_sensor_->ConnectUpdated(
      std::bind(&WifiRouterPlugin::OnUpdate, this));

  // Make sure the parent sensor is active.
  this->parent_sensor_->SetActive(true);

  if (!ros::isInitialized()) {
    // There is a race condition. But who cares if ROS tutorial recommends this
    // code: http://gazebosim.org/tutorials?tut=guided_i6
    int argc = 0;
    char **argv = NULL;
    ros::init(argc, argv, "gazebo_client",
        ros::init_options::NoSigintHandler);
  }

  ros::NodeHandle node_handle;

  ros::SubscribeOptions so_activate =
      ros::SubscribeOptions::create<std_msgs::String>(
      "/cogrob/wifi/activate", 1,
      [this](const std_msgs::String::ConstPtr& msg) {
        if (this->parent_sensor_->ESSID() == msg->data) {
          this->parent_sensor_->SetActive(true);
        }
      }, ros::VoidPtr(), &this->ros_queue_);
  activate_sub_ = node_handle.subscribe(so_activate);

  ros::SubscribeOptions so_deactivate =
      ros::SubscribeOptions::create<std_msgs::String>(
      "/cogrob/wifi/deactivate", 1,
      [this](const std_msgs::String::ConstPtr& msg) {
        if (this->parent_sensor_->ESSID() == msg->data) {
          this->parent_sensor_->SetActive(false);
        }
      }, ros::VoidPtr(), &this->ros_queue_);
  deactivate_sub_ = node_handle.subscribe(so_deactivate);

  ros_async_spinner_ = std::unique_ptr<ros::AsyncSpinner>(
      new ros::AsyncSpinner(1, &this->ros_queue_));
  ros_async_spinner_->start();
}

void WifiRouterPlugin::OnUpdate() {
  std::string essid;
  essid = this->parent_sensor_->ESSID();
}
