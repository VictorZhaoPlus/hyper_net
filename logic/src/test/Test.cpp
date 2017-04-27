#include "Test.h"
#include <string>
#include "IMapReduce.h"

Test * Test::s_self = nullptr;
IKernel * Test::s_kernel = nullptr;

class TestResult : public IMapReduceResult {
public:
	TestResult(s64 data) : _data(data) {}

	virtual void Release() { DEL this; }

	virtual const void * Context() const { return &_data; }
	virtual void Merge(IMapReduceResult * rhs) {
		_data = (_data > ((TestResult*)rhs)->_data) ? _data : ((TestResult*)rhs)->_data;
	}

private:
	s64 _data;
};

class TestTask : public IAsyncMapReduceTask {
public:
	TestTask() {}
	virtual ~TestTask() {}

	virtual std::vector<std::function<IMapReduceResult * (IKernel * kernel)>> Split(IKernel * kernel) {
		std::vector<std::function<IMapReduceResult * (IKernel * kernel)>> ret;

		for (s32 i = 0; i < 4; ++i) {
			std::vector<s32> data;
			for (s32 j = 0; j < 10; ++j) {
				s32 r = rand() % 1071;
				printf("%d ", r);
				data.push_back(r);
			}
			ret.push_back([data](IKernel * kernel) -> IMapReduceResult * {
				s32 a = data.front();
				for (auto b : data) {
					a = (a < b) ? b : a;
				}
				return NEW TestResult(a);
			});
		}
		printf("\n");

		return std::move(ret);
	}

	virtual void OnComplete(IKernel * kernel, IMapReduceResult * result) {
		printf("max %lld\n", *(s64*)result->Context());
		DEL this;
	}

private:

};

bool Test::Initialize(IKernel * kernel) {
    s_self = this;
    s_kernel = kernel;

    return true;
}

bool Test::Launched(IKernel * kernel) {
	FIND_MODULE(_mapReduce, MapReduce);

	_mapReduce->StartAsyncTask(NEW TestTask, __FILE__, __LINE__);
    return true;
}

bool Test::Destroy(IKernel * kernel) {
    DEL this;
    return true;
}

