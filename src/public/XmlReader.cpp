#include "XmlReader.h"
#include <vector>
#include "tools.h"
#include <unordered_map>

namespace olib {
	class XmlNull : public IXmlObject {
	public:
		XmlNull() {}
		virtual ~XmlNull() {}

		virtual s8 GetAttributeInt8(const char * attr) const { OASSERT(false, "this is a null xml"); return 0; }
		virtual s16 GetAttributeInt16(const char * attr) const { OASSERT(false, "this is a null xml"); return 0; }
		virtual s32 GetAttributeInt32(const char * attr) const { OASSERT(false, "this is a null xml"); return 0; }
		virtual s64 GetAttributeInt64(const char * attr) const { OASSERT(false, "this is a null xml"); return 0; }
		virtual float GetAttributeFloat(const char * attr) const { OASSERT(false, "this is a null xml"); return 0.f; }
		virtual bool GetAttributeBoolean(const char * attr) const { OASSERT(false, "this is a null xml"); return false; }
		virtual const char * GetAttributeString(const char * attr) const { OASSERT(false, "this is a null xml"); return nullptr; }

		virtual const char * CData() { OASSERT(false, "this is a null xml"); return nullptr; }
		virtual const char * Text() { OASSERT(false, "this is a null xml"); return nullptr; }

		virtual IXmlObject& operator[](const s32 index) { OASSERT(false, "this is a null xml"); return *this; }
		virtual s32 Count() { OASSERT(false, "this is a null xml"); return 0; }

		virtual IXmlObject& operator[](const char * name) { OASSERT(false, "this is a null xml"); return *this; }
	};

	class XmlObject;
	class XmlArray : public IXmlObject {
	public:
		XmlArray() {}
		virtual ~XmlArray() {
			for (auto * element : _elements)
				DEL element;
			_elements.clear();
		}

		void AddElement(const TiXmlElement * element);

		virtual s8 GetAttributeInt8(const char * attr) const { OASSERT(false, "this is a array xml"); return 0; }
		virtual s16 GetAttributeInt16(const char * attr) const { OASSERT(false, "this is a array xml"); return 0; }
		virtual s32 GetAttributeInt32(const char * attr) const { OASSERT(false, "this is a array xml"); return 0; }
		virtual s64 GetAttributeInt64(const char * attr) const { OASSERT(false, "this is a array xml"); return 0; }
		virtual float GetAttributeFloat(const char * attr) const { OASSERT(false, "this is a array xml"); return 0.f; }
		virtual bool GetAttributeBoolean(const char * attr) const { OASSERT(false, "this is a array xml"); return false; }
		virtual const char * GetAttributeString(const char * attr) const { OASSERT(false, "this is a array xml"); return nullptr; }

		virtual const char * CData() { OASSERT(false, "this is a array xml"); return nullptr; }
		virtual const char * Text() { OASSERT(false, "this is a array xml"); return nullptr; }

		virtual IXmlObject& operator[](const s32 index);
		virtual s32 Count() { return (s32)_elements.size(); }

		virtual IXmlObject& operator[](const char * name) { OASSERT(false, "this is a array xml"); return _null; }

	private:
		std::vector<XmlObject *> _elements;
		XmlNull _null;
	};

	class XmlObject : public IXmlObject{
	public:
		XmlObject(const TiXmlElement * element) : _element(element) {}
		virtual ~XmlObject() {
			for (auto itr = _objects.begin(); itr != _objects.end(); ++itr)
				DEL itr->second;
			_objects.clear();
		}

		virtual s8 GetAttributeInt8(const char * attr) const { return (s8)tools::StringAsInt(_element->Attribute(attr)); }
		virtual s16 GetAttributeInt16(const char * attr) const { return (s16)tools::StringAsInt(_element->Attribute(attr)); }
		virtual s32 GetAttributeInt32(const char * attr) const { return (s32)tools::StringAsInt(_element->Attribute(attr)); }
		virtual s64 GetAttributeInt64(const char * attr) const { return tools::StringAsInt64(_element->Attribute(attr)); }
		virtual float GetAttributeFloat(const char * attr) const { return tools::StringAsFloat(_element->Attribute(attr)); }
		virtual bool GetAttributeBoolean(const char * attr) const { return tools::StringAsBool(_element->Attribute(attr)); }
		virtual const char * GetAttributeString(const char * attr) const { 
			OASSERT(_element->Attribute(attr), "where is attribute %s", attr);
			return _element->Attribute(attr); 
		}

		virtual const char * CData() { 
			OASSERT(_element->FirstChild(), "where is cdata");
			return _element->FirstChild()->Value();
		}

		virtual const char * Text() { 
			return _element->GetText();
		}

		virtual IXmlObject& operator[](const s32 index) { OASSERT(false, "this is a obejct xml"); return _null; }
		virtual s32 Count() { OASSERT(false, "this is a obejct xml"); return 0; }

		virtual IXmlObject& operator[](const char * name) {
			auto itr = _objects.find(name);
			if (itr == _objects.end()) {
				const TiXmlElement * node = _element->FirstChildElement(name);
				if (node) {
					XmlArray * arr = NEW XmlArray;
					while (node) {
						arr->AddElement(node);

						node = node->NextSiblingElement(name);
					}
					_objects[name] = arr;
				}
				else {
					OASSERT(false, "where is children %s", name);
					return _null;
				}
			}
			return *_objects[name];
		}

	private:
		const TiXmlElement * _element;
		std::unordered_map<std::string, IXmlObject*> _objects;
		XmlNull _null;
	};

	void XmlArray::AddElement(const TiXmlElement * element) {
		_elements.push_back(NEW XmlObject(element));
	}

	IXmlObject& XmlArray::operator[](const s32 index) {
		OASSERT(index >= 0 && index < (s32)_elements.size(), "xml index out of range");
		return *_elements[index];
	}

	XmlReader::~XmlReader() {
		if (_root)
			DEL _root;
	}

	bool XmlReader::LoadXml(const char * path) {
		if (!_doc.LoadFile(path)) {
			OASSERT(false, "can't find xml file : %s", path);
			return false;
		}

		const TiXmlElement * root = _doc.RootElement();
		if (root == nullptr) {
			OASSERT(false, "core xml format error");
			return false;
		}
		
		_root = NEW XmlObject(root);
		return true;
	}

	IXmlObject& XmlReader::Root() {
		OASSERT(_root, "where is root");
		return *_root;
	}
}
