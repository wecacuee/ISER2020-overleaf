#!/usr/bin/env python

import rospy
from gazebo_wifi_plugin.msg import ReceiverOracleReport
from nav_msgs.msg import OccupancyGrid
import sys
import math

pub = None

WIDTH = 150
HEIGHT = 150
RESOLUTION = 0.2

def report_callback(oracle_report):
  result = OccupancyGrid()
  result.header = oracle_report.header

  result.info.resolution = RESOLUTION
  result.info.width = WIDTH
  result.info.height = HEIGHT
  result.info.origin.position.x = - WIDTH * RESOLUTION / 2.0
  result.info.origin.position.y = - HEIGHT * RESOLUTION / 2.0
  result.info.origin.orientation.w = 1

  power_sum = []

  for i in range(WIDTH):
    y = i * RESOLUTION + result.info.origin.position.x
    for j in range(HEIGHT):
      x = j * RESOLUTION + result.info.origin.position.y

      power = 0.0
      for router in oracle_report.router_info:
        dx = router.pose.position.x - x
        dy = router.pose.position.y - y
        distance = max(1.0, math.sqrt(dx * dx + dy * dy))
        speed_of_light = 299792458.0
        wavelength = speed_of_light / (router.frequency * 1000000)
        rx_power = (
            router.power + router.gain + oracle_report.receiver_gain
            + 20 * math.log10(wavelength) - - 20 * math.log10(4 * math.pi)
            - 10 * 12.0 * math.log10(distance))
        power += math.exp(rx_power / 10)
      # TODO(shengye): Is this correct?
      final_power = 10 * math.log(power)
      power_sum.append(final_power)

  min_power_sum = min(power_sum)
  max_power_sum = max(power_sum)
  scaler = 100 / (max_power_sum - min_power_sum)

  for p in power_sum:
    num = (p - min_power_sum) * scaler
    num = min(max(num, 0), 100)
    result.data.append(num)

  pub.publish(result)


def main(argv):
  global pub
  rospy.init_node("rssi_heatmap", anonymous=True)
  pub = rospy.Publisher("rssi_heatmap", OccupancyGrid, queue_size=10)
  sub = rospy.Subscriber("/cogrob_wireless_receiver/receiver_oracle_report",
                         ReceiverOracleReport, report_callback)
  rospy.spin()

if __name__ == "__main__":
  main(sys.argv)
