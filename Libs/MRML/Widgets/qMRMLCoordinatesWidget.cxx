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

#include "qMRMLCoordinatesWidget.h"

// Qt includes
#include <QHBoxLayout>

// qMRML includes
#include "qMRMLSpinBox.h"

// MRML nodes includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLUnitNode.h>

// --------------------------------------------------------------------------
class qMRMLCoordinatesWidgetPrivate
{
  Q_DECLARE_PUBLIC(qMRMLCoordinatesWidget);
protected:
  qMRMLCoordinatesWidget* const q_ptr;
public:
  qMRMLCoordinatesWidgetPrivate(qMRMLCoordinatesWidget& object);

  QString Quantity;
  vtkMRMLScene* MRMLScene;
  vtkMRMLSelectionNode* SelectionNode;

  void setAndObserveSelectionNode();
};

// --------------------------------------------------------------------------
qMRMLCoordinatesWidgetPrivate
::qMRMLCoordinatesWidgetPrivate(qMRMLCoordinatesWidget& object)
: q_ptr(&object)
{
  this->MRMLScene = 0;
  this->SelectionNode = 0;
}

// --------------------------------------------------------------------------
void qMRMLCoordinatesWidgetPrivate::setAndObserveSelectionNode()
{
  Q_Q(qMRMLCoordinatesWidget);

  vtkMRMLSelectionNode* selectionNode = 0;
  if (this->MRMLScene)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));
    Q_ASSERT(selectionNode);
    }

  q->qvtkReconnect(this->SelectionNode, selectionNode,
    vtkMRMLSelectionNode::UnitModifiedEvent,
    q, SLOT(updateFromUnitNode()));
  this->SelectionNode = selectionNode;
  q->updateFromUnitNode();
}

// --------------------------------------------------------------------------
// qMRMLCoordinatesWidget

//------------------------------------------------------------------------------
qMRMLCoordinatesWidget::qMRMLCoordinatesWidget(QWidget* _parent)
: Superclass(_parent), d_ptr(new qMRMLCoordinatesWidgetPrivate(*this))
{
}

//------------------------------------------------------------------------------
qMRMLCoordinatesWidget::~qMRMLCoordinatesWidget()
{
}

//-----------------------------------------------------------------------------
void qMRMLCoordinatesWidget::setQuantity(const QString& quantity)
{
  Q_D(qMRMLCoordinatesWidget);
  if (quantity == d->Quantity)
    {
    return;
    }

  d->Quantity = quantity;
  this->updateFromUnitNode();
}

//-----------------------------------------------------------------------------
QString qMRMLCoordinatesWidget::quantity()const
{
  Q_D(const qMRMLCoordinatesWidget);
  return d->Quantity;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLCoordinatesWidget::mrmlScene()const
{
  Q_D(const qMRMLCoordinatesWidget);
  return d->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLCoordinatesWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLCoordinatesWidget);

  if (this->mrmlScene() == scene)
    {
    return;
    }

  d->MRMLScene = scene;
  d->setAndObserveSelectionNode();
  this->setEnabled(this->isEnabled() && scene != 0);
}

// --------------------------------------------------------------------------
void qMRMLCoordinatesWidget::updateFromUnitNode()
{
  Q_D(qMRMLCoordinatesWidget);

  if (d->SelectionNode)
    {
    vtkMRMLUnitNode* unitNode =
      vtkMRMLUnitNode::SafeDownCast(d->MRMLScene->GetNodeByID(
        d->SelectionNode->GetUnitNodeID(d->Quantity.toLatin1())));

    if (unitNode)
      {
      for (int i = 0; this->layout()->itemAt(i); ++i)
        {
        QLayoutItem* item = this->layout()->itemAt(i);
        ctkSpinBox* spinBox =
          item ? qobject_cast<ctkSpinBox*>(item->widget()) : 0;
        if (spinBox)
          {
          spinBox->setSingleStep(1.0 / unitNode->GetPrecision());
          spinBox->setDecimals(unitNode->GetPrecision());
          spinBox->setRange(unitNode->GetMinimumValue(),
            unitNode->GetMaximumValue());
          spinBox->setPrefix(unitNode->GetPrefix());
          spinBox->setSuffix(unitNode->GetSuffix());
          }
        }
      }
    }
}
