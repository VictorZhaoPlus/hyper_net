#ifndef __VISIONCONTROLLERFACTORY_H__
#define __VISIONCONTROLLERFACTORY_H__
#include "singleton.h"
#include "../IVisionController.h"

class VCFactory : public OSingleton<VCFactory> {
	friend class OSingleton<VCFactory>;
public:
	IVisionController * Create(const char * type);

	bool Ready();

private:
	VCFactory() {}
	virtual ~VCFactory() {}

	std::map<std::string, std::function<IVisionController * ()>> _creators;
};

#endif //__VISIONCONTROLLERFACTORY_H__

