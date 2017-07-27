/*=========================================================================

  Program:   Converter Class for Point
  Language:  C++

  Copyright (c) Brigham and Women's Hospital. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "rib_converter_point.h"
#include "ros/ros.h"
#include "igtlPointMessage.h"

RIBConverterPoint::RIBConverterPoint()
  : RIBConverter<ros_igtl_bridge::igtlpoint>()
{
}

RIBConverterPoint::RIBConverterPoint(ros::NodeHandle *nh)
  : RIBConverter<ros_igtl_bridge::igtlpoint>(nh)
{
}

RIBConverterPoint::RIBConverterPoint(const char* topicPublish, const char* topicSubscribe, ros::NodeHandle *nh)
  : RIBConverter<ros_igtl_bridge::igtlpoint>(topicPublish, topicSubscribe, nh)
{
}

int RIBConverterPoint::onIGTLMessage(igtl::MessageHeader * header)
{
  igtl::PointMessage::Pointer pointMsg = igtl::PointMessage::New();
  pointMsg->SetMessageHeader(header);
  pointMsg->AllocatePack();
  
  this->socket->Receive(pointMsg->GetPackBodyPointer(), pointMsg->GetPackBodySize());
  int c = pointMsg->Unpack(1);
  
  if ((c & igtl::MessageHeader::UNPACK_BODY) == 0) 
    {
    ROS_ERROR("[ROS-IGTL-Bridge] Failed to unpack the message. Datatype: POINT.");
    return 0;
    }
  
  int npoints = pointMsg->GetNumberOfPointElement ();
  
  if (npoints > 0)
    {
    for (int i = 0; i < npoints; i ++)
      {
      igtlFloat32 point[3];
      igtl::PointElement::Pointer elem = igtl::PointElement::New();
      pointMsg->GetPointElement (i,elem);
      elem->GetPosition(point);
      
      ros_igtl_bridge::igtlpoint msg;
      
      msg.pointdata.x = point[0];
      msg.pointdata.y = point[1];
      msg.pointdata.z = point[2];
      msg.name = elem->GetName();
      
      this->publisher.publish(msg);
      }
    }
  else
    {
    ROS_ERROR("[ROS-IGTL-Bridge] Message POINT is empty");
    return 0;
    }
  
  return 1;
  
}

void RIBConverterPoint::onROSMessage(const ros_igtl_bridge::igtlpoint::ConstPtr & msg)
{
  geometry_msgs::Point point = msg->pointdata;
  
  igtl::PointMessage::Pointer pointMsg = igtl::PointMessage::New();
  pointMsg->SetDeviceName(msg->name.c_str());
  
  igtl::PointElement::Pointer pointE; 
  pointE = igtl::PointElement::New();
  pointE->SetPosition(point.x, point.y,point.z);
  pointMsg->AddPointElement(pointE);
  pointMsg->Pack();
  
  this->socket->Send(pointMsg->GetPackPointer(), pointMsg->GetPackSize());
}

