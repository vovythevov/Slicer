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

#ifndef __qMRMLSliderWidget_h
#define __qMRMLSliderWidget_h

// CTK includes
#include <ctkSliderWidget.h>
#include <ctkVTKObject.h>

// qMRML includes
#include <qMRMLWidgetsExport.h>

// VTK includes
class vtkMRMLNode;
class vtkMRMLScene;

// qMRMLSliderWidget includes
class qMRMLSliderWidgetPrivate;

/// \class qMRMLSliderWidget
/// \brief Extend the ctkSliderWidget to integrate units supports
///
/// This custom widgets extends the ctkSliderWidget widget to integrate the
/// unit support within Slicer. By default, this widget behaves just like
/// a normal ctkSliderWidget.
///
/// To use the units, one needs to set what kind of quantity this widget should
/// look for. For example, when dealing with world positions, the quantity is
/// probably going to be "length". Once a scene is set to this widget,
/// it listens to the changes made upon the selection node to extract the
/// unit properties related to its quantity and update accordingly.
///
class QMRML_WIDGETS_EXPORT qMRMLSliderWidget : public ctkSliderWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(vtkMRMLScene* mrmlScene READ mrmlScene WRITE setMRMLScene)
  Q_PROPERTY(QString quantity READ quantity WRITE setQuantity)

public:
  typedef ctkSliderWidget Superclass;

  /// Construct an empty qMRMLSliderWidget with a null scene.
  explicit qMRMLSliderWidget(QWidget* parent = 0);
  virtual ~qMRMLSliderWidget();

  /// Get MRML scene that has been set by setMRMLScene(). Default is no scene.
  /// \sa setMRMLScene
  vtkMRMLScene* mrmlScene()const;

  /// Get the quantity is used to determine what unit the spinbox is in.
  /// This determines the spinboxes properties like minimum, maximum,
  /// single step, prefix and suffix.
  // \sa setQuantity
  QString quantity()const;

public slots:
   /// Set the quantity the widget should look for.
  /// \sa quantity()
  void setQuantity(const QString& baseName);

  /// Set the scene the spinbox listens to.
  /// \sa mrmlScene()
  virtual void setMRMLScene(vtkMRMLScene* scene);

protected slots:
  void updateFromUnitNode();

protected:
  QScopedPointer<qMRMLSliderWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLSliderWidget);
  Q_DISABLE_COPY(qMRMLSliderWidget);
};
#endif
