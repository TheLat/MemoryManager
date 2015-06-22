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
* Handles Pointers To Dynamic Arrays:  **No**
* Detects And Frees Arbitrary Graphs With No External References:  **No**
* Ability To Start Tables At Previous Maximum Load:  **No**
* Threadsafe: **No**

### Use:
#### Declaration:
Access to the memory manager's contents are extremely limited and should be done exclusively through the Pointer class.  Declaring a pointer should look like this:
```
Pointer<object> Variable;
```
object can be any data type.  On declaration, variable's Init() function will automatically be called.  This will also call object's Init() function, if it exists and follows the following declaration:
```
void Init();
```
If object is an array of objects, it will call every element's Init() function.
#### Destruction:
When a Pointer falls out of scope or is deleted (shame on you), it will call the Destroy function of the object it's pointing to if it is a valid pointer and if the object's destroy function follows the following declaration:
```
void Destroy();
```
and if this Pointer is the last pointer pointing to the object.  Then it will set itself to the invalid pointer.  If you need to trigger this, you can call the Pointer's Destroy() function.
#### Null Pointer:
A Pointer's default state is the null pointer and can be used in if statements as normal pointers can, though it has slightly deeper validation checking.  Using the un-dereferenced pointer in an if statement is equivalent to calling the IsGood() function.  You can set a Pointer to the null pointer by calling the Clear() function.  If this pointer is the last pointer to the object, it will call the Destroy function of that object, if it exists.  In general, this should be done over calling the Destroy() fuction.
#### Allocation:
You can allocate a new object to a pointer by calling its Allocate() function.  This will implicitly call Clear(), and Destroy() if appropriate.
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
Using the Pointer in boolean expressions will evaluate whether or not the Pointer points to a valid object.
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