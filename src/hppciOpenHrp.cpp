
/*************************************
INCLUDE
**************************************/
#include <iostream>
#include <string>

#include "hppciOpenHrp.h"

#include "common-modelloader.hh"
#include "jrl-modelloader.hh"

#include "hppOpenHRP/parserOpenHRPKineoHRP2.h"
#include "hppOpenHRP/parserOpenHRPKineoObstacle.h"


/*************************************
DEFINE
**************************************/


  
// ==============================================================================
//
//  INTERNAL CLASS : Internal Corba Object
//
//  Note : this class has been created because "common.hh" and "modelloader.hh"
//  should not appear in hppciOpenHrp.h  
//
// ==============================================================================


class CinternalCorbaObject {
    
public :

  /// Corba elements.
  CORBA::ORB_var orb;
  /// Corba elements.
  CORBA::Object_var nameService;

  // HRP2 Corba Information 
  ModelInfo_var HRP2info;

  // Obstacle Corba Information
  std::vector<ModelInfo_var> obstInfoVector;

  ChppciOpenHrpClient *attOpenHrpClient ;


  // Constructor
  CinternalCorbaObject(ChppciOpenHrpClient *openHrpClientPtr) {
    attOpenHrpClient = openHrpClientPtr ;
  } ; 

  ktStatus getModelLoader();

  ModelLoader_var attLoader;

} ; 


//-------------------------------------------------------------------------------
//
//  
//  METHODS FOR ChppciOpenHrpClient
//
//
//-------------------------------------------------------------------------------
  
// ==============================================================================

ChppciOpenHrpClient::ChppciOpenHrpClient(ChppPlanner *hpp) 
{
  hppPlanner = hpp;
  privateCorbaObject = new CinternalCorbaObject(this) ;
}
  
// ==============================================================================

ChppciOpenHrpClient::~ChppciOpenHrpClient() {
  // nothing to do
}
  
// ==============================================================================

ktStatus ChppciOpenHrpClient::loadHrp2Model(ChppDeviceShPtr &HRP2Device)
{
  // 
  // Get Corba objects containing model of HRP2 and Obstacles.
  //
  if (getRobotURL()!= KD_OK) {
    privateCorbaObject->orb->destroy();
    cerr << "ERROR : ChppciOpenHrpClient::loadHrp2Model::Failed to load Hrp2 model" 
	 << endl;
    return KD_ERROR;
  }

  //
  // Build KineoHRP2 from OpenHRPModel
  //
  CparserOpenHRPKineoHRP2 parserHRP2(privateCorbaObject->HRP2info) ;
  HRP2Device=parserHRP2.parser() ;
  if (!HRP2Device) {
    cerr << " ERROR : ChppciOpenHrpClient::loadHrp2Model : Failed building Kineo HRP2 Model" << endl ;
    return KD_ERROR;
  }
  
  //
  // set HRP2 joints in invisible mode by default
  //
  HRP2Device->isVisible(false) ;

  //
  // set HRP2 in a default configuration (HALFSITTING) ;
  //
  CkwsConfigShPtr halfSittingConfig = CkwsConfig::create(HRP2Device) ;
  double dInitPos[46] = HALFSITTINGPOSITION_RAD_KINEO ;
  std::vector<double>  halfSittingVector (dInitPos  , dInitPos + sizeof(dInitPos) / sizeof(double) );
  halfSittingConfig->setDofValues( halfSittingVector ) ; 
  HRP2Device->applyCurrentConfig(halfSittingConfig) ;

  return KD_OK;
}




ktStatus ChppciOpenHrpClient::loadRobotModel(std::string inFilename, std::string inDeviceName, 
					     ChppDeviceShPtr &outDevice)
{
  ModelLoader_var loader;
  if (privateCorbaObject->getModelLoader() != KD_OK){
    cerr << "ERROR : ChppciOpenHrpClient::loadRobotModel::Failed to load robot model" 
	 << endl;
    return KD_ERROR;
  }
	
  //
  // Build hppDevice from OpenHRPModel
  //
  std::string url("file://");
  url += std::string(OPENHRP_PREFIX);
  url += std::string("/etc/");
  url += inFilename;
    
  std:: cout << "ChppciOpenHrpClient::loadRobotModel: reading " << url << std::endl;

  try{
    CparserOpenHRPKineoDevice parser(privateCorbaObject->attLoader->loadURL(url.c_str())) ;
    outDevice=parser.parser() ;
  } catch (CORBA::SystemException &ex) {
    cerr << "System exception: " << ex._rep_id() << endl;
    return KD_ERROR;
  } catch (...){
    cerr << "Unknown exception" << endl;
    return KD_ERROR;
  }
  if (!outDevice) {
    cerr << " ERROR : ChppciOpenHrpClient::loadRobotModel : Failed building Kineo Robot Model" << endl ;
    return KD_ERROR;
  }
  
  //
  // set joints in invisible mode by default
  //
  outDevice->isVisible(false) ;

  privateCorbaObject->orb->destroy();

  return KD_OK;
}




ktStatus  ChppciOpenHrpClient::loadHrp2Model()
{
  ChppDeviceShPtr HRP2Device;
  if (loadHrp2Model(HRP2Device) != KD_OK){
    return KD_ERROR;
  }
  
  //
  // ADD HRP2 to the planner
  // 
  if (hppPlanner->addHppProblem(HRP2Device) != KD_OK) {
    cerr << "ChppciOpenHrpClient::loadHrp2Model: Failed to add robot" << endl;
    return KD_ERROR;
  }

  return KD_OK;
 
}

// ==============================================================================

ktStatus  ChppciOpenHrpClient::loadObstacleModel(std::string inFilename, std::string inObstacleName, 
						 CkppKCDPolyhedronShPtr& outPolyhedron)
{
  // 
  // Get Corba objects containing model of HRP2 and Obstacles.
  //
  if (getObstacleURL(inFilename)!= KD_OK) {
    privateCorbaObject->orb->destroy();
    cerr << "ERROR : ChppciOpenHrpClient::loadObstacleModel: failed to load Hrp2 model" 
	 << endl;
    return KD_ERROR;
  }

  //
  // build obstacles (if there is some obstacle)
  //
  if (privateCorbaObject->obstInfoVector.size() > 0){  


    //
    // Build KineoObstacle from OpenHRPModel
    //
    CparserOpenHRPKineoObstacle parserObstacle(privateCorbaObject->obstInfoVector);

    std::vector<CkppKCDPolyhedronShPtr> obstacleVector = parserObstacle.parser() ;

    if (obstacleVector.size() == 0) {
      cerr << "ERROR ChppciOpenHrpClient::loadHrp2Model : Failed to parse the obstacle model " <<  endl;
      return KD_ERROR;
    }
    outPolyhedron = obstacleVector[0];
  }

  privateCorbaObject->orb->destroy();

  return KD_OK;
}


  
ktStatus CinternalCorbaObject::getModelLoader()
{
  // Services
  string modelLoaderString="ModelLoaderJRL" ;
  string nameServiceString="NameService" ;

  try {
    int argc=1;
    char *argv[1]={"unused"};
    // ORB Creation.
    orb = CORBA::ORB_init(argc, argv);

    // Getting reference of name server
    try {
      nameService = orb->resolve_initial_references(nameServiceString.c_str());
    } catch (const CORBA::ORB::InvalidName&) {
      cerr << "ERROR :ChppciOpenHrpClient::getModelLoader : cannot resolve " << nameServiceString << endl;
      return KD_ERROR;
    }
    if(CORBA::is_nil(nameService)) {
      cerr << " ERROR :ChppciOpenHrpClient::getModelLoader "<< nameServiceString << "is a nil object reference"
	   << endl;
      return KD_ERROR;
    }

    // Getting root naming context
    CosNaming::NamingContext_var cxt = 
      CosNaming::NamingContext::_narrow(nameService);
    if(CORBA::is_nil(cxt)) {
      cerr << " ERROR :ChppciOpenHrpClient::getModelLoader "<< nameServiceString << " is not a NamingContext object reference"
	   << endl;
      return KD_ERROR;
    }

    CosNaming::Name ncFactory;
    ncFactory.length(1);

    // modelloaderJRL
    CosNaming::Name mFactory;
    mFactory.length(1);
    mFactory[0].id = CORBA::string_dup(modelLoaderString.c_str());
    mFactory[0].kind = CORBA::string_dup("");

    try {
      attLoader = ModelLoader::_narrow(cxt -> resolve(mFactory));
    } catch (const CosNaming::NamingContext::NotFound&){
      cerr << "ERROR :  ChppciOpenHrpClient::getModelLoader : "<< modelLoaderString <<  "NOT FOUND" << endl;
      return KD_ERROR;
    }
  } catch (CORBA::SystemException &ex) {
    cerr << "System exception: " << ex._rep_id() << endl;
    return KD_ERROR;
  } catch (...){
    cerr << "Unknown exception" << endl;
    return KD_ERROR;
  }
  return KD_OK;
}


ktStatus ChppciOpenHrpClient::getRobotURL()
{
  if (privateCorbaObject->getModelLoader() != KD_OK){
      return KD_ERROR;
  }
  
  if(privateCorbaObject->attLoader) {
    //Get HRP2 Information 
    std::string url("file://");
    url += std::string(OPENHRP_PREFIX);
    url += std::string("/etc/HRP2JRL/HRP2JRLmain.wrl");

    privateCorbaObject->HRP2info = privateCorbaObject->attLoader->loadURL(url.c_str());
    
    // TO CHECK ON SCREEN
    cout << endl ;
    cout << "Loaded URL        : " << privateCorbaObject->HRP2info->getUrl()<<endl;
    cout << "Loaded Model Name : " << privateCorbaObject->HRP2info->getCharObject()->name() << endl ; 
    cout << "Loaded Model Size : " << privateCorbaObject->HRP2info->getCharObject()->modelObjectSeq()->length() << endl;
  }
  return KD_OK;
}

// ==============================================================================

ktStatus ChppciOpenHrpClient::getObstacleURL(std::string inFilename)
{
  if (privateCorbaObject->getModelLoader() != KD_OK){
      return KD_ERROR;
  }
  
  if(privateCorbaObject->attLoader) {
    //Get HRP2 Information 
    std::string url("file://");
    url += std::string(OPENHRP_PREFIX);
    url += std::string("/etc/");
    url += inFilename;
   
    try {
      privateCorbaObject->obstInfoVector.push_back(privateCorbaObject->attLoader->loadURL(url.c_str()));
      int sz_obst = privateCorbaObject->obstInfoVector.size();
	                 
      // TO CHECK ON SCREEN
      cout << endl ;
      cout << "Vector Obstacle " << 0 << endl ; 
      cout << "Loaded URL        : " << privateCorbaObject->obstInfoVector[0]->getUrl() << endl;
      cout << "Loaded Model Name : " << privateCorbaObject->obstInfoVector[0]->getCharObject()->name() << endl ; 
      cout << "Loaded Model Size : " << privateCorbaObject->obstInfoVector[0]->getCharObject()->modelObjectSeq()->length() << endl ;
    } catch (ModelLoader::ModelLoaderException& exception) {
      std::cerr << "ChppciOpenHrpClient::getObstacleURL: could not open file" << std::endl;
      return KD_ERROR;
    }
  } 
  return KD_OK;
}
  

// ==============================================================================

#if 0
ktStatus ChppciOpenHrpClient::getInfoFromCorba()
{
  int argc=4;
  char *argv[4] = {"unused",
		   "-vrml",
		   "HRP2JRL",
		   "file://"OPENHRP_PREFIX"/etc/HRP2JRL/HRP2JRLmain.wrl"};

  int vrml_idx=0;
  for (unsigned int i=0; (int)i<argc; i++){
    if (strcmp("-vrml", argv[i])==0){
      vrml_idx = ++i;
    }
  }

  if (privateCorbaObject->getModelLoader() != KD_OK){
      return KD_ERROR;
  }

  try{
    if (vrml_idx>0){
      for (unsigned int i=vrml_idx;(int)i<argc-1;){ 
	
	cout << " ---------------------------------------- " << endl ;
	cout << "Character argv    : " << argv[i] << endl;
	cout << "Model     argv    : " << argv[i+1] << endl;
	

	//
	// GET INFORMATIONS
	//
	if(privateCorbaObject->attLoader) {
	  if(strstr(argv[i],"HRP2") != NULL){ 
	    
	    //Get HRP2 Information 
	    privateCorbaObject->HRP2info = privateCorbaObject->attLoader->loadURL(argv[i+1]);
	    
	    // TO CHECK ON SCREEN
            cout << endl ;
	    cout << "Loaded URL        : " << privateCorbaObject->HRP2info->getUrl()<<endl;
	    cout << "Loaded Model Name : " << privateCorbaObject->HRP2info->getCharObject()->name() << endl ; 
	    cout << "Loaded Model Size : " << privateCorbaObject->HRP2info->getCharObject()->modelObjectSeq()->length() << endl;
	  }
	  else {  
	   	    
	    // Get Obstacle Information
	    privateCorbaObject->obstInfoVector.push_back(privateCorbaObject->attLoader->loadURL(argv[i+1]));
	    int sz_obst = privateCorbaObject->obstInfoVector.size();
	    int idLastObst = sz_obst - 1 ;

	                 
	    // TO CHECK ON SCREEN
	    cout << endl ;
	    cout << "Vector Obstacle " << idLastObst << endl ; 
	    cout << "Loaded URL        : " << privateCorbaObject->obstInfoVector[idLastObst]->getUrl() << endl;
	    cout << "Loaded Model Name : " << privateCorbaObject->obstInfoVector[idLastObst]->getCharObject()->name() << endl ; 
	    cout << "Loaded Model Size : " << privateCorbaObject->obstInfoVector[idLastObst]->getCharObject()->modelObjectSeq()->length() << endl ;
	  }
	}
	else
	  cout<<"ERROR :  ChppciOpenHrpClient::getInfoFromCorba : LOADER NULL"<<endl;
	i+=2;
      }
    }
    
    cout << " ---------------------------------------- " << endl ;

  } catch (CORBA::SystemException &ex) {
    cerr << "System exception: " << ex._rep_id() << endl;
    return KD_ERROR;
  } catch (...){
    cerr << "Unknown exception" << endl;
    return KD_ERROR;
  }
  
  return KD_OK;
}
  
#endif