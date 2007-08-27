#ifndef CHPPCISERVERPRIVATE_H
#define CHPPCISERVERPRIVATE_H

#include "hppciRobot.h"
#include "hppciObstacle.h"
#include "hppciProblem.h"

class ChppciServerPrivate {
public:
  ~ChppciServerPrivate() {
    if (robotServant) delete robotServant;
    if (obstacleServant) delete obstacleServant;
    if (problemServant) delete problemServant;
  };
private:
  CORBA::ORB_var orb;
  PortableServer::POA_var poa;
#if 0
  PortableServer::POAManager_var mgr;
#endif
  /// \brief Implementation of object ChppciRobot
  ChppciRobot_impl *robotServant;
  /// \brief Implementation of object ChppciObstacle
  ChppciObstacle_impl *obstacleServant;
  /// \brief Implementation of object ChppciProblem.
  ChppciProblem_impl *problemServant;
#if 0
  /// \brief Stringifield reference of object.
  CORBA::String_var strRef;
  CosNaming::Name objectName;
#endif
  /// \brief Corba context.
  CosNaming::NamingContext_var hppContext;
  // methods
  /// \brief Create context.
  CORBA::Boolean createHppContext();
  /// \brief Store objects in Corba name service.
  CORBA::Boolean bindObjectToName(CORBA::Object_ptr objref,
				  CosNaming::Name objectName);


  friend class ChppciServer;
};

#endif