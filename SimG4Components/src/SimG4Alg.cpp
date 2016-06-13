#include "SimG4Alg.h"

// FCCSW
#include "SimG4Interface/ISimG4Svc.h"

// Geant
#include "G4Event.hh"

DECLARE_ALGORITHM_FACTORY(SimG4Alg)

SimG4Alg::SimG4Alg(const std::string& aName, ISvcLocator* aSvcLoc):
  GaudiAlgorithm(aName, aSvcLoc) {
  declareProperty("outputs",m_saveToolNames);
  declarePrivateTool(m_eventGenTool, "SimG4PrimariesFromEdmTool", true);
  declareProperty("eventGenerator", m_eventGenTool);
}
SimG4Alg::~SimG4Alg() {}

StatusCode SimG4Alg::initialize() {
  if (GaudiAlgorithm::initialize().isFailure())
    return StatusCode::FAILURE;
  m_geantSvc = service("SimG4Svc");
  if (! m_geantSvc) {
    error() << "Unable to locate Geant Simulation Service" << endmsg;
    return StatusCode::FAILURE;
  }
  for(auto& toolname: m_saveToolNames) {
    m_saveTools.push_back(tool<ISimG4SaveOutputTool>(toolname));
    // FIXME: check StatusCode once the m_saveTools is a ToolHandleArray
    // if (!) {
    //   error() << "Unable to retrieve the output saving tool." << endmsg;
    //   return StatusCode::FAILURE;
    // }
  }
  if (!m_eventGenTool.retrieve()) {
    error()<<"Unable to retrieve the G4Event generator " << m_eventGenTool <<endmsg;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode SimG4Alg::execute() {
  // first translate the event
  G4Event* event = m_eventGenTool->g4Event();

  if ( !event ) {
    error() << "Unable to retrieve G4Event from " << m_eventGenTool << endmsg;
    return StatusCode::FAILURE;
  }
  m_geantSvc->processEvent(*event);
  G4Event* constevent;
  m_geantSvc->retrieveEvent(constevent);
  for(auto& tool: m_saveTools) {
    tool->saveOutput(*constevent);
  }
  m_geantSvc->terminateEvent();
  return StatusCode::SUCCESS;
}

StatusCode SimG4Alg::finalize() {
  return GaudiAlgorithm::finalize();
}