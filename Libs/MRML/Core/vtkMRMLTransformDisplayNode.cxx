/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLTransformDisplayNode.h"

#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLVolumeNode.h"

#include <vtkColorTransferFunction.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkMRMLProceduralColorNode.h>

#include <sstream>

const char RegionReferenceRole[] = "region";
const char GlyphPointsReferenceRole[] = "glyphPoints";
const char* DISPLACEMENT_MAGNITUDE_SCALAR_NAME = "DisplacementMagnitude";
const char CONTOUR_LEVEL_SEPARATOR=' ';
const char* DEFAULT_COLOR_TABLE_NAME = "Displacement to color";

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransformDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLTransformDisplayNode::vtkMRMLTransformDisplayNode()
  :vtkMRMLDisplayNode()
{
  // Don't show transform nodes by default
  // to allow the users to adjust visualization parameters first
  this->Visibility = 0;
  this->SliceIntersectionVisibility = 0;

  this->ScalarVisibility=1;
  this->SetActiveScalarName(DISPLACEMENT_MAGNITUDE_SCALAR_NAME);

  this->VisualizationMode=VIS_MODE_GLYPH;

  this->GlyphSpacingMm=10.0;
  this->GlyphScalePercent=100;
  this->GlyphDisplayRangeMaxMm=100;
  this->GlyphDisplayRangeMinMm=0.01;
  this->GlyphType=GLYPH_TYPE_ARROW;
  this->GlyphTipLengthPercent=30;
  this->GlyphDiameterMm=5.0;
  this->GlyphShaftDiameterPercent=40;
  this->GlyphResolution=6;

  this->GridScalePercent=100;
  this->GridSpacingMm=15.0;
  this->GridLineDiameterMm=1.0;
  this->GridResolutionMm=5.0;
  this->GridShowNonWarped=false;

  this->ContourResolutionMm=5.0;
  this->ContourOpacity=0.8;
  this->ContourLevelsMm.clear();
  for (double level=2.0; level<20.0; level+=2.0)
    {
    this->ContourLevelsMm.push_back(level);
    }

  this->TranslationEnabled = 1;
  this->RotationEnabled = 1;
  this->ScalingEnabled = 1;

  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLTransformableNode::TransformModifiedEvent);
  this->AddNodeReferenceRole(RegionReferenceRole, RegionReferenceRole, events.GetPointer());
  this->AddNodeReferenceRole(GlyphPointsReferenceRole, GlyphPointsReferenceRole, events.GetPointer());
}


//----------------------------------------------------------------------------
vtkMRMLTransformDisplayNode::~vtkMRMLTransformDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent << " VisualizationMode=\""<< ConvertVisualizationModeToString(this->VisualizationMode) << "\"";

  of << indent << " GlyphSpacingMm=\""<< this->GlyphSpacingMm << "\"";
  of << indent << " GlyphScalePercent=\""<< this->GlyphScalePercent << "\"";
  of << indent << " GlyphDisplayRangeMaxMm=\""<< this->GlyphDisplayRangeMaxMm << "\"";
  of << indent << " GlyphDisplayRangeMinMm=\""<< this->GlyphDisplayRangeMinMm << "\"";
  of << indent << " GlyphType=\""<< ConvertGlyphTypeToString(this->GlyphType) << "\"";
  of << indent << " GlyphTipLengthPercent=\"" << this->GlyphTipLengthPercent << "\"";
  of << indent << " GlyphDiameterMm=\""<< this->GlyphDiameterMm << "\"";
  of << indent << " GlyphShaftDiameterPercent=\"" << this->GlyphShaftDiameterPercent << "\"";
  of << indent << " GlyphResolution=\"" << this->GlyphResolution << "\"";

  of << indent << " GridScalePercent=\""<< this->GridScalePercent << "\"";
  of << indent << " GridSpacingMm=\""<< this->GridSpacingMm << "\"";
  of << indent << " GridLineDiameterMm=\""<< this->GridLineDiameterMm << "\"";
  of << indent << " GridResolutionMm=\""<< this->GridResolutionMm << "\"";
  of << indent << " GridShowNonWarped=\""<< this->GridShowNonWarped << "\"";

  of << indent << " ContourResolutionMm=\""<< this->ContourResolutionMm << "\"";
  of << indent << " ContourLevelsMm=\"" << this->GetContourLevelsMmAsString() << "\"";
  of << indent << " ContourOpacity=\""<< this->ContourOpacity << "\"";

  of << indent << " TranslationEnabled=\""<< this->TranslationEnabled << "\"";
  of << indent << " RotationEnabled=\"" << this->RotationEnabled << "\"";
  of << indent << " ScalingEnabled=\""<< this->ScalingEnabled << "\"";
}


#define READ_FROM_ATT(varName)    \
  if (!strcmp(attName,#varName))  \
    {                             \
    std::stringstream ss;         \
    ss << attValue;               \
    ss >> this->varName;          \
    continue;                     \
    }


//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName,"VisualizationMode"))
      {
      this->VisualizationMode = ConvertVisualizationModeFromString(attValue);
      continue;
      }
    READ_FROM_ATT(GlyphSpacingMm);
    READ_FROM_ATT(GlyphScalePercent);
    READ_FROM_ATT(GlyphDisplayRangeMaxMm);
    READ_FROM_ATT(GlyphDisplayRangeMinMm);
    if (!strcmp(attName,"GlyphType"))
      {
      this->GlyphType = ConvertGlyphTypeFromString(attValue);
      continue;
      }
    READ_FROM_ATT(GlyphTipLengthPercent);
    READ_FROM_ATT(GlyphDiameterMm);
    READ_FROM_ATT(GlyphShaftDiameterPercent);
    READ_FROM_ATT(GlyphResolution);
    READ_FROM_ATT(GridScalePercent);
    READ_FROM_ATT(GridSpacingMm);
    READ_FROM_ATT(GridLineDiameterMm);
    READ_FROM_ATT(GridResolutionMm);
    READ_FROM_ATT(GridShowNonWarped);
    READ_FROM_ATT(ContourResolutionMm);
    READ_FROM_ATT(ContourOpacity);
    if (!strcmp(attName,"ContourLevelsMm"))
      {
      SetContourLevelsMmFromString(attValue);
      continue;
      }
    READ_FROM_ATT(TranslationEnabled);
    READ_FROM_ATT(RotationEnabled);
    READ_FROM_ATT(ScalingEnabled);
    }

  this->Modified();
  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLTransformDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLTransformDisplayNode *node = vtkMRMLTransformDisplayNode::SafeDownCast(anode);

  this->VisualizationMode = node->VisualizationMode;

  this->GlyphSpacingMm = node->GlyphSpacingMm;
  this->GlyphScalePercent = node->GlyphScalePercent;
  this->GlyphDisplayRangeMaxMm = node->GlyphDisplayRangeMaxMm;
  this->GlyphDisplayRangeMinMm = node->GlyphDisplayRangeMinMm;
  this->GlyphType = node->GlyphType;
  this->GlyphTipLengthPercent = node->GlyphTipLengthPercent;
  this->GlyphDiameterMm = node->GlyphDiameterMm;
  this->GlyphShaftDiameterPercent = node->GlyphShaftDiameterPercent;
  this->GlyphResolution = node->GlyphResolution;

  this->GridScalePercent = node->GridScalePercent;
  this->GridSpacingMm = node->GridSpacingMm;
  this->GridLineDiameterMm = node->GridLineDiameterMm;
  this->GridResolutionMm = node->GridResolutionMm;
  this->GridShowNonWarped = node->GridShowNonWarped;

  this->ContourResolutionMm = node->ContourResolutionMm;
  this->ContourOpacity = node->ContourOpacity;
  this->ContourLevelsMm = node->ContourLevelsMm;

  this->TranslationEnabled = node->TranslationEnabled;
  this->RotationEnabled = node->RotationEnabled;
  this->ScalingEnabled = node->ScalingEnabled;

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "VisualizationMode = "<< ConvertVisualizationModeToString(this->VisualizationMode) << "\n";
  os << indent << "GlyphScalePercent = "<< this->GlyphScalePercent << "\n";
  os << indent << "GlyphDisplayRangeMaxMm = "<< this->GlyphDisplayRangeMaxMm << "\n";
  os << indent << "GlyphDisplayRangeMinMm = "<< this->GlyphDisplayRangeMinMm << "\n";
  os << indent << "GlyphType = "<< ConvertGlyphTypeToString(this->GlyphType) << "\n";
  os << indent << "GlyphTipLengthPercent = " << this->GlyphTipLengthPercent << "\n";
  os << indent << "GlyphDiameterMm = " << this->GlyphDiameterMm << "\n";
  os << indent << "GlyphShaftDiameterPercent = " << this->GlyphShaftDiameterPercent << "\n";
  os << indent << "GlyphResolution = " << this->GlyphResolution << "\n";

  os << indent << "GridScalePercent = " << this->GridScalePercent << "\n";
  os << indent << "GridSpacingMm = " << this->GridSpacingMm << "\n";
  os << indent << "GridLineDiameterMm = " << this->GridLineDiameterMm << "\n";
  os << indent << "GridResolutionMm = " << this->GridResolutionMm << "\n";
  os << indent << "GridShowNonWarped = " << this->GridShowNonWarped << "\n";

  os << indent << "ContourResolutionMm = "<< this->ContourResolutionMm << "\n";
  os << indent << "ContourOpacity = " << this->ContourOpacity << "\n";
  os << indent << "ContourLevelsMm = " << GetContourLevelsMmAsString() << "\n";

  os << indent << " TranslationEnabled=\""<< this->TranslationEnabled << "\"";
  os << indent << " RotationEnabled=\"" << this->RotationEnabled << "\"";
  os << indent << " ScalingEnabled=\""<< this->ScalingEnabled << "\"";
}

//---------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{
  if (caller!=NULL
    && (event==vtkCommand::ModifiedEvent || event==vtkMRMLTransformableNode::TransformModifiedEvent)
    && caller==GetRegionNode()
    && this->Visibility)
    {
    // update visualization if the region node is changed
    // Note: this updates all the 2D views as well, so instead of a generic modified event a separate
    // even for 2D and 3D views could be useful.
    // If 3D visibility is disabled then we can ignore this event, as the region is only used for 3D display.
    this->Modified();
    }
  else if (caller!=NULL
    && (event==vtkCommand::ModifiedEvent || event==vtkMRMLTransformableNode::TransformModifiedEvent)
    && caller==GetGlyphPointsNode()
    && this->VisualizationMode == VIS_MODE_GLYPH
    && (this->Visibility || this->GetSliceIntersectionVisibility()) )
    {
    // update visualization if glyph points are changed
    this->Modified();
    }
  else if (caller!=NULL
    && event==vtkCommand::ModifiedEvent
    && caller==GetColorNode())
    {
    // update visualization if the color node is changed
    this->Modified();
    }
  else this->Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLTransformDisplayNode::GetRegionNode()
{
  return this->GetNodeReference(RegionReferenceRole);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetAndObserveRegionNode(vtkMRMLNode* node)
{
  this->SetAndObserveNthNodeReferenceID(RegionReferenceRole, 0, node ? node->GetID() : NULL);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLTransformDisplayNode::GetGlyphPointsNode()
{
  return this->GetNodeReference(GlyphPointsReferenceRole);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetAndObserveGlyphPointsNode(vtkMRMLNode* node)
{
  this->SetAndObserveNthNodeReferenceID(GlyphPointsReferenceRole, 0, node ? node->GetID() : NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetContourLevelsMm(double* values, int size)
{
  this->ContourLevelsMm.clear();
  for (int i=0; i<size; i++)
    {
    this->ContourLevelsMm.push_back(values[i]);
    }
  this->Modified();
}

//----------------------------------------------------------------------------
double* vtkMRMLTransformDisplayNode::GetContourLevelsMm()
{
  if (this->ContourLevelsMm.size()==0)
    {
    return NULL;
    }
  // std::vector values are guaranteed to be stored in a continuous block in memory,
  // so we can return the address to the first one
  return &(this->ContourLevelsMm[0]);
}

//----------------------------------------------------------------------------
unsigned int vtkMRMLTransformDisplayNode::GetNumberOfContourLevels()
{
  return this->ContourLevelsMm.size();
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformDisplayNode::ConvertVisualizationModeToString(int modeIndex)
{
  switch (modeIndex)
    {
    case VIS_MODE_GLYPH: return "GLYPH";
    case VIS_MODE_GRID: return "GRID";
    case VIS_MODE_CONTOUR: return "CONTOUR";
    case VIS_MODE_INTERACTIVE: return "INTERACTIVE";
    default: return "";
    }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformDisplayNode::ConvertVisualizationModeFromString(const char* modeString)
{
  if (modeString==NULL)
    {
    return -1;
    }
  for (int modeIndex=0; modeIndex<VIS_MODE_LAST; modeIndex++)
    {
    if (strcmp(modeString, vtkMRMLTransformDisplayNode::ConvertVisualizationModeToString(modeIndex))==0)
      {
      return modeIndex;
      }
    }
  return -1;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransformDisplayNode::ConvertGlyphTypeToString(int modeIndex)
{
  switch (modeIndex)
    {
    case GLYPH_TYPE_ARROW: return "ARROW";
    case GLYPH_TYPE_CONE: return "CONE";
    case GLYPH_TYPE_SPHERE: return "SPHERE";
    default: return "";
    }
}

//----------------------------------------------------------------------------
int vtkMRMLTransformDisplayNode::ConvertGlyphTypeFromString(const char* modeString)
{
  if (modeString==NULL)
    {
    return -1;
    }
  for (int modeIndex=0; modeIndex<GLYPH_TYPE_LAST; modeIndex++)
    {
    if (strcmp(modeString,vtkMRMLTransformDisplayNode::ConvertGlyphTypeToString(modeIndex))==0)
      {
      return modeIndex;
      }
    }
  return -1;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTransformDisplayNode::GetContourLevelsMmAsString()
{
  return ConvertContourLevelsToString(this->ContourLevelsMm);
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetContourLevelsMmFromString(const char* str)
{
  std::vector<double> newLevels=this->ConvertContourLevelsFromString(str);
  if (this->IsContourLevelEqual(newLevels, this->ContourLevelsMm))
    {
    // no change
    return;
    }
  this->ContourLevelsMm=newLevels;
  this->Modified();
}

//----------------------------------------------------------------------------
std::vector<double> vtkMRMLTransformDisplayNode::ConvertContourLevelsFromString(const char* str)
{
  return vtkMRMLTransformDisplayNode::StringToDoubleVector(str);
}

//----------------------------------------------------------------------------
std::vector<double> vtkMRMLTransformDisplayNode::StringToDoubleVector(const char* str)
{
  std::vector<double> values;
  std::stringstream ss(str);
  std::string itemString;
  double itemDouble;
  while (std::getline(ss, itemString, CONTOUR_LEVEL_SEPARATOR))
    {
    std::stringstream itemStream;
    itemStream << itemString;
    itemStream >> itemDouble;
    values.push_back(itemDouble);
    }
  return values;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTransformDisplayNode::ConvertContourLevelsToString(const std::vector<double>& levels)
{
  return vtkMRMLTransformDisplayNode::DoubleVectorToString(&(levels[0]), levels.size());
}

//----------------------------------------------------------------------------
std::string vtkMRMLTransformDisplayNode::DoubleVectorToString(const double* values, int numberOfValues)
{
  std::stringstream ss;
  for (int i=0; i<numberOfValues; i++)
    {
    if (i>0)
      {
      ss << CONTOUR_LEVEL_SEPARATOR;
      }
    ss << values[i];
    }
  return ss.str();
}

//----------------------------------------------------------------------------
bool vtkMRMLTransformDisplayNode::IsContourLevelEqual(const std::vector<double>& levels1, const std::vector<double>& levels2)
{
  if (levels1.size()!=levels2.size())
    {
    return false;
    }
  const double COMPARISON_TOLERANCE=0.01;
  for (unsigned int i=0; i<levels1.size(); i++)
  {
    if (fabs(levels1[i]-levels2[i])>COMPARISON_TOLERANCE)
      {
      return false;
      }
  }
  return true;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::GetContourLevelsMm(std::vector<double> &levels)
{
  levels=this->ContourLevelsMm;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetDefaultColors()
{
  if (!this->GetScene())
    {
    vtkErrorMacro("vtkMRMLTransformDisplayNode::SetDefaultColors failed: scene is not set");
    return;
    }

  // Create and set a new color table node
  vtkNew<vtkMRMLProceduralColorNode> colorNode;
  colorNode->SetName(this->GetScene()->GenerateUniqueName(DEFAULT_COLOR_TABLE_NAME).c_str());
  colorNode->SetAttribute("Category", "Transform display");
  // The color node is a procedural color node, which is saved using a storage node.
  // Hidden nodes are not saved if they use a storage node, therefore
  // the color node must be visible.
  colorNode->SetHideFromEditors(false);

  vtkColorTransferFunction* colorMap=colorNode->GetColorTransferFunction();
  // Map: mm -> RGB
  colorMap->AddRGBPoint( 1.0,  0.2, 0.2, 0.2);
  colorMap->AddRGBPoint( 2.0,  0.0, 1.0, 0.0);
  colorMap->AddRGBPoint( 5.0,  1.0, 1.0, 0.0);
  colorMap->AddRGBPoint(10.0,  1.0, 0.0, 0.0);

  this->GetScene()->AddNode(colorNode.GetPointer());
  this->SetAndObserveColorNodeID(colorNode->GetID());
}

//----------------------------------------------------------------------------
vtkColorTransferFunction* vtkMRMLTransformDisplayNode::GetColorMap()
{
  vtkMRMLProceduralColorNode* colorNode=vtkMRMLProceduralColorNode::SafeDownCast(GetColorNode());
  if (colorNode==NULL
    || colorNode->GetColorTransferFunction()==NULL
    || colorNode->GetColorTransferFunction()->GetSize()==0)
    {
    // We don't have a color node or it is not the right type
    this->SetDefaultColors();
    colorNode=vtkMRMLProceduralColorNode::SafeDownCast(this->GetColorNode());
    if (colorNode==NULL)
      {
      vtkErrorMacro("vtkMRMLTransformDisplayNode::GetColorMap failed: could not create default color node");
      return NULL;
      }
    }
  vtkColorTransferFunction* colorMap=colorNode->GetColorTransferFunction();
  return colorMap;
}

//----------------------------------------------------------------------------
void vtkMRMLTransformDisplayNode::SetColorMap(vtkColorTransferFunction* newColorMap)
{
  int oldModified=this->StartModify();
  vtkMRMLProceduralColorNode* colorNode=vtkMRMLProceduralColorNode::SafeDownCast(this->GetColorNode());
  if (colorNode==NULL)
    {
    // We don't have a color node or it is not the right type
    this->SetDefaultColors();
    colorNode=vtkMRMLProceduralColorNode::SafeDownCast(this->GetColorNode());
    }
  if (colorNode!=NULL)
    {
    if (colorNode->GetColorTransferFunction()==NULL)
      {
      vtkNew<vtkColorTransferFunction> ctf;
      colorNode->SetAndObserveColorTransferFunction(ctf.GetPointer());
      }
    if (!vtkMRMLProceduralColorNode::IsColorMapEqual(colorNode->GetColorTransferFunction(),newColorMap))
      {
      colorNode->GetColorTransferFunction()->DeepCopy(newColorMap);
      }
    }
  else
    {
    vtkErrorMacro("vtkMRMLTransformDisplayNode::SetColorMap failed: could not create default color node");
    }
  this->EndModify(oldModified);
}
