#ifndef _GAZEBO_WIFI_RECEIVER_PLUGIN_HH_
#define _GAZEBO_WIFI_RECEIVER_PLUGIN_HH_

#include <string>

#include <gazebo/gazebo.hh>
#include <gazebo/sensors/sensors.hh>
#include <ros/ros.h>

namespace gazebo {

/// \brief An example plugin for a contact sensor.
class WifiReceiverPlugin : public SensorPlugin
{
  /// \brief Constructor.
  public: WifiReceiverPlugin();

  /// \brief Destructor.
  public: virtual ~WifiReceiverPlugin();

  /// \brief Load the sensor plugin.
  /// \param[in] sensor Pointer to the sensor that loaded this plugin.
  /// \param[in] sdf SDF element that describes the plugin.
  public: virtual void Load(sensors::SensorPtr sensor, sdf::ElementPtr sdf);

  /// \brief Callback that receives the contact sensor's update signal.
  // private: virtual void OnUpdate();
  private: virtual bool UpdateImpl();

  /// \brief Pointer to the contact sensor
  private: sensors::WirelessReceiverPtr parent_sensor_;

  /// \brief Connection that maintains a link between the contact sensor's
  /// updated signal and the OnUpdate callback.
  private: event::ConnectionPtr update_connection_;

  private: ros::Publisher receiver_pub_;
  private: ros::Publisher receiver_oracle_pub_;

  private: int seq_;
};

}  // namespace gazebo

#endif
