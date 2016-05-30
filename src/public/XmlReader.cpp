#include "XmlReader.h"
#include <vector>
#include "tools.h"
#include <unordered_map>
#include "tinyxml.h"

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

		virtual const char * CData() const { OASSERT(false, "this is a null xml"); return nullptr; }
		virtual const char * Text() const { OASSERT(false, "this is a null xml"); return nullptr; }

		virtual const IXmlObject& operator[](const s32 index) const { OASSERT(false, "this is a null xml"); return *this; }
		virtual const s32 Count() const { OASSERT(false, "this is a null xml"); return 0; }

		virtual const IXmlObject& operator[](const char * name) const { OASSERT(false, "this is a null xml"); return *this; }
		virtual bool IsExist(const char * name) const { OASSERT(false, "this is a null xml"); return false; }
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

		virtual const char * CData() const { OASSERT(false, "this is a array xml"); return nullptr; }
		virtual const char * Text() const { OASSERT(false, "this is a array xml"); return nullptr; }

		virtual const IXmlObject& operator[](const s32 index) const;
		virtual const s32 Count() const { return (s32)_elements.size(); }

		virtual const IXmlObject& operator[](const char * name) const { OASSERT(false, "this is a array xml"); return _null; }
		virtual bool IsExist(const char * name) const { OASSERT(false, "this is a array xml"); return false; }

	private:
		std::vector<XmlObject *> _elements;
		XmlNull _null;
	};

	class XmlObject : public IXmlObject{
		struct Value {
			std::string valueString;
			s64 valueInt64;
			float valueFloat;
			bool valueBoolean;
		};

	public:
		XmlObject(const TiXmlElement * element) {
			LoadAttrs(element);
			LoadChildren(element);
			LoadText(element);
		}

		virtual ~XmlObject() {
			for (auto itr = _objects.begin(); itr != _objects.end(); ++itr)
				DEL itr->second;
			_objects.clear();
		}

		virtual s8 GetAttributeInt8(const char * attr) const { const Value * value = FindAttr(attr); return value ? (s8)value->valueInt64 : 0; }
		virtual s16 GetAttributeInt16(const char * attr) const { const Value * value = FindAttr(attr); return value ? (s16)value->valueInt64 : 0; }
		virtual s32 GetAttributeInt32(const char * attr) const { const Value * value = FindAttr(attr); return value ? (s32)value->valueInt64 : 0; }
		virtual s64 GetAttributeInt64(const char * attr) const { const Value * value = FindAttr(attr); return value ? value->valueInt64 : 0; }
		virtual float GetAttributeFloat(const char * attr) const { const Value * value = FindAttr(attr); return value ? value->valueFloat : 0; }
		virtual bool GetAttributeBoolean(const char * attr) const { const Value * value = FindAttr(attr); return value ? value->valueBoolean : 0; }
		virtual const char * GetAttributeString(const char * attr) const { const Value * value = FindAttr(attr); return value ? value->valueString.c_str() : nullptr; }

		virtual const char * CData() const {  return _text.c_str(); }
		virtual const char * Text() const { return _text.c_str();}

		virtual const IXmlObject& operator[](const s32 index) const { OASSERT(false, "this is a obejct xml"); return _null; }
		virtual const s32 Count() const { OASSERT(false, "this is a obejct xml"); return 0; }

		virtual const IXmlObject& operator[](const char * name) const {
			auto itr = _objects.find(name);
			if (itr == _objects.end()) {
				OASSERT(false, "where is child %s ???", name);
				return _null;
			}
			return *itr->second;
		}

		virtual bool IsExist(const char * name) const { return _objects.find(name) != _objects.end(); }

	private:
		const Value * FindAttr(const char * attr) const {
			auto itr = _attrs.find(attr);
			if (itr != _attrs.end())
				return &itr->second;
			return nullptr;
		}

		void LoadAttrs(const TiXmlElement * element) {
			for (auto * attr = element->FirstAttribute(); attr; attr = attr->Next()) {
				const char * name = attr->Name();
				const char * value = attr->Value();

				Value unit;
				unit.valueString = value;
				unit.valueFloat = tools::StringAsFloat(value);
				unit.valueInt64 = tools::StringAsInt64(value);
				unit.valueBoolean = tools::StringAsBool(value);
				_attrs[attr->Name()] = unit;
			}
		}

		void LoadChildren(const TiXmlElement * element) {
			for (auto * node = element->FirstChildElement(); node; node = node->NextSiblingElement()) {
				if (_objects.find(node->Value()) == _objects.end())
					_objects[node->Value()] = NEW XmlArray;

				_objects[node->Value()]->AddElement(node);
			}
		}

		void LoadText(const TiXmlElement * element) {
			if (element->GetText())
				_text = element->GetText();
		}

	private:
		std::unordered_map<std::string, Value> _attrs;
		std::string _text;

		std::unordered_map<std::string, XmlArray*> _objects;
		XmlNull _null;
	};

	void XmlArray::AddElement(const TiXmlElement * element) {
		_elements.push_back(NEW XmlObject(element));
	}

	const IXmlObject& XmlArray::operator[](const s32 index) const {
		OASSERT(index >= 0 && index < (s32)_elements.size(), "xml index out of range");
		return *_elements[index];
	}

	XmlReader::~XmlReader() {
		if (_root)
			DEL _root;
	}

	bool XmlReader::LoadXml(const char * path) {
		TiXmlDocument doc;
		if (!doc.LoadFile(path)) {
			OASSERT(false, "can't find xml file : %s", path);
			return false;
		}

		const TiXmlElement * root = doc.RootElement();
		if (root == nullptr) {
			OASSERT(false, "core xml format error");
			return false;
		}
		
		_root = NEW XmlObject(root);
		return true;
	}

	const IXmlObject& XmlReader::Root() {
		OASSERT(_root, "where is root???");
		return *_root;
	}
}
