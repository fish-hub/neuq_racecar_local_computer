#include <iostream>
#include "ros/ros.h"
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Twist.h>
#include <tf/transform_listener.h>
#include <tf/transform_datatypes.h>
#include "nav_msgs/Path.h"
#include <nav_msgs/Odometry.h>
#include <visualization_msgs/Marker.h>

class Teb_controller
{
public:
    Teb_controller();


private:
    /* data */
    ros::NodeHandle n_;
    ros::Subscriber odom_sub, path_sub, goal_sub;
    nav_msgs::Odometry odom;
    nav_msgs::Path map_path;


    void odomCB(const nav_msgs::Odometry::ConstPtr& odomMsg);
    void pathCB(const nav_msgs::Path::ConstPtr& pathMsg);
    void goalCB(const geometry_msgs::PoseStamped::ConstPtr& goalMsg);
};

Teb_controller::Teb_controller()
{
    ros::NodeHandle p("~");

    odom_sub=n_.subscribe("/odometry/filtered",1,&Teb_controller::odomCB,this);
    path_sub=n_.subscribe("/planed_path",1,&Teb_controller::pathCB,this);
    goal_sub=n_.subscribe("move_base_simple/goal",1,&Teb_controller::goalCB,this);

}

void Teb_controller::odomCB(const nav_msgs::Odometry::ConstPtr& odomMsg)
{
    odom = *odomMsg;
}

void Teb_controller::pathCB(const nav_msgs::Path::ConstPtr& pathMsg)
{
    map_path = *pathMsg;
}

void Teb_controller::goalCB(const geometry_msgs::PoseStamped::ConstPtr& goalMsg)
{

}

int main(int argc, char **argv)
{
    /* code for main function */
    ros::init(argc, argv, "Teb_Controller");
    Teb_controller Controller;
    ros::spin();
    return 0;
}