include_directories(${CMAKE_SOURCE_DIR}/src ${VLE_INCLUDE_DIRS}
  ${VLE_DEPS_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${CELLDEVS_INCLUDE_DIRS} 
  ${CELLQSS_INCLUDE_DIRS}
  ${DECISION_INCLUDE_DIRS} ${DIFFERENCE_EQU_INCLUDE_DIRS}
  ${DIFFERENTIAL_EQU_INCLUDE_DIRS} ${DSDEVS_INCLUDE_DIRS} ${FSA_INCLUDE_DIRS}
  ${PETRINET_INCLUDE_DIRS})

link_directories(${VLE_LIBRARY_DIRS}
  ${VLE_DEPS_LIBRARY_DIRS} ${CELLDEVS_LIBRARY_DIRS}
  ${CELLQSS_LIBRARY_DIRS} ${DECISION_LIBRARY_DIRS}
  ${DIFFERENCE_EQU_LIBRARY_DIRS} ${DIFFERENTIAL_EQU_LIBRARY_DIRS}
  ${DSDEVS_LIBRARY_DIRS} ${FSA_LIBRARY_DIRS} ${PETRINET_LIBRARY_DIRS})
  
DeclareDifferenceEquationDynamics(passive "passive.cpp")
DeclareDifferenceEquationDynamics(CropPhenology "CropPhenology.cpp")
DeclareDifferenceEquationDynamics(CropClimate "CropClimate.cpp")
DeclareDifferenceEquationDynamics(CropLeafArea "CropLeafArea.cpp")
DeclareDifferenceEquationDynamics(Unit "Unit.cpp")
DeclareFsaDifferenceEquationDynamics(Initiation "Initiation.cpp")
DeclareFsaDifferenceEquationDynamics(PilotGraph "PilotGraph.cpp")
DeclareFsaDifferenceEquationDynamics(PilotFSA "PilotFSA.cpp")
DeclareDsDevsDynamics(ExecutiveFSA "ExecutiveFSA.cpp;GraphTranslator.hpp;GraphTranslator.cpp")
DeclareDsDevsDynamics(ExecutiveGraph "ExecutiveGraph.cpp;GraphTranslator.hpp;GraphTranslator.cpp")
DeclareDsDevsDynamics(Connector "Connector.cpp")
DeclareFsaDifferenceEquationDynamics(CropState "CropState.cpp")

