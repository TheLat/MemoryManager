# MemoryManager
I am writing a memory manager to make life easier in C++.  The goal is to create something that replaces new and delete and maintains or improves speed while also handling garbage collection and preventing memory fragmentation.

### Features:
* Statistics Logging:  Yes
* Lost Memory Tracking:  Yes
* Costructors:  Yes
* Destructors:  Yes
* Handles Pointers To Primitives:  Yes
* Handles Pointers To Objects:  Yes
* Handles Pointers To Arrays Of Objects:  Yes
* Handles Pointers To Dynamic Arrays:  **No**
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