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

template<class T>
void mmInitialize(T & val) {
	hasInit<T>::Call_Optional(val);
}


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

template<class T>
void mmDestroy(T & val) {
	hasDestroy<T>::Call_Optional(val);
}


template <class T>
class Pointer {
	friend class Pointer < T >;
private:
	int index;
	int size;
	void Set(int i);
	int GetIndex() const{
		return index;
	}
	int GetSize() const{
		return size;
	}
public:
	Pointer(){
		Init();
	}
	void Init(){
		index = -1;
		size = sizeof(T);
	}
	~Pointer(){
		if (IsGood()) {
			mmDestroy(Get());
		}
		Set(-1);
	}
	void Clear() {
		Set(-1);
	}
	bool IsGood(){
		if (size > 0 && index >= 0)
			return true;
		return false;
	}
	Pointer& operator=(const Pointer& param) {
		if (this == &param)
			return *this;
		Set(param.GetIndex());
		return *this;
	}
	operator bool(){
		return (IsGood());
	}
	void Allocate();
	T& operator* ();
	T* operator& ();
	T& Get();
};


class mm {
	template <class T> friend class Pointer;
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


template<class T> void Pointer<T>::Set(int i) {
   int* count = (int*)mm::get().GetObject(index, size);
   if (count != 0) {
       (*count)--;
       mm::get().GC(index, size);
   }
   index = i;
   count = (int*)mm::get().GetObject(index, size);
   if (count != 0)
       (*count)++;
}

template<class T> void Pointer<T>::Allocate() {
   Set(mm::get().Allocate(size));
   if (IsGood()) {
       T* t = &Get();
       mmInitialize<T>(*t);
   }
}

template<class T> T& Pointer<T>::operator* () {
   return *((T*)(((int*)(mm::get().GetObject(index, size))) + 1));
}

template<class T> T* Pointer<T>::operator& () {
   return ((T*)(((int*)(mm::get().GetObject(index, size))) + 1));
}

template<class T> T& Pointer<T>::Get() {
   return *((T*)(((int*)(mm::get().GetObject(index, size))) + 1));
}
