#include "WifiReceiverPlugin.hh"

#include <ros/ros.h>
#include "gazebo_wifi_plugin/ReceiverOracleReport.h"
#include "gazebo_wifi_plugin/ReceiverOracleRouterInfo.h"
#include "gazebo_wifi_plugin/ReceiverReport.h"
#include "gazebo_wifi_plugin/ReceiverRouterInfo.h"

#include "gazebo/sensors/SensorFactory.hh"
#include "gazebo/sensors/SensorManager.hh"
#include "gazebo/msgs/msgs.hh"
#include "gazebo/transport/Node.hh"
#include "gazebo/transport/Publisher.hh"
#include "gazebo/sensors/WirelessTransmitter.hh"

using namespace gazebo;
using namespace sensors;
GZ_REGISTER_SENSOR_PLUGIN(WifiReceiverPlugin)

WifiReceiverPlugin::WifiReceiverPlugin() : SensorPlugin() {
}

WifiReceiverPlugin::~WifiReceiverPlugin() {
}

void WifiReceiverPlugin::Load(
    sensors::SensorPtr sensor, sdf::ElementPtr sdf) {
  // Get the parent sensor.
  this->parent_sensor_ =
      std::dynamic_pointer_cast<sensors::WirelessReceiver>(sensor);

  // Make sure the parent sensor is valid.
  if (!this->parent_sensor_) {
    gzerr << "WifiReceiverPlugin requires a Wireless Transmitter Sensor.\n";
    return;
  }

  // Connect to the sensor update event.
  this->update_connection_ = this->parent_sensor_->ConnectUpdated(
      std::bind(&WifiReceiverPlugin::UpdateImpl, this));

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
  this->receiver_pub_ =
      node_handle.advertise<gazebo_wifi_plugin::ReceiverReport>(
      this->parent_sensor_->Name() + "/receiver_report", 1000);

  this->receiver_oracle_pub_ =
      node_handle.advertise<gazebo_wifi_plugin::ReceiverOracleReport>(
      this->parent_sensor_->Name() + "/receiver_oracle_report", 1000);
}

static ignition::math::Pose3d GetWorldPose(const sensors::SensorPtr sensor) {
  gazebo::physics::WorldPtr world = physics::get_world(
      sensor->WorldName());
  boost::weak_ptr<physics::Link> parent =
      boost::dynamic_pointer_cast<physics::Link>(
      world->EntityByName(sensor->ParentName()));
  return sensor->Pose() + parent.lock()->WorldPose();
}

std::string GetLinkName(const sensors::SensorPtr sensor) {
  gazebo::physics::WorldPtr world = physics::get_world(
      sensor->WorldName());
  boost::weak_ptr<physics::Link> parent =
      boost::dynamic_pointer_cast<physics::Link>(
      world->EntityByName(sensor->ParentName()));
  return parent.lock()->GetName();
}

bool WifiReceiverPlugin::UpdateImpl() {
  gazebo_wifi_plugin::ReceiverReport receiver_report_msg;
  gazebo_wifi_plugin::ReceiverOracleReport oracle_report_msg;

  this->seq_ ++;

  receiver_report_msg.header.stamp = ros::Time::now();
  receiver_report_msg.header.frame_id = GetLinkName(this->parent_sensor_);
  receiver_report_msg.header.seq = this->seq_;
  receiver_report_msg.receiver_gain = this->parent_sensor_->Gain();

  oracle_report_msg.header = receiver_report_msg.header;
  oracle_report_msg.receiver_gain = this->parent_sensor_->Gain();

  Sensor_V sensors = SensorManager::Instance()->GetSensors();
  const ignition::math::Pose3d this_pose = GetWorldPose(this->parent_sensor_);

  for (Sensor_V::iterator it = sensors.begin(); it != sensors.end(); ++it) {
    if ((*it)->Type() == "wireless_transmitter") {
      sensors::WirelessTransmitterPtr transmit_sensor =
          std::dynamic_pointer_cast<sensors::WirelessTransmitter>(*it);

      if (!transmit_sensor->IsActive()) {
        continue;
      }

      const double signal_strength = transmit_sensor->SignalStrength(
          this_pose, this->parent_sensor_->Gain());
      const double tx_freq = transmit_sensor->Freq();
      const std::string tx_essid = transmit_sensor->ESSID();

      // Save oracle router info.
      gazebo_wifi_plugin::ReceiverOracleRouterInfo oracle_info;
      oracle_info.essid = tx_essid;
      oracle_info.frequency = tx_freq;
      oracle_info.gain = transmit_sensor->Gain();
      oracle_info.power = transmit_sensor->Power();

      const ignition::math::Pose3d rel_pose =
          GetWorldPose(transmit_sensor) - this_pose;
      oracle_info.pose.position.x = rel_pose.Pos().X();
      oracle_info.pose.position.y = rel_pose.Pos().Y();
      oracle_info.pose.position.z = rel_pose.Pos().Z();

      oracle_info.pose.orientation.x = rel_pose.Rot().X();
      oracle_info.pose.orientation.y = rel_pose.Rot().Y();
      oracle_info.pose.orientation.z = rel_pose.Rot().Z();
      oracle_info.pose.orientation.w = rel_pose.Rot().W();

      oracle_report_msg.router_info.push_back(oracle_info);

      // Discard if the frequency received is out of our frequency range,
      // or if the received signal strengh is lower than the sensivity
      if ((tx_freq < this->parent_sensor_->MinFreqFiltered())
          || (tx_freq > this->parent_sensor_->MaxFreqFiltered())
          || (signal_strength < this->parent_sensor_->Sensitivity())) {
        continue;
      }

      // Save router info.
      gazebo_wifi_plugin::ReceiverRouterInfo router_info;
      router_info.signal_strength = signal_strength;
      router_info.frequency = tx_freq;
      router_info.essid = tx_essid;
      receiver_report_msg.router_info.push_back(router_info);
    }
  }

  receiver_pub_.publish(receiver_report_msg);
  receiver_oracle_pub_.publish(oracle_report_msg);
  return true;
}
