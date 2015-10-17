#ifndef __XMLREADER_H_
#define __XMLREADER_H_
#include "util.h"
#include "tinyxml.h"

namespace olib {
	class IXmlObject {
	public:
		virtual ~IXmlObject() {}

		virtual s8 GetAttributeInt8(const char * attr) const = 0;
		virtual s16 GetAttributeInt16(const char * attr) const = 0;
		virtual s32 GetAttributeInt32(const char * attr) const = 0;
		virtual s64 GetAttributeInt64(const char * attr) const = 0;
		virtual float GetAttributeFloat(const char * attr) const = 0;
		virtual bool GetAttributeBoolean(const char * attr) const = 0;
		virtual const char * GetAttributeString(const char * attr) const = 0;

		virtual const char * CData() = 0;
		virtual const char * Text() = 0;

		virtual IXmlObject& operator[](const s32 index) = 0;
		virtual s32 Count() = 0;

		virtual IXmlObject& operator[](const char * name) = 0;
	};

    class XmlReader {
    public:
		XmlReader() : _root(nullptr) {}
		~XmlReader();

		bool LoadXml(const char * path);

		IXmlObject& Root();

    private:
		IXmlObject * _root;
		TiXmlDocument _doc;
    };
}

#endif //__XMLREADER_H_
