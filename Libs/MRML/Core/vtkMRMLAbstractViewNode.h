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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLAbstractViewNode_h
#define __vtkMRMLAbstractViewNode_h

// MRML includes
#include "vtkMRMLNode.h"

/// \brief Abstract MRML node to represent a view.
/// The class holds the properties common to any view type (3D, slice, chart..)
/// Views are not hidden from editors by default (HideFromEditor is 0)
class VTK_MRML_EXPORT vtkMRMLAbstractViewNode
  : public vtkMRMLNode
{
public:
  vtkTypeMacro(vtkMRMLAbstractViewNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  ///
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Name of the layout. Must be unique between all the view nodes of the
  /// same type because it is used as a singleton tag.
  /// Typical names can be colors "Red", "Green", "Yellow",...
  /// or numbers "1", "2"... to uniquely define the node.
  /// No name (i.e. "") by default.
  /// \sa SetSingletonTag(), SetViewLabel()
  inline void SetLayoutName(const char *layoutName);
  inline const char *GetLayoutName();

  ///
  /// Label for the view. Usually a 1 character label, e.g. R, 1, 2, etc.
  /// \sa SetLayoutName()
  vtkSetStringMacro(LayoutLabel);
  vtkGetStringMacro(LayoutLabel);

  ///
  /// Indicates whether or not the view is active
  vtkGetMacro (Active, int );
  vtkSetMacro (Active, int );

  ///
  /// Indicates whether or not the view is visible (if it is not visible,
  /// then the view is not shown in any of the view layouts, but can be privately
  /// used by modules)
  vtkGetMacro(Visibility, int);
  vtkSetMacro(Visibility, int);

  ///
  /// 1st background color of the view.
  /// Black (0,0,0) by default.
  /// \sa SetBackgroundColor2()
  vtkGetVector3Macro (BackgroundColor, double);
  vtkSetVector3Macro (BackgroundColor, double);

  ///
  /// 2nd background color of the view
  /// Black (0,0,0) by default.
  /// \sa SetBackgroundColor2()
  vtkGetVector3Macro (BackgroundColor2, double);
  vtkSetVector3Macro (BackgroundColor2, double);

  /// Add View Node ID for the view to display this node in.
  /// \sa ViewNodeIDs, RemoveViewNodeID(), RemoveAllViewNodeIDs()
  void AddViewNodeID(const char* viewNodeID);
  /// Remove View Node ID for the view to display this node in.
  /// \sa ViewNodeIDs, AddViewNodeID(), RemoveAllViewNodeIDs()
  void RemoveViewNodeID(const char* viewNodeID);
  /// Remove All View Node IDs for the views to display this node in.
  /// \sa ViewNodeIDs, AddViewNodeID(), RemoveViewNodeID()
  void RemoveAllViewNodeIDs();
  /// Get number of View Node ID's for the view to display this node in.
  /// If 0, display in all views
  /// \sa ViewNodeIDs, GetViewNodeIDs(), AddViewNodeID()
  int GetNumberOfViewNodeIDs();
  /// Get View Node ID's for the view to display this node in.
  /// If NULL, display in all views
  /// \sa ViewNodeIDs, GetViewNodeIDs(), AddViewNodeID()
  const char* GetNthViewNodeID(int index);
  /// Get all View Node ID's for the view to display this node in.
  /// If empty, display in all views
  /// \sa ViewNodeIDs, GetNthViewNodeID(), AddViewNodeID()
  std::vector< std::string > GetViewNodeIDs();
  /// True if the view node id is present in the viewnodeid list
  /// false if not found
  /// \sa ViewNodeIDs, IsDisplayableInView(), AddViewNodeID()
  bool IsViewNodeIDPresent(const char* viewNodeID);
  /// Returns true if the viewNodeID is present in the ViewNodeId list
  /// or there is no ViewNodeId in the list (meaning all the views display the
  /// node)
  /// \sa ViewNodeIDs, IsViewNodeIDPresent(), AddViewNodeID()
  bool IsDisplayableInView(const char* viewNodeID);

protected:
  vtkMRMLAbstractViewNode();
  ~vtkMRMLAbstractViewNode();

  vtkMRMLAbstractViewNode(const vtkMRMLAbstractViewNode&);
  void operator=(const vtkMRMLAbstractViewNode&);

  static const char* ViewNodeReferenceRole;
  static const char* ViewNodeReferenceMRMLAttributeName;

  virtual const char* GetViewNodeReferenceRole()const;
  virtual const char* GetViewNodeReferenceMRMLAttributeName()const;

  ///
  /// Label to show for the view (shortcut for the name)
  char * LayoutLabel;

  ///
  /// Indicates whether or not the View is visible.
  /// Invisible (0) by default.
  int Visibility;

  ///
  /// Indicates whether or not the View is active.
  /// Inactive (1) by default.
  int Active;

  ///
  /// When a view is set Active, make other views inactive.
  virtual void RemoveActiveFlagInScene();

  ///
  /// Background colors
  double BackgroundColor[3];
  double BackgroundColor2[3];
};

//------------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::SetLayoutName(const char *layoutName)
{
  this->SetSingletonTag(layoutName);
}

//------------------------------------------------------------------------------
const char *vtkMRMLAbstractViewNode::GetLayoutName()
{
  return this->GetSingletonTag();
}

#endif
