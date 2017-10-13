#include "CacheDB.h"
#include "IRedis.h"
#include "OArgs.h"
#include "XmlReader.h"

#define MAX_KEYS 128
#define MAX_ARGS 4096

class CacheDBReader : public ICacheDBReader {
public:
	CacheDBReader(const CacheDB::CacheTable& desc, IArgs<MAX_KEYS, MAX_ARGS>& args) : _desc(desc), _args(args), _count(0) {}
	virtual ~CacheDBReader() {}

	virtual void ReadColumn(const char * column) {
		OASSERT(_desc.columns.find(column) != _desc.columns.end(), "wtf");
		_args << column;
		++_count;
	}

	inline s32 Count() const { return _count; }

private:
	const CacheDB::CacheTable& _desc;
	IArgs<MAX_KEYS, MAX_ARGS>& _args;
	s32 _count;
};

class CacheDBReadResult : public ICacheDBReadResult {
public:
	CacheDBReadResult(const IRedisResult * rst) : _rst(rst) {}
	virtual ~CacheDBReadResult() {}

	virtual s32 Count() { return _rst->Count(); }

	virtual s8 GetDataInt8(s32 row, s32 col) const {
		s8 value = 0;
		_rst->GetResult(row, [&value, col](const IKernel * kernel, const IRedisResult * data) {
			return data->GetResult(col, [&value](const IKernel * kernel, const IRedisResult * data) {
				value = data->AsInt8();
				return true;
			});
		});
		return value;
	}

	virtual s16 GetDataInt16(s32 row, s32 col) const {
		s16 value = 0;
		_rst->GetResult(row, [&value, col](const IKernel * kernel, const IRedisResult * data) {
			return data->GetResult(col, [&value](const IKernel * kernel, const IRedisResult * data) {
				value = data->AsInt16();
				return true;
			});
		});
		return value;
	}

	virtual s32 GetDataInt32(s32 row, s32 col) const {
		s32 value = 0;
		_rst->GetResult(row, [&value, col](const IKernel * kernel, const IRedisResult * data) {
			return data->GetResult(col, [&value](const IKernel * kernel, const IRedisResult * data) {
				value = data->AsInt32();
				return true;
			});
		});
		return value;
	}

	virtual s64 GetDataInt64(s32 row, s32 col) const {
		s64 value = 0;
		_rst->GetResult(row, [&value, col](const IKernel * kernel, const IRedisResult * data) {
			return data->GetResult(col, [&value](const IKernel * kernel, const IRedisResult * data) {
				value = data->AsInt64();
				return true;
			});
		});
		return value;
	}

	virtual float GetDataFloat(s32 row, s32 col) const {
		float value = 0.f;
		_rst->GetResult(row, [&value, col](const IKernel * kernel, const IRedisResult * data) {
			return data->GetResult(col, [&value](const IKernel * kernel, const IRedisResult * data) {
				value = data->AsFloat();
				return true;
			});
		});
		return value;
	}

	virtual const char * GetDataString(s32 row, s32 col) const {
		const char * value = "";
		_rst->GetResult(row, [&value, col](const IKernel * kernel, const IRedisResult * data) {
			return data->GetResult(col, [&value](const IKernel * kernel, const IRedisResult * data) {
				value = data->AsString();
				return true;
			});
		});
		return value;
	}

	virtual const void * GetBinary(s32 row, s32 col, s32& size) const {
		const void * value = nullptr;
		_rst->GetResult(row, [&value, col, &size](const IKernel * kernel, const IRedisResult * data) {
			return data->GetResult(col, [&value, &size](const IKernel * kernel, const IRedisResult * data) {
				value = data->AsBlob(size);
				return true;
			});
		});
		return value;
	}

private:
	const IRedisResult * _rst;
};

class CacheDBContext : public ICacheDBContext {
public:
	CacheDBContext(const CacheDB::CacheTable& desc, IArgs<MAX_KEYS, MAX_ARGS>& args) : _desc(desc), _args(args), _count(0), _index(false) {}
	virtual ~CacheDBContext() {}

	virtual void WriteInt8(const char * col, s8 val) { _args << col << val; ++_count; CheckIndex(col); }
	virtual void WriteInt16(const char * col, s16 val) { _args << col << val; ++_count; CheckIndex(col); }
	virtual void WriteInt32(const char * col, s32 val) { _args << col << val; ++_count; CheckIndex(col); }
	virtual void WriteInt64(const char * col, s64 val) { _args << col << val; ++_count; CheckIndex(col); }
	virtual void WriteFloat(const char * col, float val) { _args << col << val; ++_count; CheckIndex(col); }
	virtual void WriteString(const char * col, const char * val) { _args << col << val; ++_count; CheckIndex(col); }

	inline void CheckIndex(const char * col) {
		if (_desc.index.name == col)
			_index = true;
	}

	inline bool IsChangedIndex() const { return _index; }
	inline s32 Count() const { return _count; }

private:
	const CacheDB::CacheTable& _desc;
	IArgs<MAX_KEYS, MAX_ARGS>& _args;
	s32 _count;
	bool _index;
};


bool CacheDB::Initialize(IKernel * kernel) {
    _kernel = kernel;

	char path[512] = { 0 };
	SafeSprintf(path, sizeof(path), "%s/config/cache_db.xml", tools::GetWorkPath());
	olib::XmlReader conf;
	if (!conf.LoadXml(path)) {
		OASSERT(false, "load object.xml failed");
		return false;
	}

	bool ret = true;
	conf.Root().ForEach([this, &ret](const char * name, const olib::IXmlObject& section) {
		if (section[0].IsExist("table")) {
			const olib::IXmlObject& tables = section[0]["table"];
			for (s32 j = 0; j < tables.Count(); ++j) {
				CacheTable desc;
				desc.del = tables[j].GetAttributeBoolean("del");
				const olib::IXmlObject& columns = tables[j]["column"];
				for (s32 i = 0; i < columns.Count(); ++i) {
					const char * name = columns[i].GetAttributeString("name");
					const char * typeStr = columns[i].GetAttributeString("type");

					if (!strcmp(typeStr, "s8")) {
						desc.columns[name] = CDB_TYPE_INT8;
					}
					else if (!strcmp(typeStr, "s16")) {
						desc.columns[name] = CDB_TYPE_INT16;
					}
					else if (!strcmp(typeStr, "s32")) {
						desc.columns[name] = CDB_TYPE_INT32;
					}
					else if (!strcmp(typeStr, "s64")) {
						desc.columns[name] = CDB_TYPE_INT64;
					}
					else if (!strcmp(typeStr, "float")) {
						desc.columns[name] = CDB_TYPE_FLOAT;
					}
					else if (!strcmp(typeStr, "string")) {
						desc.columns[name] = CDB_TYPE_STRING;
					}
					else {
						OASSERT(false, "what's this");
						ret = false;
						return;
					}

					if (columns[i].HasAttribute("key") && columns[i].GetAttributeBoolean("key")) {
						if (desc.key == "")
							desc.key = name;
						else {
							OASSERT(false, "wtf");
							ret = false;
						}
					}
				}

				if (desc.key == "") {
					OASSERT(false, "wtf");
					ret = false;
				}

				if (tables[j].IsExist("index")) {
					const char * column = tables[j]["index"][0].GetAttributeString("name");
					auto itr = desc.columns.find(column);
					if (itr != desc.columns.end())
						desc.index = { itr->second, itr->first };
					else {
						OASSERT(false, "wtf");
					}
				}

				_tables[tables[j].GetAttributeString("name")] = desc;
			}
			
		}
	});

    return ret;
}

bool CacheDB::Launched(IKernel * kernel) {
    return true;
}

bool CacheDB::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

bool CacheDB::Read(const char * table, const CacheDBColumnFuncType& cf, const CacheDBReadFuncType& f, s32 count, ...) {
	if (count > 0 && _tables.find(table) != _tables.end()) {
		CacheTable& desc = _tables[table];
		IArgs<MAX_KEYS, MAX_ARGS> args;

		CacheDBReader reader(desc, args);
		cf(_kernel, &reader);

		va_list ap;
		va_start(ap, count);
		s8 type = desc.columns[desc.key];
		for (s32 i = 0; i < count; ++i) {
			switch (type) {
			case CDB_TYPE_INT8: args << va_arg(ap, s8); break;
			case CDB_TYPE_INT16: args << va_arg(ap, s16); break;
			case CDB_TYPE_INT32: args << va_arg(ap, s32); break;
			case CDB_TYPE_INT64: args << va_arg(ap, s64); break;
			case CDB_TYPE_STRING: args << va_arg(ap, const char*); break;
			default: OASSERT(false, "wtf");
			}
		}
		va_end(ap);

		OASSERT(reader.Count() > 0, "wtf");
		args.Fix();
		return OMODULE(Redis)->Call(0, "db_get", reader.Count(), args.Out(), [this, &f](const IKernel * kernel, const IRedisResult * rst) {
			CacheDBReadResult result(rst);
			f(_kernel, &result);
			return true;
		});
	}
	return false;
}

bool CacheDB::ReadByIndex(const char * table, const CacheDBColumnFuncType& cf, const CacheDBReadFuncType& f, const s64 index) {
	if (_tables.find(table) != _tables.end()) {
		CacheTable& desc = _tables[table];
		if (desc.index.type < CDB_TYPE_CANT_BE_KEY &&  desc.index.type != CDB_TYPE_STRING && desc.index.type != CDB_TYPE_NONE) {
			IArgs<MAX_KEYS, MAX_ARGS> args;
			CacheDBReader reader(desc, args);
			cf(_kernel, &reader);
			OASSERT(reader.Count() > 0, "wtf");

			char tmp[128];
			SafeSprintf(tmp, sizeof(tmp), "%s|i+%lld", table, index);
			args << tmp;

			args.Fix();
			return OMODULE(Redis)->Call(0, "db_get_index", reader.Count(), args.Out(), [this, &f](const IKernel * kernel, const IRedisResult * rst) {
				CacheDBReadResult result(rst);
				f(_kernel, &result);
				return true;
			});
		}
	}
	return false;
}

bool CacheDB::ReadByIndex(const char * table, const CacheDBColumnFuncType& cf, const CacheDBReadFuncType& f, const char * index) {
	if (_tables.find(table) != _tables.end()) {
		CacheTable& desc = _tables[table];
		if (desc.index.type == CDB_TYPE_STRING) {
			IArgs<MAX_KEYS, MAX_ARGS> args;
			CacheDBReader reader(desc, args);
			cf(_kernel, &reader);
			OASSERT(reader.Count() > 0, "wtf");

			char tmp[128];
			SafeSprintf(tmp, sizeof(tmp), "%s|i+%s", table, index);
			args << tmp;

			args.Fix();
			return OMODULE(Redis)->Call(0, "db_get_index", reader.Count(), args.Out(), [this, &f](const IKernel * kernel, const IRedisResult * rst) {
				CacheDBReadResult result(rst);
				f(_kernel, &result);
				return true;
			});
		}
	}
	return false;
}

bool CacheDB::Write(const char * table, const CacheDBWriteFunc& f, s32 count, ...) {
	if (count > 0 && _tables.find(table) != _tables.end()) {
		CacheTable& desc = _tables[table];
		IArgs<MAX_KEYS, MAX_ARGS> args;

		CacheDBContext context(desc, args);
		f(_kernel, &context);

		va_list ap;
		va_start(ap, count);
		s8 type = desc.columns[desc.key];
		args << desc.key.c_str() << count;
		for (s32 i = 0; i < count; ++i) {
			switch (type) {
			case CDB_TYPE_INT8: args << va_arg(ap, s8); break;
			case CDB_TYPE_INT16: args << va_arg(ap, s16); break;
			case CDB_TYPE_INT32: args << va_arg(ap, s32); break;
			case CDB_TYPE_INT64: args << va_arg(ap, s64); break;
			case CDB_TYPE_STRING: args << va_arg(ap, const char*); break;
			default: OASSERT(false, "wtf");
			}
		}
		va_end(ap);

		if (context.IsChangedIndex()) {
			char tmp[128];
			SafeSprintf(tmp, sizeof(tmp), "%s|i+", table);
			args << tmp << desc.index.name.c_str();
		}

		args.Fix();
		return OMODULE(Redis)->Call(0, "db_set", context.Count() * 2, args.Out());
	}
	return false;
}

bool CacheDB::WriteByIndex(const char * table, const CacheDBWriteFunc& f, const s64 index) {
	if (_tables.find(table) != _tables.end()) {
		CacheTable& desc = _tables[table];
		if (desc.index.type < CDB_TYPE_CANT_BE_KEY &&  desc.index.type != CDB_TYPE_STRING && desc.index.type != CDB_TYPE_NONE) {
			IArgs<MAX_KEYS, MAX_ARGS> args;

			CacheDBContext context(desc, args);
			f(_kernel, &context);

			char tmp[128];
			SafeSprintf(tmp, sizeof(tmp), "%s|i+%lld", table, index);
			args << tmp;

			args.Fix();
			return OMODULE(Redis)->Call(0, "db_set_index", context.Count() * 2, args.Out());
		}
		else {
			OASSERT(false, "wtf");
		}
	}
	return false;
}

bool CacheDB::WriteByIndex(const char * table, const CacheDBWriteFunc& f, const char * index) {
	if (_tables.find(table) != _tables.end()) {
		CacheTable& desc = _tables[table];
		if (desc.index.type == CDB_TYPE_STRING) {
			IArgs<MAX_KEYS, MAX_ARGS> args;

			CacheDBContext context(desc, args);
			f(_kernel, &context);

			char tmp[128];
			SafeSprintf(tmp, sizeof(tmp), "%s|i+%s", table, index);
			args << tmp;

			args.Fix();
			return OMODULE(Redis)->Call(0, "db_set_index", context.Count() * 2, args.Out());
		}
		else {
			OASSERT(false, "wtf");
		}
	}
	return false;
}

bool CacheDB::Destroy(const char * table, s32 count, ...) {
	if (count > 0 && _tables.find(table) != _tables.end()) {
		CacheTable& desc = _tables[table];
		IArgs<MAX_KEYS, MAX_ARGS> args;
		args << (desc.del ? 1 : 0);

		va_list ap;
		va_start(ap, count);
		s8 type = desc.columns[desc.key];
		for (s32 i = 0; i < count; ++i) {
			switch (type) {
			case CDB_TYPE_INT8: args << va_arg(ap, s8); break;
			case CDB_TYPE_INT16: args << va_arg(ap, s16); break;
			case CDB_TYPE_INT32: args << va_arg(ap, s32); break;
			case CDB_TYPE_INT64: args << va_arg(ap, s64); break;
			case CDB_TYPE_STRING: args << va_arg(ap, const char*); break;
			default: OASSERT(false, "wtf");
			}
		}
		va_end(ap);

		args.Fix();
		return OMODULE(Redis)->Call(0, "db_del", 1, args.Out());
	}
	return false;
}

bool CacheDB::DestroyByIndex(const char * table, const s64 index) {
	if (_tables.find(table) != _tables.end()) {
		CacheTable& desc = _tables[table];
		if (desc.index.type < CDB_TYPE_CANT_BE_KEY &&  desc.index.type != CDB_TYPE_STRING && desc.index.type != CDB_TYPE_NONE) {
			char tmp[128];
			SafeSprintf(tmp, sizeof(tmp), "%s|i+%lld", table, index);

			IArgs<MAX_KEYS, MAX_ARGS> args;
			args << tmp << (desc.del ? 1 : 0);
			args.Fix();

			return OMODULE(Redis)->Call(0, "db_del_index", 0, args.Out());
		}
		else {
			OASSERT(false, "wtf");
		}
	}
	return false;
}

bool CacheDB::DestroyByIndex(const char * table, const char * index) {
	if (_tables.find(table) != _tables.end()) {
		CacheTable& desc = _tables[table];
		if (desc.index.type == CDB_TYPE_STRING) {
			char tmp[128];
			SafeSprintf(tmp, sizeof(tmp), "%s|i+%s", table, index);

			IArgs<MAX_KEYS, MAX_ARGS> args;
			args << (desc.del ? 1 : 0) << tmp;
			args.Fix();

			return OMODULE(Redis)->Call(0, "db_del_index", 1, args.Out());
		}
		else {
			OASSERT(false, "wtf");
		}
	}
	return false;
}
