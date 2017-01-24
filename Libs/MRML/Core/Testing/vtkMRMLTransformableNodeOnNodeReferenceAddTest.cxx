/*=auto=========================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Yuzheng Zhou, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

namespace
{

int populateScene(vtkMRMLScene* scene);
int testScene(vtkMRMLScene* scene);

} // end of anonymous namespace

//---------------------------------------------------------------------------
int vtkMRMLTransformableNodeOnNodeReferenceAddTest(int , char * [] )
{
  vtkNew<vtkMRMLScene> scene;
  if (populateScene(scene.GetPointer())!=EXIT_SUCCESS)
    {
    std::cerr << "Failed to populate scene." << std::endl;
    return EXIT_FAILURE;
    }

  // Test with a freshly created scene
  if (testScene(scene.GetPointer())!=EXIT_SUCCESS)
    {
    std::cerr << "vtkMRMLTransformableNode events failed on original scene." << std::endl;
    return EXIT_FAILURE;
    }


  scene->SetSaveToXMLString(1);
  scene->Commit();
  std::string xmlScene = scene->GetSceneXMLString();
  std::cout << xmlScene << std::endl;

  // Test with a loaded scene
  vtkNew<vtkMRMLScene> scene2;

  scene2->SetLoadFromXMLString(1);
  scene2->SetSceneXMLString(xmlScene);
  scene2->Import();

  if (testScene(scene2.GetPointer())!=EXIT_SUCCESS)
    {
    std::cerr << "vtkMRMLTransformableNode events failed on saved scene." << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

namespace
{

//---------------------------------------------------------------------------
int populateScene(vtkMRMLScene* scene)
{
  vtkNew<vtkMRMLScalarVolumeNode> transformableableNode;
  scene->AddNode(transformableableNode.GetPointer());

  vtkNew<vtkMRMLLinearTransformNode> transformNode;
  scene->AddNode(transformNode.GetPointer());

  vtkNew<vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback> callback;
  transformNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  transformableableNode->SetAndObserveTransformNodeID(transformNode->GetID());
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfEvents(vtkMRMLTransformNode::TransformReferenceModifiedEvent) != 1)
    {
    std::cerr << "vtkMRMLTransformNode::TransformReferenceModifiedEvent failed."
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of TransformReferenceAddedEvent: "
              << callback->GetNumberOfEvents(vtkMRMLTransformNode::TransformReferenceModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}

int testScene(vtkMRMLScene* scene)
{
  // Check transform node IDs
  vtkMRMLNode* transformNode =
    scene->GetNthNodeByClass(0, "vtkMRMLLinearTransformNode");
  if (!transformNode || strcmp(transformNode->GetID(), "vtkMRMLLinearTransformNode1") != 0)
    {
    std::cerr << __LINE__ << ": import failed." << std::endl
              << "    transform node ID is " << transformNode->GetID()
              << " instead of vtkMRMLLinearTransformNode1." << std::endl;

    return EXIT_FAILURE;
    }

  // Check references
  vtkMRMLTransformableNode* trnsformableNode =
    vtkMRMLTransformableNode::SafeDownCast(scene->GetNthNodeByClass(0, "vtkMRMLTransformableNode"));

  if (strcmp(trnsformableNode->GetTransformNodeID(),
             transformNode->GetID()) != 0)
    {
    std::cerr << __LINE__ << ": import failed." << std::endl
              << "    Transformable node references are not updated. "
              << "Transform node ID reference is "
              << trnsformableNode->GetTransformNodeID()
              << " instead of " << transformNode->GetID() << std::endl;
    return EXIT_FAILURE;
    }

  // Test vtkMRMLTransformableNode::TransformModifiedEvent()
  vtkMRMLLinearTransformNode* linearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->GetNthNodeByClass(0, "vtkMRMLLinearTransformNode"));
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNthNodeByClass(0, "vtkMRMLScalarVolumeNode"));
  vtkNew<vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback> callback;
  volumeNode->AddObserver(vtkCommand::AnyEvent, callback.GetPointer());

  linearTransformNode->GetTransformToParent()->Modified();
  if (!callback->GetErrorString().empty() ||
      callback->GetNumberOfModified() != 0 ||
      callback->GetNumberOfEvents(vtkMRMLTransformableNode::TransformModifiedEvent) != 1)
    {
    std::cerr << "vtkMRMLTransformableNode::TransformModifiedEvent failed."
              << callback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << callback->GetNumberOfModified() << " "
              << "Number of TransformModifiedEvent: "
              << callback->GetNumberOfEvents(vtkMRMLTransformableNode::TransformModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  callback->ResetNumberOfEvents();

  // Test TransformReferenceRemovedEvent
  vtkNew<vtkMRMLCoreTestingUtilities::vtkMRMLNodeCallback> transformCallback;
  linearTransformNode->AddObserver(vtkCommand::AnyEvent, transformCallback.GetPointer());

  volumeNode->SetAndObserveTransformNodeID(NULL);
  if (!transformCallback->GetErrorString().empty() ||
      transformCallback->GetNumberOfModified() != 0 ||
      transformCallback->GetNumberOfEvents(vtkMRMLTransformNode::TransformReferenceModifiedEvent) != 1)
    {
    std::cerr << "vtkMRMLTransformNode::vtkMRMLTransformNode failed."
              << transformCallback->GetErrorString().c_str() << " "
              << "Number of ModifiedEvent: " << transformCallback->GetNumberOfModified() << " "
              << "Number of TransformReferenceModifiedEvent: "
              << transformCallback->GetNumberOfEvents(vtkMRMLTransformNode::TransformReferenceModifiedEvent)
              << std::endl;
    return EXIT_FAILURE;
    }
  transformCallback->ResetNumberOfEvents();

  // Reset -it for next scene load
  volumeNode->SetAndObserveTransformNodeID(linearTransformNode->GetID());

  return EXIT_SUCCESS;
}

}
