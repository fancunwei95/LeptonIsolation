cmake_minimum_required( VERSION 3.2 FATAL_ERROR )

# get analysis base
find_package( AnalysisBase 21.2 REQUIRED )
#find_package(AthDerivation REQUIRED) 

# set up a work directory project:
atlas_project( WorkDir 1.0 USE AnalysisBase ${AnalysisBase_VERSION} )

# set the name of the package:
atlas_subdir( RNNSampleMaker )

## packages that this package depends on
#atlas_depends_on_subdirs(
  #PRIVATE
  ##Control/xAODRootAccess
  ##Event/xAOD/xAODTrack
  ##Event/xAOD/xAODTruth
  ##Event/xAOD/xAODEgamma
  ##Event/xAOD/xAODMuon
  ##PhysicsAnalysis/AnalysisCommon/HDF5Utils
  ##InnerDetector/InDetRecTools/InDetTrackSelectionTool
#)


# external(s) used by the package:
find_package(ROOT REQUIRED COMPONENTS RIO Hist Tree Net Core)
find_package(HDF5 1.10.1 REQUIRED COMPONENTS CXX C)
#find_package(Calorimeter REQUIRED PATHS "/cvmfs/atlas.cern.ch/repo/sw/software/21.2/AthDerivation/21.2.97.0/InstallArea/x86_64-slc6-gcc62-opt/src/Calorimeter" ) 
find_package(lwtnn)


# common requirements
set(_common
  src/ObjectFilters.cxx
  src/ObjectWriters.cxx
  src/TrackWriter.cxx
  src/ElectronWriter.cxx
  src/MuonWriter.cxx
  Event/FourMomUtils
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} ${LWTNN_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} ${HDF5_LIBRARIES} ${LWTNN_LIBRARIES}
  xAODRootAccess
  xAODTracking
  xAODTruth
  xAODEgamma
  xAODMuon
  HDF5Utils
  MuonSelectorToolsLib
  ElectronPhotonSelectorToolsLib
  InDetTrackSelectionToolLib
  xAODBTaggingEfficiencyLib
  xAODJet
  xAODEventShape
  IsolationSelectionLib
)

atlas_depends_on_subdirs(
	Event/FourMomUtils
) 
# External dependencies:
find_package( Boost )

atlas_add_library(
	PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} FourMomUtils
)
# build the test executable:
atlas_add_executable( SampleMaker src/SampleMaker.cxx ${_common})

# Set up grid magic. Atlas uses CPack to package up the local files
# and submit them to the grid. If you're not looking to run on the
# grid this is unneeded.
atlas_cpack_setup()

