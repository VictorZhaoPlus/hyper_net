/*
 * File: IAnalysis.h
 * Author: ooeyusea
 *
 * Created On February 16, 2016, 08:31 AM
 */

#ifndef __IANALYSIS_H__
#define __IANALYSIS_H__
 
#include "IModule.h"

class IAnalysis : public IModule {
public:
	virtual ~IAnalysis() {}
};

class IAnalysisClient : public IModule {
public:
	virtual ~IAnalysisClient() {}
};

#endif /*__IANALYSIS_H__ */