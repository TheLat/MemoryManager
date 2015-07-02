#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>

#define INITIAL_SIZE 5

#ifndef _MSC_VER
inline static void fopen_s(FILE **F, const char *filename, const char *options) {
	*F = fopen(filename, options);
}
#endif

template< typename T>
struct hasInit
{
	template<typename U>
	static std::true_type Check(void (U::*)()) {
		return std::true_type();
	}
	template <typename U>
	static decltype(Check(&U::Init))
		Check(decltype(&U::Init), void *) {
		typedef decltype(Check(&U::Init)) return_type;
		return return_type();
	}
	template<typename U>
	static std::false_type Check(...) {
		return std::false_type();
	}
	typedef decltype(Check<T>(0, 0)) type;
	static const bool value = type::value;
	static void Call_Optional(T & t, std::true_type) {
		t.Init();
	}
	static void Call_Optional(...){
	}
	static void Call_Optional(T & t) {
		Call_Optional(t, type());
	}
};


template< typename T>
struct hasDestroy
{
	template<typename U>
	static std::true_type Check(void (U::*)()) {
		return std::true_type();
	}

	template <typename U>
	static decltype(Check(&U::Destroy))
		Check(decltype(&U::Destroy), void *) {
		typedef decltype(Check(&U::Destroy)) return_type;
		return return_type();
	}
	template<typename U>
	static std::false_type Check(...) {
		return std::false_type();
	}
	typedef decltype(Check<T>(0, 0)) type;
	static const bool value = type::value;
	static void Call_Optional(T & t, std::true_type) {
		t.Destroy();
	}
	static void Call_Optional(...){
	}
	static void Call_Optional(T & t) {
		Call_Optional(t, type());
	}
};



template< typename T>
struct hasRefCount
{
	template<typename U>
	static std::true_type Check(int (U::*)()) {
		return std::true_type();
	}
	template <typename U>
	static decltype(Check(&U::RefCount))
		Check(decltype(&U::RefCount), int *) {
		typedef decltype(Check(&U::RefCount)) return_type;
		return return_type();
	}
	template<typename U>
	static std::false_type Check(...) {
		return std::false_type();
	}
	typedef decltype(Check<T>(0, 0)) type;
	static const bool value = type::value;
	static int Call_Optional(T & t, std::true_type) {
		return t.RefCount();
	}
	static int Call_Optional(...){
		return 0;
	}
	static int Call_Optional(T & t) {
		return Call_Optional(t, type());
	}
};

template<class T>
void mmInitialize(T & val) {
	hasInit<T>::Call_Optional(val);
}

template<class T>
void mmDestroy(T & val) {
	hasDestroy<T>::Call_Optional(val);
}

template<class T>
int mmRefCount(T & val) {
	return hasRefCount<T>::Call_Optional(val);
}


template<typename T, int size> void mmRecursiveInitialize(T(&val)[size]){
	for (int i = 0; i < size; ++i) {
		mmInitialize(val[i]);
	}
}
template<typename T> void mmRecursiveInitialize(T(&)){
}
template<typename T, int size> void mmRecursiveDestroy(T(&val)[size]){
	for (int i = 0; i < size; ++i) {
		mmDestroy(val[i]);
	}
}
template<typename T> void mmRecursiveDestroy(T(&)){
}

template <class T, int N=-1> class Pointer {
	friend class Pointer < T >;
protected:
	bool destroyed;
	int index;
	int size;
	inline bool IsArray() {
		return N == -1;
	}
	void Set(int i){
		int* count = CountReferences();
		if (count != 0) {
			(*count)--;
			DestroyContents();
			mm::get().GC(index, size);
		}
		index = i;
		count = CountReferences();
		if (count != 0) {
			(*count)++;
			destroyed = false;
		}
	}
	int GetIndex() const{
		return index;
	}
	int GetSize() const{
		return size;
	}
	int* CountReferences(){
		return (int*)mm::get().GetObject(index, size);
	}
	void DestroyContents(){
		if (destroyed)
			return;
		if (IsGood() && CountReferences() && *CountReferences() == mmRefCount(Get())) {
			mmRecursiveDestroy(Get());
			mmDestroy(Get());
			destroyed = true;
		}
	}
	void Clear(){
		Set(-1);
	}
public:
	void Grow(int newsize) {
		static_assert(IsArray(), "Can't grow non-arrays!");
	}
	void Pack(int newsize) {
		static_assert(IsArray(), "Can't pack non-arrays!");
		Grow(newsize);
	}
	Pointer(){
		Init();
	}
	void Init(){
		index = -1;
		size = sizeof(T);
	}
	~Pointer(){
		Clear();
	}
	void Destroy(){
		Set(-1);
	}
	bool IsGood(){
		if (size > 0 && index >= 0)
			return true;
		return false;
	}
	Pointer& operator=(const Pointer& param){
		if (this == &param)
			return *this;
		Set(param.GetIndex());
		if (IsGood())
			destroyed = false;
		return *this;
	}
	bool operator==(const Pointer& param){
		if (index == param.GetIndex() && size == param.GetSize())
			return true;
		return false;
	}
	operator bool(){
		return (IsGood());
	}
	void Allocate(){
		Set(mm::get().Allocate(size));
		if (IsGood()) {
			T* t = &Get();
			mmInitialize<T>(*t);
			mmRecursiveInitialize(*t);
			destroyed = false;
		}
	}
	T& operator* (){
		return *((T*)(((int*)(mm::get().GetObject(index, size))) + 1));
	}
	T* operator& (){
		return ((T*)(((int*)(mm::get().GetObject(index, size))) + 1));
	}
	T& Get(){
		return *((T*)(((int*)(mm::get().GetObject(index, size))) + 1));
	}
};


class mm {
	template <class T, int N> friend class Pointer;
private:
	char** tables;
	int* sizes;
	int* goodIndex;
	int NumTables;
	void GrowTable(int index){
		int newsize = sizes[index] * 2;
		if (newsize == 0) {
			newsize = INITIAL_SIZE;
		}
		char* newtable = new(char[(index + sizeof(int))*newsize]);
		memset(newtable, 0, (index + sizeof(int))*newsize);
		if (sizes[index] > 0){
			memcpy(newtable, tables[index], (index + sizeof(int))*sizes[index]);
			delete [] tables[index];
		}
		tables[index] = newtable;
		int oldsize = sizes[index];
		sizes[index] = newsize;

		for (int i = oldsize; i < newsize; ++i) {
			*((int*)(GetObject(i, index)) + (sizeof(void*) / sizeof(int))) = i + 1;
		}
		*((int*)(GetObject(newsize - 1, index)) + (sizeof(void*) / sizeof(int))) = -1;
		goodIndex[index] = oldsize;
	}
	void GrowTables(int NewTable){
		if (NewTable < NumTables) {
			return;
		}
		char** temp = new(char*[NewTable + 1]);
		memset(temp, 0, (NewTable + 1) * sizeof(int));
		if (NumTables > 0)
			memcpy(temp, tables, sizeof(int) * (NumTables));
		delete tables;
		tables = temp;

		int* tempsizes = new(int[NewTable + 1]);
		memset(tempsizes, 0, (NewTable + 1) * sizeof(int));
		if (NumTables > 0)
			memcpy(tempsizes, sizes, sizeof(int) * (NumTables));
		delete sizes;
		sizes = tempsizes;

		int* tempGoodIndex = new(int[NewTable + 1]);
		memset(tempGoodIndex, 0, (NewTable + 1)*sizeof(int));
		if (NumTables > 0)
			memcpy(tempGoodIndex, goodIndex, sizeof(int) * (NumTables));
		delete goodIndex;
		goodIndex = tempGoodIndex;

		NumTables = NewTable + 1;
	}
	mm(){
		tables = 0;
		sizes = 0;
		NumTables = 0;
	}
	int Allocate(int size){
		if (NumTables == 0 || NumTables < size) {
			GrowTables(size);
			GrowTable(size);
		}
		if (sizes[size] == 0) {
			GrowTable(size);
		}
		//Find free memory
		if (size < 4) {
			//not bothering with optimization for objects of 1-3 bytes
			int i = 0;
			while (i < sizes[size]) {
				if (*((int*)&tables[size][i*(sizeof(int) + size)]) == 0)
					break;
				++i;
			}
			if (i < sizes[size]) {
				memset(&tables[size][i*(sizeof(int) + size)], 0, sizeof(int) + size);
				return i;
			}
			else {
				GrowTable(size);
				return Allocate(size);
			}
		}
		else {
			if (goodIndex[size] == -1) {
				GrowTable(size);
				return Allocate(size);
			}
			else {
				int index = goodIndex[size];
				goodIndex[size] = *(((int*)(GetObject(goodIndex[size], size))) + 1);
				memset(&tables[size][index*(sizeof(int) + size)], 0, sizeof(int) + size);
				return index;
			}
		}
	}
	void* GetObject(int index, int size) {
		if (index < 0)
			return 0;
		if (size < 1)
			return 0;
		if (tables == 0)
			return 0;
		if (sizes == 0)
			return 0;
		if (sizes[size] <= index)
			return 0;
		if (tables[size] == 0)
			return 0;
		return (void*)&tables[size][index*(size + sizeof(int))];
	}
	void GC(int index, int size) {
		if (size >= 4) {
			int* obj = (int*)GetObject(index, size);
			if (obj && *obj == 0) {
				*(obj + (sizeof(void*) / sizeof(int))) = goodIndex[size];
				goodIndex[size] = index;
			}
		}
	}
	~mm() {
		FILE * out;
		fopen_s(&out, "Memory Stats.txt", "w");

		fprintf(out, "Memory Use:\n");
		for (int i = 1; i < NumTables; ++i) {
			if (sizes[i])
				fprintf(out, "Objects of size %i bytes: %i\n", i, sizes[i]);
		}
		fprintf(out, "\nLost objects:\n");
		for (int i = 1; i < NumTables; ++i) {
			int k;
			k = 0;
			for (int j = 0; j < sizes[i]; ++j) {
				if (*((int*)GetObject(j, i)) != 0) {
					k++;
				}
			}
			if (sizes[i])
				fprintf(out, "Objects of size %i bytes: %i\n", i, k);
		}
		fclose(out);
	}
public:
	static mm& get()
	{
		static mm instance;
		return instance;
	}
};