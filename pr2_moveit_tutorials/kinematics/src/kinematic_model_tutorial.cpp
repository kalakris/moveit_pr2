/*********************************************************************
*
* Software License Agreement (BSD License)
*
*  Copyright (c) 2012, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of Willow Garage, Inc. nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
* Author: Sachin Chitta
*********************************************************************/

#include <ros/ros.h>

// MoveIt!
#include <moveit/planning_models_loader/kinematic_model_loader.h>
#include <moveit/kinematic_model/kinematic_model.h>
#include <moveit/kinematic_state/kinematic_state.h>
#include <moveit/kinematic_state/joint_state_group.h>
  
int main(int argc, char **argv)
{
  ros::init (argc, argv, "right_arm_kinematics");
  ros::AsyncSpinner spinner(1);
  spinner.start();
  
  /* Load the robot model */
  planning_models_loader::KinematicModelLoader kinematic_model_loader("robot_description"); 

  /* Get a shared pointer to the model */
  kinematic_model::KinematicModelPtr kinematic_model = kinematic_model_loader.getModel();

  /* Get and print the name of the coordinate frame in which the transforms for this model are computed*/
  ROS_INFO("Model frame: %s", kinematic_model->getModelFrame().c_str());  
  
  /* Create a kinematic state - this represents the configuration for the robot represented by kinematic_model */
  kinematic_state::KinematicStatePtr kinematic_state(new kinematic_state::KinematicState(kinematic_model));

  /* Set all joints in this state to their default values */
  kinematic_state->setToDefaultValues();  

  /* Get the configuration for the joints in the right arm of the PR2*/
  kinematic_state::JointStateGroup* joint_state_group = kinematic_state->getJointStateGroup("right_arm");
  
  /* Compute FK for a set of random joint values*/
  joint_state_group->setToRandomValues();
  const Eigen::Affine3d &end_effector_state = joint_state_group->getKinematicState()->getLinkState("r_wrist_roll_link")->getGlobalLinkTransform();    
  
  /* Get the joint states that we just computed FK for*/
  std::vector<double> joint_values;
  joint_state_group->getVariableValues(joint_values);
  
  /* Get the names of the joints in the right_arm*/
  const std::vector<std::string> &joint_names = joint_state_group->getJointModelGroup()->getJointModelNames();
  
  /* Print joint names and values */
  for(std::size_t i = 0; i < joint_names.size(); ++i)
  {
    ROS_INFO("Joint %s: %f", joint_names[i].c_str(), joint_values[i]);
  }
  
  /* Print end-effector pose. Remember that this is in the model frame */
  ROS_INFO_STREAM("Translation: " << end_effector_state.translation()); 
  ROS_INFO_STREAM("Rotation: " << end_effector_state.rotation()); 
  

  /* Set joint state group to a set of random values*/
  joint_state_group->setToRandomValues();

  /* Do IK on the pose we just generated using forward kinematics
   * Here 5 is the number of random restart and 0.1 is the allowed time after
   * each restart
   */
  bool found_ik = joint_state_group->setFromIK(end_effector_state, 10, 1.0);
  
  /* Get and print the joint values */

  if (found_ik)
  {
    joint_state_group->getVariableValues(joint_values);
    for(std::size_t i=0; i < joint_names.size(); ++i)
    {
      ROS_INFO("Joint %s: %f", joint_names[i].c_str(), joint_values[i]);
    }
  }
  else
  {
    ROS_INFO("Did not find IK solution");
  }

  ros::shutdown();  
  return 0;
}
