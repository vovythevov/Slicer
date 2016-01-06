import vtk, qt, ctk, slicer
from slicer import util
from slicer.util import VTKObservationMixin
from slicer import testing

#
# CLIPipelineNode
#

class CLIPipelineNode:

  def __init__(self):
    self.CLI = None
    self.CLIModule = None
    self.Parameters = {}

#
# CLIPipeline
#

class CLIPipeline(VTKObservationMixin):
  def __init__(self):
    VTKObservationMixin.__init__(self)

    self.Nodes = []
    self.CLI = slicer.vtkMRMLCommandLineModuleNode()
    self.StatusModifiedEvent = slicer.vtkMRMLCommandLineModuleNode().StatusModifiedEvent
    self.OnNodeStatusChangedCallback = None

    self._CurrentRunningStep = 0
    self._waitForCompletion = False
    self._intermediateNodes= {}
    self._intermediateNodeConnections = {}

  def addStep(self, cliModule, params=[]):
    node = CLIPipelineNode()
    node.CLIModule = cliModule
    node.CLI = slicer.cli.createNode(cliModule)
    node.Parameters = params
    self.Nodes.append(node)

  def node(self, step):
    try:
      return self.Nodes[step]
    except IndexError:
      print('Step #%s not found !' %step)

  def run(self, wait_for_completion=False):
    if self.CLI.IsBusy():
      print('Cannot start pipeline, it is already running !')
      return

    if len(self.Nodes) < 1:
      print('Cannot run pipeline, its has no nodes !') 
      self.CLI.SetStatus(slicer.vtkMRMLCommandLineModuleNode.Completed)
      return

    self._waitForCompletion = wait_for_completion
    self._intermediateNodeConnections = {}
    self.addObserver(self.Nodes[0].CLI, self.StatusModifiedEvent, self.onFirstNodeModified)
    self.addObserver(self.Nodes[-1].CLI, self.StatusModifiedEvent, self.onLastNodeModified)

    self._CurrentStepRunning = 0
    self.runNode(self._CurrentStepRunning)

  def _onPipelineEnd(self, lastExecutingNode):
    self.removeObserver(lastExecutingNode, self.StatusModifiedEvent, self.onNthNodeModified)
    self.removeObserver(self.Nodes[-1].CLI, self.StatusModifiedEvent, self.onLastNodeModified)
    self._removeAllIntermediateNodes()

  def onFirstNodeModified(self, cliNode, event):
    print('FIRST node modified - %s' %cliNode.GetStatusString())
    status = cliNode.GetStatus()
    if status == cliNode.Scheduled or status == cliNode.Running:
      self.CLI.SetStatus(status)
      if status == cliNode.Running:
        self.removeObserver(cliNode, self.StatusModifiedEvent, self.onFirstNodeModified)

  def onLastNodeModified(self, cliNode, event):
    print('LAST node modified - %s' %cliNode.GetStatusString())
    status = cliNode.GetStatus()
    if status == cliNode.Completing or status == cliNode.Completed:
      self.CLI.SetStatus(status)
      if status == cliNode.Completed:
        self._onPipelineEnd(cliNode)

  def onNthNodeModified(self, cliNode, event):
    status = cliNode.GetStatus()
    if self.OnNodeStatusChangedCallback:
      self.OnNodeStatusChangedCallback(self._CurrentRunningStep, status)

    if (status == cliNode.Cancelling or status == cliNode.Cancelled
        or status == cliNode.CompletedWithErrors):
      self.CLI.SetStatus(status)

    if status == cliNode.Cancelled or status == cliNode.CompletedWithErrors:
      self._PipelineEnd(cliNode)
    elif status == cliNode.Completed:
      self.removeObserver(cliNode, self.StatusModifiedEvent, self.onNthNodeModified)

      if self._CurrentRunningStep < len(self.Nodes) - 1:
        self._CurrentRunningStep = self._CurrentRunningStep + 1
        self.runNode(self._CurrentRunningStep)
        self._deleteIntermediateNodes(self._CurrentRunningStep)

  def runNode(self, step):
    print('Running node #%s' %step)
    pipelineNode = self.node(step)
    self.createIntermediateNodes(step)
    self.addObserver(pipelineNode.CLI, self.StatusModifiedEvent, self.onNthNodeModified)
    cliNode = slicer.cli.run(pipelineNode.CLIModule, pipelineNode.CLI, pipelineNode.Parameters, self._waitForCompletion)

  def setStepParameters(self, step, params):
    pipelineNode = self.Nodes[step]
    pipelineNode.Parameters = params

  def addIntermediateNode(self, stepFrom, paramNameFrom, stepTo, paramNameTo, nodeClass):
    pNodeFrom = self.node(stepFrom)
    pNodeTo = self.node(stepTo)
    if not pNodeFrom or not pNodeTo:
      return None

    if (not paramNameFrom in pNodeFrom.Parameters
        or not paramNameTo in pNodeTo.Parameters):
      return None

    key = (stepFrom, paramNameFrom)
    self._intermediateNodes[key] = nodeClass

    if not self._intermediateNodeConnections.has_key(key):
      self._intermediateNodeConnections[key] = []
    self._intermediateNodeConnections[key].append( (stepTo, paramNameTo) )

  def createIntermediateNodes(self, step):
    pipelineNode = self.node(step)

    for key in pipelineNode.Parameters:
      connection = (step, key)
      if connection in self._intermediateNodes: # Check if we need to create the node
        intermediateNode = slicer.mrmlScene.AddNode( self._intermediateNodes[connection]() )
        intermediateNode.HideFromEditorsOn()
        self._intermediateNodes[connection] = intermediateNode

        for (s, p) in self._intermediateNodeConnections.iteritems():
          n = self.node(s)
          n.Parameters[p] = intermediateNode

  def _deleteIntermediateNodes(self, runningStep):
    for (step, param) in self._intermediateNodeConnections:
      canDelete = True
      for (s, p) in self._intermediateNodeConnections[step, param]:
        if s < runningStep:
          canDelete = False

      if canDelete:
        intermediateNode = self._intermediateNodes[(step, param)]
        if type(intermediateNode) is not type(slicer.vtkMRMLNode):
          slicer.mrmlScene.RemoveNode(intermediateNode)

  def _removeAllIntermediateNodes(self):
    for key, value in self._intermediateNodes.iteritems():
      if type(value) is not type(slicer.vtkMRMLNode):
        slicer.mrmlScene.RemoveNode(value)

#
# CLIPipelineModule
#

class CLIPipelineModule:
  def __init__(self, parent):
    parent.title = "CLI Pipeline"
    parent.categories = ["Utilities"]
    parent.contributors = ["Johan Andruejol (Kitware Inc.)"]
    parent.helpText = """
    Module to help the piping of CLIs.
    """
    parent.acknowledgementText = """\todo"""
    self.parent = parent

#
# qCLIPipelineModuleWidget
#

class CLIPipelineModuleWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    self.Tests = (
       ( 'Simple test - wait_for_completion', self.simpleTestWaitForCompletion ),
       ( 'Simple test - async', self.simpleTestAsynchronous ),
       ( 'Two CLIs test - wait_for_completion', self.twoCLIsTestWaitForCompletion ),
       ( 'Two CLIs test - async', self.twoCLIsTestAsynchronous ),
       ( 'Two CLIs piped together test - wait_for_completion', self.twoCLIsPipedTogetherTestWaitForCompletion ),
       ( 'Two CLIs piped together test - async', self.twoCLIsPipedTogetherTestAsynchronous ),
      )

    for test in self.Tests:
      b = qt.QPushButton(test[0])
      self.layout.addWidget(b)
      b.connect('clicked()', test[1])

    b = qt.QPushButton('Run all tests')
    self.layout.addWidget(b)
    b.connect('clicked()', self.runAllTests)

    self.log = qt.QTextEdit()
    self.log.readOnly = True
    self.layout.addWidget(self.log)
    self.log.insertHtml('<p>Status: <i>Idle</i>\n')
    self.log.insertPlainText('\n')
    self.log.ensureCursorVisible()

    # Add spacer to layout
    self.layout.addStretch(1)

  def runAllTests(self):
    for test in self.Tests:
      test[1]()

  def loadMRHead(self):
    mrHead = slicer.util.getFirstNodeByClassByName('vtkMRMLScalarVolumeNode', 'MRHead')
    if not mrHead:
      # Load sample data
      import SampleData
      sampleDataLogic = SampleData.SampleDataLogic()
      sampleDataLogic.downloadSample('MRHead')
      mrHead = slicer.util.getFirstNodeByClassByName('vtkMRMLScalarVolumeNode', 'MRHead')

    if not mrHead:
      slicer.testing.exitFailure('No MRHead found !')
    return mrHead

  # -- Simple test --
  # Just run one CLI
  def simpleTestWaitForCompletion(self):
    self.simpleTest(True)

  def simpleTestAsynchronous(self):
    self.simpleTest(False)

  def simpleTest(self, wait_for_completion):
    mrHead = self.loadMRHead()
    
    pipeline = CLIPipeline()
    pipeline.addStep(slicer.modules.thresholdscalarvolume)

    # Output
    outputNode = slicer.vtkMRMLScalarVolumeNode()
    slicer.mrmlScene.AddNode(outputNode)

    firstCLIParams = {}
    firstCLIParams["InputVolume"] = mrHead.GetID()
    firstCLIParams["OutputVolume"] = outputNode.GetID()
    firstCLIParams["ThresholdValue"] = 100
    pipeline.setStepParameters(0, firstCLIParams)
    pipeline.run(wait_for_completion)

    if not wait_for_completion:
      while pipeline.CLI.GetStatusString() != 'Completed':
        slicer.util.delayDisplay('Waiting for simple test completion...')

    if not pipeline.CLI.GetStatusString() == 'Completed':
      slicer.testing.exitFailure('Pipeline did not work !')

    slicer.util.delayDisplay('Test succeeded !')

  # -- Two clis test --
  # Run two clis in a row, but they have no relation between each other
  def twoCLIsTestWaitForCompletion(self):
    self.twoCLIsTest(True)

  def twoCLIsTestAsynchronous(self):
    self.twoCLIsTest(False)

  def twoCLIsTest(self, wait_for_completion):
    mrHead = self.loadMRHead()

    pipeline = CLIPipeline()
    pipeline.addStep(slicer.modules.thresholdscalarvolume)
    pipeline.addStep(slicer.modules.thresholdscalarvolume)

    # Output
    outputNode = slicer.vtkMRMLScalarVolumeNode()
    slicer.mrmlScene.AddNode(outputNode)

    firstCLIParams = {}
    firstCLIParams["InputVolume"] = mrHead.GetID()
    firstCLIParams["OutputVolume"] = outputNode.GetID()
    firstCLIParams["ThresholdValue"] = 100
    pipeline.setStepParameters(0, firstCLIParams)
    pipeline.setStepParameters(1, firstCLIParams)
    
    pipeline.run(wait_for_completion)

    if not wait_for_completion:
      while pipeline.CLI.GetStatusString() != 'Completed':
        slicer.util.delayDisplay('Waiting for simple test completion...')

    if not pipeline.CLI.GetStatusString() == 'Completed':
      slicer.testing.exitFailure('Pipeline did not work !')

    slicer.util.delayDisplay('Test succeeded !')

  # -- Two clis with intermediate node test --
  # Run two clis in a row, and pipe the output of the first to the second
  def twoCLIsPipedTogetherTestWaitForCompletion(self):
    self.twoCLIsPipedTogetherTest(True)

  def twoCLIsPipedTogetherTestAsynchronous(self):
    self.twoCLIsPipedTogetherTest(False)

  def twoCLIsPipedTogetherTest(self, wait_for_completion):
    mrHead = self.loadMRHead()

    pipeline = CLIPipeline()
    pipeline.addStep(slicer.modules.thresholdscalarvolume)
    pipeline.addStep(slicer.modules.thresholdscalarvolume)

    # Output
    outputNode = slicer.vtkMRMLScalarVolumeNode()
    slicer.mrmlScene.AddNode(outputNode)

    # Get number of scalar volume node before running the pipeling
    beforeCollection = slicer.mrmlScene.GetNodesByClass('vtkMRMLScalarVolumeNode')
    beforeCollection.UnRegister(beforeCollection)

    firstCLIParams = {}
    firstCLIParams["InputVolume"] = mrHead.GetID()
    firstCLIParams["ThresholdValue"] = 100
    pipeline.setStepParameters(0, firstCLIParams)

    secondCLIParams = firstCLIParams
    firstCLIParams["OutputVolume"] = outputNode
    firstCLIParams["ThresholdValue"] = 100
    pipeline.setStepParameters(1, secondCLIParams)

    # Handle intermediate parameter
    pipeline.addIntermediateNode(0, 'OutputVolume', 1, 'InputVolume', slicer.vtkMRMLScalarVolumeNode)

    pipeline.run(wait_for_completion)

    if not wait_for_completion:
      while pipeline.CLI.GetStatusString() != 'Completed':
        slicer.util.delayDisplay('Waiting for simple test completion...')

    if not pipeline.CLI.GetStatusString() == 'Completed':
      slicer.testing.exitFailure('Pipeline did not work !')

    # Compare beforeCollection and after collection
    afterCollection = slicer.mrmlScene.GetNodesByClass('vtkMRMLScalarVolumeNode')
    afterCollection.UnRegister(afterCollection)

    if beforeCollection.GetNumberOfItems() != afterCollection.GetNumberOfItems():
      slicer.testing.exitFailure('Pipeline left some volumes around !')

    slicer.util.delayDisplay('Test succeeded !')

