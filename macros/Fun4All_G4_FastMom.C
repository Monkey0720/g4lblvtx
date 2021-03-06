#pragma once
#include <phgenfit/Track.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllNoSyncDstInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>

#include <g4detectors/PHG4DetectorSubsystem.h>
#include <g4detectors/PHG4CylinderSubsystem.h>

#include <g4histos/G4HitNtuple.h>
#include <g4lblvtx/AllSiliconTrackerSubsystem.h>
#include <g4main/PHG4ParticleGenerator.h>
#include <g4main/PHG4ParticleGeneratorBase.h>
#include <g4main/PHG4ParticleGun.h>
#include <g4main/PHG4Reco.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <g4main/PHG4TruthSubsystem.h>
#include <g4trackfastsim/PHG4TrackFastSim.h>
#include <g4trackfastsim/PHG4TrackFastSimEval.h>
#include <phool/recoConsts.h>

#include <g4lblvtx/G4LBLVtxSubsystem.h>
#include <g4lblvtx/SimpleNtuple.h>
#include <g4lblvtx/TrackFastSimEval.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4detectors.so)
R__LOAD_LIBRARY(libg4lblvtx.so)
R__LOAD_LIBRARY(libg4trackfastsim.so)

void Fun4All_G4_FastMom(
			int nEvents = -1,
			const char *outputFile = "out_allSi",
			const char *genpar = "pi-")
{
	// ======================================================================================================
	// Input from the user
	const int particle_gen = 1;	// 1 = particle generator, 2 = particle gun
	const int magnetic_field = 4;	// 1 = uniform 1.5T, 2 = uniform 3.0T, 3 = sPHENIX 1.4T map, 4 = Beast 3.0T map
	// ======================================================================================================
	// Parameters for projections
	string projname1   = "DIRC";		// Cylindrical surface object name
	double projradius1 = 50.;		// [cm] 
	double length1     = 400.;		// [cm]
	// ---
	double thinness    = 0.1;		// black hole thickness, needs to be taken into account for the z positions
	// ---
	string projname2   = "FOR"; 		// Forward plane object name
	double projzpos2   = 130+thinness/2.;	// [cm]
	double projradius2 = 50.;		// [cm]
	// ---
	string projname3   = "BACK";		// Backward plane object name
	double projzpos3   = -(130+thinness/2.);// [cm]
	double projradius3 = 50.;		// [cm]
	// ======================================================================================================
	// Make the Server
	Fun4AllServer *se = Fun4AllServer::instance();
	// If you want to fix the random seed for reproducibility
	// recoConsts *rc = recoConsts::instance();
	// rc->set_IntFlag("RANDOMSEED", 12345);
	// ======================================================================================================
	// Particle Generation
	cout << "Particle that will be generated: " << std::string(genpar) << endl;
	// --------------------------------------------------------------------------------------
	// Particle Generator Setup
	PHG4ParticleGenerator *gen = new PHG4ParticleGenerator();
	gen->set_name(std::string(genpar));	// geantino, pi-, pi+, mu-, mu+, e-., e+, proton, ... (currently passed as an input)
	gen->set_vtx(0,0,0);			// Vertex generation range
	gen->set_mom_range(.00001,50.);		// Momentum generation range in GeV/c
	gen->set_z_range(0.,0.);
	gen->set_eta_range(-4,4);		// Detector coverage corresponds to |η|< 4
	gen->set_phi_range(0.,2.*TMath::Pi());
	// --------------------------------------------------------------------------------------
	// Particle Gun Setup
	PHG4ParticleGun *gun = new PHG4ParticleGun();
	gun->set_name(std::string(genpar));	// geantino, pi-, pi+, mu-, mu+, e-., e+, proton, ...
	gun->set_vtx(0,0,0);
	gun->set_mom(0,1,0);
	// --------------------------------------------------------------------------------------
	if     (particle_gen==1){se->registerSubsystem(gen); cout << "Using particle generator" << endl;}
	else if(particle_gen==2){se->registerSubsystem(gun); cout << "Using particle gun"       << endl;}

	// ======================================================================================================
	PHG4Reco *g4Reco = new PHG4Reco();
	// ======================================================================================================
	// Magnetic field setting
	TString B_label;
	if(magnetic_field==1){		// uniform 1.5T
		B_label = "_B_1.5T";
		g4Reco->set_field(1.5);
	}
	else if(magnetic_field==2){	// uniform 3.0T
		B_label = "_B_3.0T";
		g4Reco->set_field(3.0);
	}
	else if(magnetic_field==3){	// sPHENIX 1.4T map
		B_label = "_sPHENIX";
		g4Reco->set_field_map(string(getenv("CALIBRATIONROOT")) + string("/Field/Map/sPHENIX.2d.root"), PHFieldConfig::kField2D);
		g4Reco->set_field_rescale(-1.4/1.5);
	}
	else if(magnetic_field==4){	// Beast 3.0T map
		B_label = "_Beast";
		g4Reco->set_field_map(string(getenv("CALIBRATIONROOT")) + string("/Field/Map/mfield.4col.dat"), PHFieldConfig::kFieldBeast);
	}
	else{				// The user did not provide a valid B field setting
		cout << "User did not provide a valid magnetic field setting. Set 'magnetic_field'. Bailing out!" << endl;
	}
	// ======================================================================================================
	// Physics list (default list is "QGSP_BERT")
	//g4Reco->SetPhysicsList("FTFP_BERT_HP"); // This list is slower and only useful for hadronic showers.
	// ======================================================================================================
	// Loading All-Si Tracker from dgml file
	AllSiliconTrackerSubsystem *allsili = new AllSiliconTrackerSubsystem();
	//allsili->set_string_param("GDMPath", string(getenv("CALIBRATIONROOT")) + "/AllSiliconTracker/FAIRGeom.gdml");
	allsili->set_string_param("GDMPath","FAIRGeom.gdml"); 

	allsili->AddAssemblyVolume("VST");	// Barrel
	allsili->AddAssemblyVolume("FST");	// Forward disks
	allsili->AddAssemblyVolume("BST");	// Backward disks
	allsili->AddAssemblyVolume("BEAMPIPE");	// Beampipe

	// this is for plotting single logical volumes for debugging and geantino scanning they end up at the center, you can plot multiple
	// but they end up on top of each other. They cannot coexist with the assembly volumes, the code will quit if you try to use both.
	// allsili->AddLogicalVolume("BstContainerVolume04");
	// allsili->AddLogicalVolume("FstContainerVolume00");
	// allsili->AddLogicalVolume("FstChipAssembly37");
	// allsili->AddLogicalVolume("VstStave00");

	allsili->SuperDetector("LBLVTX");
	allsili->SetActive();          // this saves hits in the MimosaCore volumes
	allsili->SetAbsorberActive();  // this saves hits in all volumes (in the absorber node)
	g4Reco->registerSubsystem(allsili);

	// ======================================================================================================	
	PHG4CylinderSubsystem *cyl;
	cyl = new PHG4CylinderSubsystem(projname1,0);
	cyl->set_double_param("length", length1);
	cyl->set_double_param("radius", projradius1); // dirc radius
	cyl->set_double_param("thickness", 0.1); // needs some thickness
	cyl->set_string_param("material", "G4_AIR");
	cyl->SetActive(1);
	cyl->SuperDetector(projname1);
	cyl->BlackHole();
	cyl->set_color(1,0,0,0.7); //reddish
	g4Reco->registerSubsystem(cyl);

	cyl = new PHG4CylinderSubsystem(projname2,0);
	cyl->set_double_param("length", thinness);
	cyl->set_double_param("radius", 2); // beampipe needs to fit here
	cyl->set_double_param("thickness", projradius2); // 
	cyl->set_string_param("material", "G4_AIR");
	cyl->set_double_param("place_z", projzpos2);
	cyl->SetActive(1);
	cyl->SuperDetector(projname2);
	cyl->BlackHole();
	cyl->set_color(0,1,1,0.3); //reddish
	g4Reco->registerSubsystem(cyl);

	cyl = new PHG4CylinderSubsystem(projname3,0);
	cyl->set_double_param("length", thinness);
	cyl->set_double_param("radius", 2); // beampipe needs to fit here
	cyl->set_double_param("thickness", projradius3); // 
	cyl->set_string_param("material", "G4_AIR");
	cyl->set_double_param("place_z", projzpos3);
	cyl->SetActive(1);
	cyl->SuperDetector(projname3);
	cyl->BlackHole();
	cyl->set_color(0,1,1,0.3); //reddish
	g4Reco->registerSubsystem(cyl);
	// ======================================================================================================

	PHG4TruthSubsystem *truth = new PHG4TruthSubsystem();
	g4Reco->registerSubsystem(truth);

	se->registerSubsystem(g4Reco);

	// ======================================================================================================
	// Fast pattern recognition and full Kalman filter
	// output evaluation file for truth track and reco tracks are PHG4TruthInfoContainer
	char nodename[100];
	PHG4TrackFastSim *kalman = new PHG4TrackFastSim("PHG4TrackFastSim");
	kalman->set_use_vertex_in_fitting(false);
	kalman->set_sub_top_node_name("SVTX");
	kalman->set_trackmap_out_name("SvtxTrackMap");

	for (int i=10; i<16; i++){ // CENTRAL BARREL
		sprintf(nodename,"G4HIT_LBLVTX_CENTRAL_%d", i);
		kalman->add_phg4hits(
				nodename,				// const std::string& phg4hitsNames
				PHG4TrackFastSim::Cylinder,		// const DETECTOR_TYPE phg4dettype
				999.,					// radial-resolution [cm] (this number is not used in cylindrical geometry)
				5.8e-4,					// azimuthal (arc-length) resolution [cm]
				5.8e-4,					// longitudinal (z) resolution [cm]
				1,					// efficiency (fraction)
				0					// hit noise
				);
	}
	for (int i=20; i<25; i++){ // FORWARD DISKS
		sprintf(nodename,"G4HIT_LBLVTX_FORWARD_%d", i);
		kalman->add_phg4hits(
				nodename,                          	// const std::string& phg4hitsNames
				PHG4TrackFastSim::Vertical_Plane,  	// const DETECTOR_TYPE phg4dettype
				5.8e-4,                            	// radial-resolution [cm]
				5.8e-4,                            	// azimuthal (arc-length) resolution [cm]
				999.,                              	// longitudinal (z) resolution [cm] (this number is not used in vertical plane geometry)
				1,                                 	// efficiency (fraction)
				0                                  	// hit noise
				);
	}
	for (int i=30; i<35; i++){ // BACKWARD DISKS
		sprintf(nodename,"G4HIT_LBLVTX_BACKWARD_%d", i);
		kalman->add_phg4hits(
				nodename,                          	// const std::string& phg4hitsNames
				PHG4TrackFastSim::Vertical_Plane,  	// const DETECTOR_TYPE phg4dettype
				5.8e-4,                            	// radial-resolution [cm]
				5.8e-4,                            	// azimuthal (arc-length) resolution [cm]
				999.,                              	// longitudinal (z) resolution [cm] (this number is not used in vertical plane geometry)
				1,                                 	// efficiency (fraction)
				0                                  	// hit noise
				);
	}
	// Projections	
	kalman->add_cylinder_state(projname1, projradius1);	// projection on cylinder (DIRC)
	kalman->add_zplane_state  (projname2, projzpos2  );	// projection on vertical planes
	kalman->add_zplane_state  (projname3, projzpos3  );	// projection on vertical planes
	
	se->registerSubsystem(kalman);
	// -----------------------------------------------------
	// INFO: The resolution numbers above correspond to:
	// 20e-4/sqrt(12) cm = 5.8e-4 cm, to simulate 20x20 um

	// ======================================================================================================
	TrackFastSimEval *fast_sim_eval = new TrackFastSimEval("FastTrackingEval");
	fast_sim_eval->set_filename(TString(outputFile)+B_label+"_FastTrackingEval.root");
	fast_sim_eval->AddProjection(projname1);
	fast_sim_eval->AddProjection(projname2);
	fast_sim_eval->AddProjection(projname3);
	se->registerSubsystem(fast_sim_eval);

	// ======================================================================================================
	SimpleNtuple *hits = new SimpleNtuple("Hits");
	//  hits->AddNode("ABSORBER_LBLVTX",0); // hits in the passive volumes
	for (int i = 10; i < 16; i++){sprintf(nodename, "LBLVTX_CENTRAL_%d", i);	hits->AddNode(nodename, i);} // hits in the  MimosaCore volumes
	for (int i = 20; i < 25; i++){sprintf(nodename, "LBLVTX_FORWARD_%d", i);	hits->AddNode(nodename, i);} // hits in the  MimosaCore volumes
	for (int i = 30; i < 35; i++){sprintf(nodename, "LBLVTX_BACKWARD_%d",i);	hits->AddNode(nodename, i);} // hits in the  MimosaCore volumes
	se->registerSubsystem(hits);

	///////////////////////////////////////////
	// IOManagers...
	///////////////////////////////////////////
	const std::string dst_name = std::string(outputFile)+"_G4LBLVtx.root";
	//Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT",TString(outputFile)+"_G4LBLVtx.root");
	Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT",dst_name);
	out->Verbosity(10);
	se->registerOutputManager(out);

	Fun4AllInputManager *in = new Fun4AllDummyInputManager("JADE");
	se->registerInputManager(in);
	if (nEvents <= 0)
	{
		return;
	}
	se->run(nEvents);
	se->End();
	delete se;
	gSystem->Exit(0);
}
