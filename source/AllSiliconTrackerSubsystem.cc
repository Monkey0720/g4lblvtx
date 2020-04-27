//____________________________________________________________________________..
//
// This is the interface to the framework. You only need to define the parameters
// you use for your detector in the SetDefaultParameters() method here
// The place to do this is marked by //implement your own here//
// The parameters have no units, they need to be converted in the
// AllSiliconTrackerDetector::ConstructMe() method
// but the convention is as mentioned cm and deg
//____________________________________________________________________________..
//
#include "AllSiliconTrackerSubsystem.h"

#include "AllSiliconTrackerDetector.h"
#include "AllSiliconTrackerDisplayAction.h"
#include "AllSiliconTrackerSteppingAction.h"

#include <phparameter/PHParameters.h>

#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4SteppingAction.h>

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNode.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHObject.h>
#include <phool/getClass.h>

#include <TSystem.h>

using namespace std;

//_______________________________________________________________________
AllSiliconTrackerSubsystem::AllSiliconTrackerSubsystem(const std::string &name)
  : PHG4DetectorSubsystem(name)
  , m_Detector(nullptr)
  , m_SteppingAction(nullptr)
  , m_DisplayAction(nullptr)
{
  // call base class method which will set up parameter infrastructure
  // and call our SetDefaultParameters() method
  InitializeParameters();
}

//_______________________________________________________________________
AllSiliconTrackerSubsystem::~AllSiliconTrackerSubsystem()
{
  delete m_DisplayAction;
}

//_______________________________________________________________________
int AllSiliconTrackerSubsystem::InitRunSubsystem(PHCompositeNode *topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
  // create display settings before detector
  m_DisplayAction = new AllSiliconTrackerDisplayAction(Name());

  PHNodeIterator dstIter(dstNode);
  if (GetParams()->get_int_param("active"))
  {
    PHCompositeNode *DetNode = dynamic_cast<PHCompositeNode *>(dstIter.findFirst("PHCompositeNode", Name()));
    if (!DetNode)
    {
      DetNode = new PHCompositeNode(Name());
      dstNode->addNode(DetNode);
    }
    string g4hitnodename = "G4HIT_" + Name();
    PHG4HitContainer *g4_hits = findNode::getClass<PHG4HitContainer>(DetNode, g4hitnodename);
    if (!g4_hits)
    {
      g4_hits = new PHG4HitContainer(g4hitnodename);
      DetNode->addNode(new PHIODataNode<PHObject>(g4_hits, g4hitnodename, "PHObject"));
    }
  }
  // create detector
  m_Detector = new AllSiliconTrackerDetector(this, topNode, GetParams(), Name());
  m_Detector->OverlapCheck(CheckOverlap());
  // create stepping action if detector is active
  if (GetParams()->get_int_param("active"))
  {
    m_SteppingAction = new AllSiliconTrackerSteppingAction(m_Detector, GetParams());
  }
  return 0;
}
//_______________________________________________________________________
int AllSiliconTrackerSubsystem::process_event(PHCompositeNode *topNode)
{
  // pass top node to stepping action so that it gets
  // relevant nodes needed internally
  if (m_SteppingAction)
  {
    m_SteppingAction->SetInterfacePointers(topNode);
  }
  return 0;
}
//_______________________________________________________________________
void AllSiliconTrackerSubsystem::Print(const string &what) const
{
  if (m_Detector)
  {
    m_Detector->Print(what);
  }
  return;
}

//_______________________________________________________________________
PHG4Detector *AllSiliconTrackerSubsystem::GetDetector(void) const
{
  return m_Detector;
}

//_______________________________________________________________________
void AllSiliconTrackerSubsystem::AddAssemblyVolume(const std::string &avol)
{
  if (m_LogVolName.empty())
  {
    m_AssemblyVolumeSet.insert(avol);
  }
  else
  {
    cout << "Assembly Volumes and Logical Volumes cannot coexist" << endl;
    cout << "Existing Logical Volume: " << m_LogVolName << endl;
    gSystem->Exit(1);
  }
}

//_______________________________________________________________________
void AllSiliconTrackerSubsystem::UseLogicalVolume(const string &name)
{
  if (m_AssemblyVolumeSet.empty())
  {
    m_LogVolName = name;
  }
  else
  {
    cout << "Assembly Volumes and Logical Volumes cannot coexist" << endl;
    cout << "Assembly Volumes: " << endl;
    for (auto it = m_AssemblyVolumeSet.begin(); it != m_AssemblyVolumeSet.end(); ++it)
    {
      cout << *it << endl;
    }
    gSystem->Exit(1);
  }
}

//_______________________________________________________________________
void AllSiliconTrackerSubsystem::SetDefaultParameters()
{
  // sizes are in cm
  // angles are in deg
  // units should be converted to G4 units when used
  //implement your own here//
  set_default_double_param("place_x", 0.);
  set_default_double_param("place_y", 0.);
  set_default_double_param("place_z", 0.);
  set_default_double_param("rot_x", 0.);
  set_default_double_param("rot_y", 0.);
  set_default_double_param("rot_z", 0.);

  set_default_string_param("GDMPath", "DefaultParameters-InvalidPath");
}
