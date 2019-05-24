//
// Created by Steven Zhang on 18-12-14.
// art racecar
//

#include "../include/art_racecar_driver.h"
#include <ros/ros.h>
#include <ros/package.h>
#include <geometry_msgs/Twist.h>
#define PI 3.14159265358979
#include <ackermann_msgs/AckermannDriveStamped.h>

float fmap(float toMap, float in_min, float in_max, float out_min, float out_max)
{
  return(toMap - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void TwistCallback(const geometry_msgs::Twist& twist)
{
    double angle;
    double vel;
    double angle_in_degree;
    ROS_INFO("x= %f", twist.linear.x);
    ROS_INFO("z= %f", twist.angular.z);
    // angle_in_degree=twist.angular.z*180/PI;
    // angle = 2500.0 - angle_in_degree * 2000.0 / 180.0;
    // angle = 2500.0 - twist.angular.z * 2000.0 / 180.0;//777  2222

     if(twist.angular.z>=0)
        angle=fmap(twist.angular.z,0,0.36,1500,2222);
    if(twist.angular.z<0)
        angle=fmap(twist.angular.z,0,-0.36,1500,777);

    if(twist.linear.x>0)
        vel=fmap(twist.linear.x,0,1,1550,1600);
    if(twist.linear.x<0)
        vel=fmap(twist.linear.x,0,-1,1350,1300);
    if(twist.linear.x==0)
        vel=1500;
     ROS_INFO("angle= %d  vel=%d",uint16_t(angle),uint16_t(vel));
    send_cmd(uint16_t(vel),uint16_t(angle));
}

void AckermannCallback(const ackermann_msgs::AckermannDriveStamped& Ackermann)
{
    double angle;
    double vel;
    //ROS_INFO("x= %f", twist.linear.x);
    //ROS_INFO("z= %f", twist.angular.z);

    if(Ackermann.drive.steering_angle>=0)
        angle=fmap(Ackermann.drive.steering_angle,0,0.4,1500,2222);
    if(Ackermann.drive.steering_angle<0)
        angle=fmap(Ackermann.drive.steering_angle,0,-0.4,1500,777);
    if(Ackermann.drive.speed>0)
        vel=fmap(Ackermann.drive.speed,0,1,1550,1600);
    if(Ackermann.drive.speed<0)
        vel=fmap(Ackermann.drive.speed,0,-1,1350,1300);
    if(Ackermann.drive.speed==0)
        vel=1500;
    //ROS_INFO("angle= %d",uint16_t(angle));
    // send_cmd(uint16_t(vel),uint16_t(angle));
}

int main(int argc, char** argv)
{
    char data[] = "/dev/car";
    art_racecar_init(38400,data);
    ros::init(argc, argv, "art_driver");
    ros::NodeHandle n;


    // ros::Subscriber sub1 = n.subscribe("/ackermann_cmd",1,AckermannCallback);
    ros::Subscriber sub2 = n.subscribe("/cmd_vel",1,TwistCallback);
        

    ros::spin();

}