#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"

#include <windows.h>
#include "joystickapi.h"
#include "mmsystem.h"

#pragma comment(lib, "winmm.lib")

using namespace std::chrono_literals;

double scale_val(DWORD val)
{
    double ret = (val - 32767.) / 32767.;

    if (-0.1 < ret && ret < 0.1)
        return 0.;
    else if (ret < -1)
        return 0.5;
    else if (ret > 1)
        return -0.5;

    return ret * -0.5;
}

/* This example creates a subclass of Node and uses std::bind() to register a
* member function as a callback from the timer. */

class MinimalPublisher : public rclcpp::Node
{
  public:
    MinimalPublisher()
    : Node("joyjoy")
    {
      publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);
      timer_ = this->create_wall_timer(100ms, std::bind(&MinimalPublisher::timer_callback, this));
      ZeroMemory(&joystickInfo, sizeof(joystickInfo));
      joystickInfo.dwSize = sizeof(joystickInfo);
      joystickInfo.dwFlags = JOY_RETURNALL;
      RCLCPP_INFO_STREAM(get_logger(), "JOYSTICK ready");
    }

  private:
    void timer_callback()
    {
        auto message = geometry_msgs::msg::Twist();
        joyGetPosEx(JOYSTICKID1, &joystickInfo);

        if (joystickInfo.dwButtons == 1)
        {
            double valY = scale_val(joystickInfo.dwYpos);
            message.linear.x = valY;
            double valX = scale_val(joystickInfo.dwXpos);
            if (valY < 0)
                message.angular.z = -valX;
            else
                message.angular.z = valX;

            RCLCPP_INFO_STREAM(get_logger(), "linear.x: " << message.linear.x << " angular.z: " << message.angular.z);
            publisher_->publish(message);
        }
    }
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    JOYINFOEX joystickInfo;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalPublisher>());
  rclcpp::shutdown();
  return 0;
}