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

#include "gazebo/common/Console.hh"

#include "gazebo/gui/ConfigWidget.hh"
#include "gazebo/gui/model/ModelEditorEvents.hh"
#include "gazebo/gui/model/JointMaker.hh"
#include "gazebo/gui/model/JointCreationDialog.hh"

using namespace gazebo;
using namespace gui;

/////////////////////////////////////////////////
JointCreationDialog::JointCreationDialog(JointMaker *_jointMaker,
    QWidget *_parent) : QDialog(_parent)
{
  this->setObjectName("JointCreationDialogDialog");
  this->setWindowTitle(tr("Create Joint"));
  this->setWindowFlags(Qt::WindowStaysOnTopHint);

  this->jointMaker = _jointMaker;

  this->configWidget = new ConfigWidget();

  // Joint types
  QRadioButton *fixedJointRadio = new QRadioButton(tr("Fixed"));
  QRadioButton *prismaticJointRadio = new QRadioButton(tr("Prismatic"));
  QRadioButton *revoluteJointRadio = new QRadioButton(tr("Revolute"));
  QRadioButton *revolute2JointRadio = new QRadioButton(tr("Revolute2"));
  QRadioButton *screwJointRadio = new QRadioButton(tr("Screw"));
  QRadioButton *universalJointRadio = new QRadioButton(tr("Universal"));
  QRadioButton *ballJointRadio = new QRadioButton(tr("Ball"));

  this->typeButtons = new QButtonGroup();
  this->typeButtons->addButton(fixedJointRadio, 1);
  this->typeButtons->addButton(prismaticJointRadio, 2);
  this->typeButtons->addButton(revoluteJointRadio, 3);
  this->typeButtons->addButton(revolute2JointRadio, 4);
  this->typeButtons->addButton(screwJointRadio, 5);
  this->typeButtons->addButton(universalJointRadio, 6);
  this->typeButtons->addButton(ballJointRadio, 7);
  connect(this->typeButtons, SIGNAL(buttonClicked(int)),
      this, SLOT(OnTypeFromDialog(int)));

  QVBoxLayout *typesLayout = new QVBoxLayout();
  typesLayout->addWidget(fixedJointRadio);
  typesLayout->addWidget(revoluteJointRadio);
  typesLayout->addWidget(revolute2JointRadio);
  typesLayout->addWidget(prismaticJointRadio);
  typesLayout->addWidget(screwJointRadio);
  typesLayout->addWidget(universalJointRadio);
  typesLayout->addWidget(ballJointRadio);

  ConfigChildWidget *typesWidget = new ConfigChildWidget();
  typesWidget->setLayout(typesLayout);

  QWidget *typesGroupWidget =
      this->configWidget->CreateGroupWidget("Joint types", typesWidget, 0);

  // Link selections
  QLabel *selectionsText = new QLabel(tr(
      "Click a link in the scene "
       "to select parent. Click "
       "again to select child."));

  QLabel *parentLabel = new QLabel("Parent: ");
  this->parentComboBox = new QComboBox();
  this->parentComboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
  this->parentComboBox->addItem("", "");
  connect(this->parentComboBox, SIGNAL(currentIndexChanged(int)), this,
      SLOT(OnParentFromDialog(int)));

  QLabel *childLabel = new QLabel("Child: ");
  this->childComboBox = new QComboBox();
  this->childComboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
  this->childComboBox->addItem("", "");
  this->childComboBox->setEnabled(false);
  connect(this->childComboBox, SIGNAL(currentIndexChanged(int)), this,
      SLOT(OnChildFromDialog(int)));

  QGridLayout *linksLayout = new QGridLayout();
  linksLayout->addWidget(selectionsText, 0, 0, 1, 2);
  linksLayout->addWidget(parentLabel, 1, 0);
  linksLayout->addWidget(this->parentComboBox, 1, 1);
  linksLayout->addWidget(childLabel, 2, 0);
  linksLayout->addWidget(this->childComboBox, 2, 1);

  ConfigChildWidget *linksWidget = new ConfigChildWidget();
  linksWidget->setLayout(linksLayout);

  QWidget *linksGroupWidget = this->configWidget->CreateGroupWidget(
      "Link Selections", linksWidget, 0);




  QVBoxLayout *configLayout = new QVBoxLayout();
  configLayout->addWidget(typesGroupWidget);
  configLayout->addWidget(linksGroupWidget);

  this->configWidget->setLayout(configLayout);






  QScrollArea *scrollArea = new QScrollArea;
  scrollArea->setWidget(configWidget);
  scrollArea->setWidgetResizable(true);


  QVBoxLayout *generalLayout = new QVBoxLayout;
  generalLayout->setContentsMargins(0, 0, 0, 0);
  generalLayout->addWidget(scrollArea);



  QHBoxLayout *buttonsLayout = new QHBoxLayout;

  QPushButton *cancelButton = new QPushButton(tr("Cancel"));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(OnCancel()));

  this->createButton = new QPushButton(tr("Create"));
  this->createButton->setEnabled(false);
  connect(this->createButton, SIGNAL(clicked()), this, SLOT(OnCreate()));

  buttonsLayout->addWidget(cancelButton);
  buttonsLayout->addWidget(createButton);
  buttonsLayout->setAlignment(Qt::AlignRight);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(generalLayout);
  mainLayout->addLayout(buttonsLayout);

  this->setMinimumWidth(500);
  this->setMinimumHeight(300);

  this->setLayout(mainLayout);

  this->connections.push_back(
      gui::model::Events::ConnectLinkInserted(
      boost::bind(&JointCreationDialog::OnLinkInserted, this, _1)));

  this->connections.push_back(
      gui::model::Events::ConnectLinkRemoved(
      boost::bind(&JointCreationDialog::OnLinkRemoved, this, _1)));

  this->connections.push_back(
      gui::model::Events::ConnectJointParentChosen3D(
      boost::bind(&JointCreationDialog::OnParentFrom3D, this, _1)));

  this->connections.push_back(
      gui::model::Events::ConnectJointChildChosen3D(
      boost::bind(&JointCreationDialog::OnChildFrom3D, this, _1)));
}

/////////////////////////////////////////////////
void JointCreationDialog::Open(JointMaker::JointType _type)
{
  if (!this->jointMaker)
  {
    gzerr << "Joint maker not found, can't open joint creation dialog."
        << std::endl;
    this->OnCancel();
  }

  // Check joint type
  this->typeButtons->button(static_cast<int>(_type))->setChecked(true);

  // Fill link combo boxes

  this->open();
}

/////////////////////////////////////////////////
void JointCreationDialog::OnTypeFromDialog(int _type)
{
}

/////////////////////////////////////////////////
void JointCreationDialog::OnParentFromDialog(int _index)
{
  QString linkName = this->parentComboBox->itemData(_index).toString();

  if (linkName.isEmpty())
    return;

  this->childComboBox->setEnabled(true);

  gui::model::Events::jointParentChosenDialog(linkName.toStdString());

  // Remove empty option
  int index = this->parentComboBox->findData("");
  this->parentComboBox->removeItem(index);
}

/////////////////////////////////////////////////
void JointCreationDialog::OnChildFromDialog(int _index)
{
  QString linkName = this->parentComboBox->itemData(_index).toString();

  if (linkName.isEmpty())
    return;

  this->createButton->setEnabled(true);

  gui::model::Events::jointChildChosenDialog(linkName.toStdString());

  // Remove empty option
  int index = this->parentComboBox->findData("");
  this->parentComboBox->removeItem(index);
}

/////////////////////////////////////////////////
void JointCreationDialog::OnParentFrom3D(const std::string &_linkName)
{
  if (_linkName.empty())
    return;

  int index = this->parentComboBox->findData(QString::fromStdString(_linkName));

  if (index == -1)
  {
    gzerr << "Requested link [" << _linkName << "] not found" << std::endl;
    return;
  }

  this->parentComboBox->setCurrentIndex(index);

  this->childComboBox->setEnabled(true);

  // Remove empty option
  index = this->parentComboBox->findData("");
  this->parentComboBox->removeItem(index);
}

/////////////////////////////////////////////////
void JointCreationDialog::OnChildFrom3D(const std::string &_linkName)
{
  if (_linkName.empty())
    return;

  if (!this->childComboBox->isEnabled())
  {
    gzerr << "Shouldn't set child link before setting parent." << std::endl;
    return;
  }

  int index = this->childComboBox->findData(QString::fromStdString(_linkName));

  if (index == -1)
  {
    gzerr << "Requested link [" << _linkName << "] not found" << std::endl;
    return;
  }

  this->childComboBox->setCurrentIndex(index);
  this->createButton->setEnabled(true);

  // Remove empty option
  index = this->parentComboBox->findData("");
  this->parentComboBox->removeItem(index);
}

/////////////////////////////////////////////////
void JointCreationDialog::OnLinkInserted(const std::string &_linkName)
{
  std::string leafName = _linkName;
  size_t idx = _linkName.find_last_of("::");
  if (idx != std::string::npos)
    leafName = _linkName.substr(idx+1);

  this->parentComboBox->addItem(
          QString::fromStdString(leafName), QString::fromStdString(_linkName));
  this->childComboBox->addItem(
          QString::fromStdString(leafName), QString::fromStdString(_linkName));
}

/////////////////////////////////////////////////
void JointCreationDialog::OnLinkRemoved(const std::string &_linkName)
{
  int index = this->parentComboBox->findData(QString::fromStdString(_linkName));
  this->parentComboBox->removeItem(index);
  this->childComboBox->removeItem(index);
}

/////////////////////////////////////////////////
void JointCreationDialog::OnCancel()
{
  this->close();
}

/////////////////////////////////////////////////
void JointCreationDialog::OnCreate()
{
  this->accept();
}

/////////////////////////////////////////////////
void JointCreationDialog::enterEvent(QEvent */*_event*/)
{
  QApplication::setOverrideCursor(Qt::ArrowCursor);
}
