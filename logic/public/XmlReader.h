#ifndef __XMLREADER_H_
#define __XMLREADER_H_
#include "util.h"

namespace olib {
	class KERNEL_API IXmlObject {
	public:
		virtual ~IXmlObject() {}

		virtual s8 GetAttributeInt8(const char * attr) const = 0;
		virtual s16 GetAttributeInt16(const char * attr) const = 0;
		virtual s32 GetAttributeInt32(const char * attr) const = 0;
		virtual s64 GetAttributeInt64(const char * attr) const = 0;
		virtual float GetAttributeFloat(const char * attr) const = 0;
		virtual bool GetAttributeBoolean(const char * attr) const = 0;
		virtual const char * GetAttributeString(const char * attr) const = 0;
		virtual bool HasAttribute(const char * attr) const = 0;

		virtual const char * CData() const = 0;
		virtual const char * Text() const = 0;

		virtual const IXmlObject& operator[](const s32 index) const = 0;
		virtual const s32 Count() const = 0;

		virtual const IXmlObject& operator[](const char * name) const = 0;
		virtual bool IsExist(const char * name) const = 0;
		virtual void ForEach(const std::function<void(const char *, const IXmlObject&)>&) const = 0;
	};

    class KERNEL_API XmlReader {
    public:
		XmlReader() : _root(nullptr) {}
		~XmlReader();

		bool LoadXml(const char * path);

		const IXmlObject& Root();

    private:
		IXmlObject * _root;
    };
}

#endif //__XMLREADER_H_
