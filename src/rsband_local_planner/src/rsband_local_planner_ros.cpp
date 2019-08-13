/*********************************************************************
* build depend on rsband by George Kouros
* Author:  bkxcyu
*********************************************************************/

#include "rsband_local_planner/rsband_local_planner_ros.h"
#include <base_local_planner/goal_functions.h>
#include <pluginlib/class_list_macros.h>
#include <angles/angles.h>
#include <tf_conversions/tf_eigen.h>
#include"rsband_local_planner/pose_se2.h"
PLUGINLIB_DECLARE_CLASS(rsband_local_planner, RSBandPlannerROS,
  rsband_local_planner::RSBandPlannerROS, nav_core::BaseLocalPlanner);

namespace rsband_local_planner
{

  RSBandPlannerROS::RSBandPlannerROS() : initialized_(false)
  {
  }


  RSBandPlannerROS::~RSBandPlannerROS()
  {
  }


  void RSBandPlannerROS::initialize(std::string name,
    tf::TransformListener* tfListener, costmap_2d::Costmap2DROS* costmapROS)
  {
    if (initialized_)
    {
      ROS_WARN("Planner already initialized. Should not be called more than "
        "once");
      return;
    }

    // store tflistener and costmapROS
    tfListener_ = tfListener;
    costmapROS_ = costmapROS;
    costmap_ = costmapROS_->getCostmap();

    ros::NodeHandle pnh("~/" + name);

    obst_pub = _n_.advertise<visualization_msgs::Marker>("obst", 10);

    L1_ = boost::shared_ptr<L1Controller>(new L1Controller(name));

    // create and initialize dynamic reconfigure
    drs_.reset(new drs(pnh));
    drs::CallbackType cb =
      boost::bind(&RSBandPlannerROS::reconfigureCallback, this, _1, _2);
    drs_->setCallback(cb);

    // set initilized
    initialized_ = true;

    //创建链表
    whosyourdaddy.warning_point = whosyourdaddy.creatlist();

    ROS_DEBUG("Local Planner Plugin Initialized!");
  }
  void RSBandPlannerROS::show_obst(float x,float y)
  {
    geometry_msgs::Point POINT;
    visualization_msgs::Marker points;
    POINT.x = x;
    POINT.y = y;
    points.points.push_back(POINT);
    obst_pub.publish(points); 
  }
  void RSBandPlannerROS::initMarker()
  {
    //initpoint
    points.header.frame_id = "odom";
    points.ns = "Markers";
    points.action =  visualization_msgs::Marker::ADD;
    points.pose.orientation.w =1.0;
    points.id = 0;
    points.type = visualization_msgs::Marker::POINTS;
    // POINTS markers use x and y scale for width/height respectively
    points.scale.x = 0.2;
    points.scale.y = 0.2;
    // Points are green
    points.color.g = 1.0f;
    points.color.a = 1.0; 
  }
  void RSBandPlannerROS::reconfigureCallback(RSBandPlannerConfig& config,
    uint32_t level)
  {
    // xyGoalTolerance_ = config.xy_goal_tolerance;
    // yawGoalTolerance_ = config.yaw_goal_tolerance;
    whosyourdaddy.gain_angle=config.gain_angle;
    whosyourdaddy.unit_distance=config.unit_distance;
    whosyourdaddy.warning_distance=config.warning_distance;
    whosyourdaddy.limit_distance=config.limit_distance;
    whosyourdaddy.angle_max=config.angle_max;
    whosyourdaddy.angle_min=whosyourdaddy.angle_min;

    if (L1_)
      L1_->reconfigure(config);
    else
      ROS_ERROR("Reconfigure CB called before path tracking controller "
        "initialization!");
  }


  bool RSBandPlannerROS::setPlan(
    const std::vector<geometry_msgs::PoseStamped>& globalPlan)
  {
    if (!initialized_)
    {
      ROS_ERROR("Planner must be initialized before setPlan is called!");
      return false;
    }

    globalPlan_ = globalPlan;
    return true;
  }


  bool RSBandPlannerROS::computeVelocityCommands(geometry_msgs::Twist& cmd)
  {
    if (!initialized_)
    {
      ROS_ERROR("Planner must be initialized before computeVelocityCommands "
        "is called!");
      return false;
    }

    if (!L1_->computeVelocityCommands(cmd))
    {
      ROS_ERROR("Path tracking controller failed to produce command");
      return false;
    }
    
    double _rectified_angular;
    _rectified_angular=rectifyAngularVel();
    cmd.angular.z+=_rectified_angular;

    return true;
  }


  bool RSBandPlannerROS::isGoalReached()
  {
    if (!initialized_)
    {
      ROS_ERROR("Planner must be initialized before isGoalReached is called!");
      return false;
    }

    tf::Stamped<tf::Pose> robotPose;
    if (!costmapROS_->getRobotPose(robotPose))
    {
      ROS_ERROR("Could not get robot pose!");
      return false;
    }

    geometry_msgs::PoseStamped goal = globalPlan_.back();

    double dist = base_local_planner::getGoalPositionDistance(
      robotPose, goal.pose.position.x, goal.pose.position.y);
    double yawDiff = base_local_planner::getGoalOrientationAngleDifference(
      robotPose, tf::getYaw(goal.pose.orientation));

    if (dist < xyGoalTolerance_ && fabs(yawDiff) < yawGoalTolerance_)
    {
      ROS_INFO("Goal Reached!");
      return true;
    }

    return false;
  }

  float map(float value, float istart, float istop, float ostart, float ostop)
  {
	  return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
  }
  double RSBandPlannerROS::rectifyAngularVel()
  {
      double rectified_angular;
      //get robot pose 
      PoseSE2 robot_pose_;
      tf::Stamped<tf::Pose> robot_pose;
      if (!costmapROS_->getRobotPose(robot_pose))
      {
        ROS_ERROR("Could not get robot pose!");
        return false;
      }
      robot_pose_ = PoseSE2(robot_pose);
      //get robot orientation vector
      Eigen::Vector2d robot_orient = robot_pose_.orientationUnitVec();

      whosyourdaddy.clearlist(whosyourdaddy.warning_point);
      //scan local costmap to find obstacle point
      for (unsigned int i=0; i<costmap_->getSizeInCellsX()-1; ++i)
      {
        for (unsigned int j=0; j<costmap_->getSizeInCellsY()-1; ++j)
        { 
          //find one of the obstacles
          if (costmap_->getCost(i,j) == costmap_2d::LETHAL_OBSTACLE)
          {
            //transform obstacle  of costmap frame into local_map(/odom) frame and get it's direction
            Eigen::Vector2d obs;
            costmap_->mapToWorld(i,j,obs.coeffRef(0), obs.coeffRef(1));
            Eigen::Vector2d obs_dir = obs-robot_pose_.position();
           //obs_dir是odom坐标系下 扫描到的障碍物与机器人位置的差向量
           //robot_orient是机器人的方向向量
          /*---------------------------------------------------*/
            //get distance between obstacle and robot
            //get angular between robot orientation and obstacle direction
            float dis,ang;
            dis=obs_dir.norm();
            ang=acos(obs_dir.dot(robot_orient)/(obs_dir.norm()*robot_orient.norm()));
            if(dis<0.2)
            {
              whosyourdaddy.append(whosyourdaddy.warning_point,dis,ang);
              ///////
              show_obst(obs.coeffRef(0),obs.coeffRef(1));//odom
            }
          /*---------------------------------------------------*/
          }
        }
      }

      //finally output a vector called whosyourdaddy.out_point ? how to transform it into adjust_angular?
      whosyourdaddy.sortlist(whosyourdaddy.warning_point);
      whosyourdaddy.last_point = whosyourdaddy.getlastnode(whosyourdaddy.warning_point);
      if(whosyourdaddy.last_point->distance == LINK_HEAD_D || whosyourdaddy.last_point->distance == LINK_HEAD_A) 
      	printf("null\n");
      else
      {
      	whosyourdaddy.v_vector(whosyourdaddy.last_point);
      }
      float out_ang=0.0;
      rectified_angular=map(out_ang,-1.58,1.58,900,2100);
      return rectified_angular;
      
  }

}  // namespace rsband_local_planner
