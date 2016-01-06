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

    self._CurrentRunningStep = 0
    self._waitForCompletion = False

  def addStep(self, cliModule, params=[]):
    node = CLIPipelineNode()
    node.CLIModule = cliModule
    node.CLI = slicer.cli.createNode(cliModule)
    node.Parameters = params
    self.Nodes.append(node)

  def step(self, step):
    return self.Nodes[step]

  def run(self, wait_for_completion=False):
    if self.CLI.IsBusy():
      print('Cannot start pipeline, it is already running !')
      return

    if len(self.Nodes) < 1:
      print('Cannot run pipeline, its has no nodes !') 
      self.CLI.SetStatus(slicer.vtkMRMLCommandLineModuleNode.Completed)
      return

    self._waitForCompletion = wait_for_completion
    self.addObserver(self.Nodes[0].CLI, self.StatusModifiedEvent, self.onFirstNodeModified)
    self.addObserver(self.Nodes[-1].CLI, self.StatusModifiedEvent, self.onLastNodeModified)

    self._CurrentStepRunning = 0
    self.runNode(self._CurrentStepRunning)

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
        self.removeObserver(cliNode, self.StatusModifiedEvent, self.onLastNodeModified)

  def onNthNodeModified(self, cliNode, event):
    status = cliNode.GetStatus()
    if (status == cliNode.Cancelling or status == cliNode.Cancelled
        or status == cliNode.CompletedWithErrors):
      self.CLI.SetStatus(status)

    if status == cliNode.Cancelled or status == cliNode.CompletedWithErrors:
      self.removeObserver(cliNode, self.StatusModifiedEvent, self.onNthNodeModified)
      self.removeObserver(self.Node[-1].CLI, self.StatusModifiedEvent, self.onLastNodeModified)
    elif status == cliNode.Completed:
      self.removeObserver(cliNode, self.StatusModifiedEvent, self.onNthNodeModified)

      if self._CurrentRunningStep < len(self.Nodes) - 1:
        self._CurrentRunningStep = self._CurrentRunningStep + 1
        self.runNode(self._CurrentRunningStep)

  def runNode(self, step):
    print('Running node #%s' %step)
    pipelineNode = self.Nodes[step]
    self.addObserver(pipelineNode.CLI, self.StatusModifiedEvent, self.onNthNodeModified)
    cliNode = slicer.cli.run(pipelineNode.CLIModule, pipelineNode.CLI, pipelineNode.Parameters, self._waitForCompletion)

  def setStepParameters(self, step, params):
    pipelineNode = self.Nodes[step]
    pipelineNode.Parameters = params

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

