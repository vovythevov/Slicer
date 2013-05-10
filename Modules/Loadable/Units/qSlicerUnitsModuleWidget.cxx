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

==============================================================================*/

// CTK includes
#include <ctkComboBox.h>

// Qt includes
#include <QDebug>
#include <QWidget>

// SlicerQt includes
#include "qSlicerUnitsModuleWidget.h"
#include "ui_qSlicerUnitsModuleWidget.h"

// MRML includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLUnitNode.h"

// STD
#include <vector>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerUnitsModuleWidgetPrivate: public Ui_qSlicerUnitsModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerUnitsModuleWidget);
protected:
  qSlicerUnitsModuleWidget* const q_ptr;

public:
  qSlicerUnitsModuleWidgetPrivate(qSlicerUnitsModuleWidget& obj);
};

//-----------------------------------------------------------------------------
// qSlicerUnitsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerUnitsModuleWidgetPrivate::qSlicerUnitsModuleWidgetPrivate(
  qSlicerUnitsModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
// qSlicerUnitsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerUnitsModuleWidget::qSlicerUnitsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerUnitsModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerUnitsModuleWidget::~qSlicerUnitsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setup()
{
  Q_D(qSlicerUnitsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  QObject::connect(d->UnitNodeComboBox,
    SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(updateWidgetFromNode()));

  QObject::connect(d->QuantityComboBox, SIGNAL(currentIndexChanged(int)),
    this, SLOT(setQuantity()));
  QObject::connect(d->QuantityComboBox,
    SIGNAL(editTextChanged(const QString &)),
    this, SLOT(setQuantity(const QString &)));
  QObject::connect(d->SuffixLineEdit, SIGNAL(editingFinished()),
    this, SLOT(setSuffix()));
  QObject::connect(d->PrefixLineEdit, SIGNAL(editingFinished()),
    this, SLOT(setPrefix()));
  QObject::connect(d->PrecisionSpinBox, SIGNAL(valueChanged(int)),
    this, SLOT(setPrecision(int)));
  QObject::connect(d->MaximumSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(setMinimum(double)));
  QObject::connect(d->MinimumSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(setMaximum(double)));

  // \todo remove this for supporting user-custom nodes
  d->UnitNodeComboBox->setAddEnabled(false);
  d->UnitNodeComboBox->setRemoveEnabled(false);
  d->UnitNodeComboBox->setRenameEnabled(false);
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::updateWidgetFromNode()
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode)
    {
    // Quantity
    d->QuantityComboBox->setEnabled(0);
    d->PrefixLineEdit->setEnabled(0);
    d->SuffixLineEdit->setEnabled(0);
    d->PrecisionSpinBox->setEnabled(0);
    d->MinimumSpinBox->setEnabled(0);
    d->MaximumSpinBox->setEnabled(0);
    return;
    }

  d->UnitNodeComboBox->setRemoveEnabled(unitNode->GetSaveWithScene());
  d->UnitNodeComboBox->setRenameEnabled(unitNode->GetSaveWithScene());

  // Quantity
  int index = d->QuantityComboBox->findText(unitNode->GetQuantity(),
      Qt::MatchFixedString);
  if (index == -1)
    {
    // Set quantity to the current text displayd
    unitNode->SetQuantity(d->QuantityComboBox->currentText().toLatin1());
    }
  else
    {
    d->QuantityComboBox->setCurrentIndex(index);
    }

  // Suffix
  d->SuffixLineEdit->setText(unitNode->GetSuffix());

  // Prefix
  d->PrefixLineEdit->setText(QString(unitNode->GetPrefix()));

  // Precision
  d->PrecisionSpinBox->setValue(unitNode->GetPrecision());

  // Min
  d->MinimumSpinBox->setValue(unitNode->GetMinimumValue());

  // Max
  d->MaximumSpinBox->setValue(unitNode->GetMaximumValue());

  d->QuantityComboBox->setEnabled(unitNode->GetSaveWithScene());
  d->PrefixLineEdit->setEnabled(unitNode->GetSaveWithScene());
  d->SuffixLineEdit->setEnabled(unitNode->GetSaveWithScene());
  d->PrecisionSpinBox->setEnabled(unitNode->GetSaveWithScene());
  d->MinimumSpinBox->setEnabled(unitNode->GetSaveWithScene());
  d->MaximumSpinBox->setEnabled(unitNode->GetSaveWithScene());

}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setPrefix()
{
  Q_D(qSlicerUnitsModuleWidget);

  QString prefix = d->PrefixLineEdit->text();

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene()
    || prefix == QString(unitNode->GetPrefix()))
    {
    return;
    }

  unitNode->SetPrefix(prefix.toLatin1());

  d->MaximumSpinBox->setPrefix(prefix);
  d->MinimumSpinBox->setPrefix(prefix);
}


//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setSuffix()
{
  Q_D(qSlicerUnitsModuleWidget);

  QString suffix = d->SuffixLineEdit->text();

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene()
    || suffix == QString(unitNode->GetSuffix()))
    {
    return;
    }

  unitNode->SetSuffix(suffix.toLatin1());

  d->MaximumSpinBox->setSuffix(suffix);
  d->MinimumSpinBox->setSuffix(suffix);
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setPrecision(int newPrecision)
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene()
    || newPrecision == unitNode->GetPrecision())
    {
    return;
    }

  unitNode->SetPrecision(newPrecision);
  d->MaximumSpinBox->setDecimals(newPrecision);
  d->MinimumSpinBox->setDecimals(newPrecision);
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setQuantity()
{
  Q_D(qSlicerUnitsModuleWidget);
  this->setQuantity(d->QuantityComboBox->currentText());
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setQuantity(const QString& newQuantity)
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene()
    || newQuantity == QString(unitNode->GetQuantity()))
    {
    return;
    }

  unitNode->SetQuantity(newQuantity.toLatin1());
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setMinimum(double newMin)
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene()
    || fabs(newMin - unitNode->GetMinimumValue()) < 1e-6)
    {
    return;
    }

  unitNode->SetMinimumValue(newMin);
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModuleWidget::setMaximum(double newMax)
{
  Q_D(qSlicerUnitsModuleWidget);

  vtkMRMLUnitNode* unitNode =
    vtkMRMLUnitNode::SafeDownCast(d->UnitNodeComboBox->currentNode());
  if (!unitNode || !unitNode->GetSaveWithScene()
    || fabs(newMax - unitNode->GetMaximumValue()) < 1e-6)
    {
    return;
    }

  unitNode->SetMaximumValue(newMax);
}
