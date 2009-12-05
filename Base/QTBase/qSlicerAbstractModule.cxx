#include "qSlicerAbstractModule.h"

// SlicerQT includes
#include "qSlicerAbstractModuleWidget.h"

// SlicerLogic includes
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
struct qSlicerAbstractModulePrivate: public qCTKPrivate<qSlicerAbstractModule>
{
  qSlicerAbstractModulePrivate()
    {
    this->ModuleEnabled = false;
    this->WidgetRepresentation = 0;
    }
  ~qSlicerAbstractModulePrivate();
  
  bool                          ModuleEnabled;
  qSlicerAbstractModuleWidget*  WidgetRepresentation;

  vtkSmartPointer<vtkMRMLScene>              MRMLScene;
  vtkSmartPointer<vtkSlicerApplicationLogic> AppLogic;
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::initialize(vtkSlicerApplicationLogic* appLogic)
{
  Q_ASSERT(appLogic);
  //this->setAppLogic(appLogic);
  qctk_d()->AppLogic = appLogic; 
  this->setup();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::printAdditionalInfo()
{
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerAbstractModule, vtkMRMLScene*, mrmlScene, MRMLScene);

//-----------------------------------------------------------------------------
void qSlicerAbstractModule::setMRMLScene(vtkMRMLScene* mrmlScene)
{
  QCTK_D(qSlicerAbstractModule);
  d->MRMLScene = mrmlScene;
  if (d->WidgetRepresentation)
    {
    d->WidgetRepresentation->setMRMLScene(mrmlScene);
    }
}

//-----------------------------------------------------------------------------
//QCTK_SET_CXX(qSlicerAbstractModule, vtkSlicerApplicationLogic*, setAppLogic, AppLogic);
QCTK_GET_CXX(qSlicerAbstractModule, vtkSlicerApplicationLogic*, appLogic, AppLogic);

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerAbstractModule, bool, moduleEnabled, ModuleEnabled);
QCTK_SET_CXX(qSlicerAbstractModule, bool, setModuleEnabled, ModuleEnabled);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget* qSlicerAbstractModule::widgetRepresentation()
{
  QCTK_D(qSlicerAbstractModule);
  if (!d->WidgetRepresentation)
    {
    d->WidgetRepresentation = this->createWidgetRepresentation();
    Q_ASSERT(d->WidgetRepresentation);
    d->WidgetRepresentation->setModule(this);
    d->WidgetRepresentation->initialize();
    d->WidgetRepresentation->setWindowTitle(this->title());
    d->WidgetRepresentation->setMRMLScene(this->mrmlScene());
    }
  return d->WidgetRepresentation; 
}

//-----------------------------------------------------------------------------
// qSlicerAbstractModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerAbstractModulePrivate::~qSlicerAbstractModulePrivate()
{
  if (this->WidgetRepresentation)
    {
    delete this->WidgetRepresentation;
    }
}
