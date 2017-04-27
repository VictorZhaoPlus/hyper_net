#include "cluster/Cluster.h"
#include "master/Master.h"
#include "starter/Starter.h"
#include "slave/Slave.h"

GET_DLL_ENTRANCE;
CREATE_MODULE(Cluster)
CREATE_MODULE(Master)
CREATE_MODULE(Starter)
CREATE_MODULE(Slave)
