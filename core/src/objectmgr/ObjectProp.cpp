#include "ObjectProp.h"
#include "MMObject.h"
#include "ObjectDescriptor.h"

const s8 ObjectProp::GetType(IObject * object) const {
	s32 objectTypeId = ((MMObject*)object)->GetDescriptor()->GetTypeId();
	OASSERT(objectTypeId > 0 && objectTypeId <= _size, "wtf");

	return _layouts[objectTypeId - 1]->type;
}

const s32 ObjectProp::GetSetting(IObject * object) const {
	s32 objectTypeId = ((MMObject*)object)->GetDescriptor()->GetTypeId();
	OASSERT(objectTypeId > 0 && objectTypeId <= _size, "wtf");

	return _layouts[objectTypeId - 1]->setting;
}
