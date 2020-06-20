#ifndef _GAZEBO_WIFI_ROUTER_PLUGIN_HH_
#define _GAZEBO_WIFI_ROUTER_PLUGIN_HH_

#include <string>
#include <memory>

#include <ros/ros.h>
#include "ros/callback_queue.h"
#include <gazebo/gazebo.hh>
#include <gazebo/sensors/sensors.hh>

namespace gazebo {

class WifiRouterPlugin : public SensorPlugin
{
  /// \brief Constructor.
  public: WifiRouterPlugin();

  /// \brief Destructor.
  public: virtual ~WifiRouterPlugin();

  /// \brief Load the sensor plugin.
  /// \param[in] sensor Pointer to the sensor that loaded this plugin.
  /// \param[in] sdf SDF element that describes the plugin.
  public: virtual void Load(sensors::SensorPtr sensor, sdf::ElementPtr sdf);

  private: virtual void OnUpdate();

  private: sensors::WirelessTransmitterPtr parent_sensor_;

  private: event::ConnectionPtr update_connection_;

  private: ros::Subscriber activate_sub_;
  private: ros::Subscriber deactivate_sub_;
  private: ros::CallbackQueue ros_queue_;
  private: std::unique_ptr<ros::AsyncSpinner> ros_async_spinner_;
};

}  // namespace gazebo
#endif
