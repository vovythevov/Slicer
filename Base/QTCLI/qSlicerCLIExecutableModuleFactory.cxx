/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QProcess>

// SlicerQt includes
#include "qSlicerCLIExecutableModuleFactory.h"
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleFactoryHelper.h"
#include "qSlicerUtils.h"

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactoryItem::qSlicerCLIExecutableModuleFactoryItem(
  const QString& newTempDirectory) : TempDirectory(newTempDirectory)
{
}

//-----------------------------------------------------------------------------
bool qSlicerCLIExecutableModuleFactoryItem::load()
{
  return true;
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerCLIExecutableModuleFactoryItem::instanciator()
{
  // Using a scoped pointer ensures the memory will be cleaned if instantiator
  // fails before returning the module. See QScopedPointer::take()
  QScopedPointer<qSlicerCLIModule> module(new qSlicerCLIModule());
  module->setModuleType("CommandLineModule");
  module->setEntryPoint(this->path());

  ctkScopedCurrentDir scopedCurrentDir(QFileInfo(this->path()).path());

  QProcess cli;
  cli.start(this->path(), QStringList(QString("--xml")));
  bool res = cli.waitForFinished(5000);
  if (!res)
    {
    return 0;
    }
  QString xmlDescription = cli.readAllStandardOutput();
  if (xmlDescription.isEmpty())
    {
    this->appendInstantiateErrorString(QString("CLI executable: %1").arg(this->path()));
    this->appendInstantiateErrorString("Failed to retrieve Xml Description");
    return 0;
    }
  if (!xmlDescription.startsWith("<?xml"))
    {
    this->appendInstantiateWarningString(QString("CLI executable: %1").arg(this->path()));
    this->appendInstantiateWarningString(QLatin1String("XML description doesn't start right away."));
    this->appendInstantiateWarningString(QString("Output before '<?xml' is [%1]").arg(
                                           xmlDescription.mid(0, xmlDescription.indexOf("<?xml"))));
    xmlDescription.remove(0, xmlDescription.indexOf("<?xml"));
    }

  module->setXmlModuleDescription(xmlDescription.toLatin1());
  module->setTempDirectory(this->TempDirectory);
  module->setPath(this->path());
  module->setInstalled(qSlicerCLIModuleFactoryHelper::isInstalled(this->path()));

  return module.take();
}

//-----------------------------------------------------------------------------
// qSlicerCLIExecutableModuleFactory

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactory::qSlicerCLIExecutableModuleFactory()
{
  this->TempDirectory = QDir::tempPath();
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactory::registerItems()
{
  this->registerAllFileItems(qSlicerCLIModuleFactoryHelper::modulePaths());
}

//-----------------------------------------------------------------------------
bool qSlicerCLIExecutableModuleFactory::isValidFile(const QFileInfo& file)const
{
  if (!this->Superclass::isValidFile(file))
    {
    return false;
    }
  if (!file.isExecutable())
    {
    return false;
    }
  return qSlicerUtils::isCLIExecutable(file.absoluteFilePath());
}

//-----------------------------------------------------------------------------
ctkAbstractFactoryItem<qSlicerAbstractCoreModule>* qSlicerCLIExecutableModuleFactory
::createFactoryFileBasedItem()
{
  return new qSlicerCLIExecutableModuleFactoryItem(this->TempDirectory);
}

//-----------------------------------------------------------------------------
QString qSlicerCLIExecutableModuleFactory::fileNameToKey(const QString& executableName)const
{
  return qSlicerUtils::extractModuleNameFromLibraryName(executableName);
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactory::setTempDirectory(const QString& newTempDirectory)
{
  this->TempDirectory = newTempDirectory;
}
