# MemoryManager
I am writing a memory manager to make life easier in C++.  The goal is to create something that replaces new and delete and maintains or improves speed while also handling garbage collection and preventing memory fragmentation.

### Features:
* Statistics Logging:  Yes
* Lost Memory Tracking:  Yes
* Costructors:  Yes
* Destructors:  Yes
* Handles Pointers To Primitives:  Yes
* Handles Pointers To Objects:  Yes
* Handles Pointers To Static Arrays Of Objects:  Yes
* Handles Pointers To Dynamic Arrays:  Yes
* Detects And Frees Arbitrary Graphs With No External References:  Yes, see RefCount
* Debugging-friendly:  Yes
* Handles Dynamic Arrays Of Length Zero:  Yes
* Ability To Pack Tables To Smallest Possible Length:  **No**
* Ability To Start Tables At Previous Maximum Load:  **No**
* Threadsafe: **No**

### Use:
#### Declaration:
Access to the memory manager's contents are extremely limited and should be done exclusively through the Pointer class.  Declaring a pointer should look like this:
```
Pointer<object> Variable;
```
object can be any data type.  On declaration, variable's Init() function will automatically be called.  This will set Variable to the null pointer.
#### Destruction:
When a Pointer falls out of scope or is deleted (shame on you), it will set itself to the null pointer.  If this is the last pointer pointing to the object, then it will call the object's Destroy() function if it is declared like this:
```
void Destroy();
```
Regard this function as the object's destructor.
#### Null Pointer:
A Pointer's default state is the null pointer and can be used in if statements as normal pointers can, though it has slightly deeper validation checking.  Using the un-dereferenced pointer in an if statement is equivalent to calling the IsGood() function.  You can set a Pointer to the null pointer by calling the Destroy() function.  If this pointer is the last pointer to the object, it will call the Destroy() function of that object, if it exists.  In general, this should be done over calling the Destroy() fuction.
#### Allocation:
You can allocate a new object to a pointer by calling its Allocate() function.  This will call the pointer's Destroy() function prior to allocating a new object.  This will call the new allocated object's Init() function if it exists and is declared like this:
```
void Init();
```
#### RefCount:
The memory manager tracks whether or not an object is lost by tracking the number of references to an object.  In some data structures, such as a graph (which exists in the example provided), reference counting is inadequate.  In those cases, you must write the RefCount() function which must be declared like this:
```
int RefCount();
```
Traversing an arbitrary graph to count all possible references is slow and impractical.  Unfortunately, you must write a function that looks at the graph and returns the number of references to the object contained within the graph.  The simpler and more efficient this function is, the better.  This function is required or cleanup of a doubly-linked list, though not a singly-linked list.
#### Pointer Operations:
```
Pointer& operator=(const Pointer& param)
```
Pointers can only have their assignment operator called for Pointers to the same type.
```
bool operator==(const Pointer& param)
```
The comparison operator will return true if both pointers are pointing to the same object or if both are the null pointer.
```
operator bool()
```
Using the Pointer in boolean expressions will evaluate whether or not the Pointer points to a valid object.  If the Pointer is to a dynamic array, this will return false if the array is of length 0 or is not allocated.
```
T& operator*()
```
Using the dereference operator gives you a reference to the object.  As with normal pointers, this will crash if you dereference a bad pointer.
```
T* operator&()
```
Using the reference operator will give you the actual memory address of the object the pointer is pointing to.  This is not recommended, as the tables can grow, which will break the raw memory addresses, though not the way that the Pointer class accesses the object.
```
T& Get()
```
Equivalent to the * operator.
```
void Peek()
```
Pointers do not know the actual memory address of the object they point to.  As such, this makes debugging very difficult.  To remedy this, Pointers have an protected variable called "obj" which is an actual pointer of the template of the Pointer class.  Calling Peek will set obj to be the actual memory address of the object.  This is only to be used for debugging purposes.
```
Pointer(const Pointer& obj)
```
The copy constructor exists and is very efficient.  However, passing by reference or constant reference is usually preferrable.
#### Dynamic Arrays:
Dynamic arrays can be declared like this:
```
Pointer<object, 0> Variable;
```
You can replace the number with any number, though must be a constant and must be positive.  This number will be the initial size of the array.  If this number is 0, it will be treated as a null pointer and can't be allocated.  TODO:  There are currently issues with dynamic array Pointers being set to equal other dynamic array pointers with different starting sizes.  Until this is resolved, it is recommended to set all Pointers to dynamic arrays to have a starting size of 0.
```
T& operator[] (int i)
```
If an object is a dynamic array, this will return the ith element of the array.
```
int Length()
```
Returns the length of the array if the pointer is to a dynamic array or 1 if it is not.
```
int Size()
```
Returns the size of the object that the Pointer pointers to.  This comes out to the sizeof whatever the Pointer is templated to, time the length if it is a dynamic array, in bytes.
```
void Resize(int newlength)
```
If the object is a dynamic array, this will change the array to the new size.  If newlength is greater than the old length, it will call Init() on all new objects in the array, if they have an Init function.  If it is less than the old length, it will call Destroy() on all of the lost objects, if they have a Destroy function.  If the Pointer has not been allocated and the new size is greater than 0, this function will allocate it.  If the size is 0, this will destroy the object in the same way that it falling out of scope would.
#### Statistics Logging
The memory manager will save its statistics to "Memory Stats.txt" in the folder that it is run in.  This is provided in plain text, human-readable format and tells both the largest levels of memory used and memory that the memory manager detects as being un-freed.  This is done by scanning through the reference counts of all memory allocated.

#### Memory tracking
The memory manager tracks whether or not an object has any pointers to it is by reference counting.  The Pointer class, when the assignment operator is called, will decrement its current object's reference count and increment its new object's reference count.  Pointers do not know their memory address.  They know the size of their template and the index into the array.

#### Memory Growing
When an object is allocated and there is no more space available in the table, the memory manager will allocate a new table twice the size of the old table, copy the old table to the new table, and free the old table.  If a table has not been allocated, it will allocate a new table of size INITIAL_SIZE, which is set to 5.

#### Memory Shrinking
The memory manager does not shrink tables.

#### Benchmarking
Data to come.