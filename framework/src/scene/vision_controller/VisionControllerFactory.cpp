#include "VisionControllerFactory.h"
#include "SimpleVision.h"

bool VCFactory::Ready() {
	_creators["simple"] = []() -> IVisionController * { return NEW SimpleVision; };

	return true;
}

IVisionController * VCFactory::Create(const char * type) {
	auto itr = _creators.find(type);
	if (itr != _creators.end())
		return itr->second();
	
	return nullptr;
}

IVisionController * CreateVisionController(const char * type) {
	return VCFactory::Instance()->Create(type);
}

