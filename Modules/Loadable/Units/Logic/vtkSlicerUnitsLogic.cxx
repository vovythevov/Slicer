/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Units Logic includes
#include "vtkSlicerUnitsLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLUnitNode.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkNew.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerUnitsLogic);

//----------------------------------------------------------------------------
vtkSlicerUnitsLogic::vtkSlicerUnitsLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerUnitsLogic::~vtkSlicerUnitsLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerUnitsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkMRMLUnitNode* vtkSlicerUnitsLogic
::AddUnitNode(const char* name, const char* quantity, const char* prefix,
              const char* suffix, int precision, double min, double max)
{
  if (!this->GetMRMLScene())
    {
    return 0;
    }

  vtkMRMLUnitNode* unitNode = vtkMRMLUnitNode::New();
  unitNode->SetName(name);
  unitNode->SetQuantity(quantity);
  unitNode->SetPrefix(prefix);
  unitNode->SetSuffix(suffix);
  unitNode->SetPrecision(precision);
  unitNode->SetMinimumValue(min);
  unitNode->SetMaximumValue(max);

  this->GetMRMLScene()->AddNode(unitNode);
  unitNode->Delete();
  return unitNode;
}

//---------------------------------------------------------------------------
void vtkSlicerUnitsLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
  this->Superclass::SetMRMLSceneInternal(newScene);

  this->AddDefaultUnits();
}

//---------------------------------------------------------------------------
void vtkSlicerUnitsLogic::AddDefaultUnits()
{
  // Add defaults nodes here
  vtkMRMLUnitNode* node = this->AddUnitNode("Meter", "length", "", "m", 3);
  node->SetSaveWithScene(false);
  node = this->AddUnitNode("Centimeter", "length", "", "cm", 3);
  node->SetSaveWithScene(false);
  node = this->AddUnitNode("Millimeter", "length", "", "mm", 3);
  node->SetSaveWithScene(false);
  node = this->AddUnitNode("Micrometer", "length", "", "um", 3);
  node->SetSaveWithScene(false);

  node = this->AddUnitNode("Second", "time", "", "s", 3);
  node->SetSaveWithScene(false);
  node = this->AddUnitNode("Millisecond", "time", "", "ms", 3);
  node->SetSaveWithScene(false);
}

//-----------------------------------------------------------------------------
void vtkSlicerUnitsLogic::SetDefaultUnit(const char* quantity, const char* id)
{
  if (!quantity || !this->GetMRMLScene())
    {
    return;
    }

  vtkMRMLSelectionNode* selectionNode =  vtkMRMLSelectionNode::SafeDownCast(
    this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
  if (selectionNode)
    {
    selectionNode->SetUnitNodeID(quantity, id);
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerUnitsLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);

  vtkNew<vtkMRMLUnitNode> unitNode;
  this->GetMRMLScene()->RegisterNodeClass( unitNode.GetPointer() );
}
