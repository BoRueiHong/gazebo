/*
 * Copyright (C) 2015 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef _GAZEBO_GUI_JOINT_INSPECTOR_PRIVATE_HH_
#define _GAZEBO_GUI_JOINT_INSPECTOR_PRIVATE_HH_

#include <string>
#include <vector>

#include "gazebo/common/CommonTypes.hh"

#include "gazebo/msgs/msgs.hh"

#include "gazebo/gui/qt.h"

namespace gazebo
{
  namespace gui
  {
    class ConfigChildWidget;
    class ConfigWidget;
    class JointMaker;

    /// \brief Private data for the JointInspector class
    class JointInspectorPrivate
    {
      /// \brief Config widget for configuring joint properties.
      public: ConfigWidget *configWidget;

      /// \brief Widget for the joint name.
      public: ConfigChildWidget *nameWidget;

      /// \brief Custom widget for the parent link to be used instead of the one
      /// generated by parsing the joint message.
      public: ConfigChildWidget *parentLinkWidget;

      /// \brief Custom widget for the child link to be used instead of the one
      /// generated by parsing the joint message.
      public: ConfigChildWidget *childLinkWidget;

      /// \brief Ok button.
      public: QPushButton *okButton;

      /// \brief A list of gui editor events connected to this.
      public: std::vector<event::ConnectionPtr> connections;

      /// \brief Pointer to the joint maker.
      public: JointMaker *jointMaker;

      /// \brief Unique ID which identifies this joint in the joint maker.
      public: std::string jointId;

      /// \brief Label holding icon which represents the parent link. The icon
      /// color changes according to the joint type.
      public: QLabel *parentIcon;

      /// \brief Flag that indicates whether current joint name is valid.
      public: bool validJointName;

      /// \brief Flag that indicates whether current links are valid.
      public: bool validLinks;

      /// \brief Message containing the data which was in the widget when first
      /// open.
      public: msgs::Joint originalDataMsg;
    };
  }
}
#endif
