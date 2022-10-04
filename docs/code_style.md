Code Style
==========

Identifiers
-----------

### Casing

The following identifiers are in snake\_case:

 - functions,
 - member functions,
 - variables,
 - member variables,
 - arguments,
 - enum values,
 - struct types,
 - union types,
 - enum types,
 - concepts.

The following identifiers are in CamelCase:

 - template arguments.

Capital letters within a snake\_case identifier are allowed for proper nouns and
abbreviations.

### Common names

Common identifiers:

 - `i`, `j`, `k`: Index inside a loop.
 - `it`, `jt`, `kt`: Iterator inside a loop.
 - `r`: Return value being prepared.
 - `first`, `last`: Iterator arguments to a function.

### Prefixes and Suffixes

Common prefixes and suffixes:

 - prefix `num_`: Size of a list of items. The prefixed name should be plural
 - suffix `_nr`: Ordinal of an item.
 - suffix `_i` `_j` `_k`: Index inside a loop associated with an specific list.
 - suffix `_it` `_jt` `_kt`: Iterator inside a loop associated with an specific list.
 - prefix `_`: Private or protected member variables and functions.
 - suffix `_`: A variable after conversion to a different type.
 - suffix `_type`: A type created inside a class.
 - prefix `hi_`: A macro
 - suffix `_cbt`: A callback-token returned when subscribing a callback.

Private or protected member variables are prefixed with "\_", so that
getter/setter member functions names will not alias with the variables.

Private or protected member functions may have the "\_" prefix when they
implement the functionality of a public function of the same name.

Macros, because they do not belong to a namespace, are prefixed with the prefix "hi\_".

The suffix "\_" may be used when a new variable needs to be introduced
when only its type has changed using casting or conversion.
If more than one such variable is needed the name of the type should be appended.

Getters and Setters
-------------------

I prefer public member variables. In case a member variable must be protected or
private due to class-invariance then we may add a getter or a setter.

Since protected and private member variables are prefixed with "\_" we can
use the non-prefixed name for the getter. And we can use the "set\_" prefix
for setters.

When the getter returns a non-constant reference to the data member, a separate
setter is not needed.

Initializing variables
----------------------

I would like to see an assignment operator `=` with every variable initialization.
Here are some examples that are acceptable:

```cpp
auto foo = foo_type{init_value};
auto foo = foo_type(init_value);
foo_type foo = foo_type{init_value};
foo_type foo = {init_value};
foo_type foo = init_value;
```

Please do NOT use the following variable initialization as I find it difficult to read:

```cpp
foo_type foo(init_value);
```

Global variables
----------------

Singletons (classes that can only be instantiated once) are discouraged.

Instead, add a static-member-function to the class, which returns a reference to an instance
of that class. When there is only one such member, call it `global()`.

In certain cases, these `global()` member functions have to initialize a subsystem and/or make
sure that it is not being allocated during system shutdown. The `system_status` will
help with correctly instantiating and closing down of subsystems,

If it is not possible to add a static-member-function due to circular dependencies than
naming a global variable as the name of the class followed by the name of the global variable with
and underscore "\_" as separator.

Another interesting global variable, either as a static-member-variable or as a global
variable would be a mutex named `mutex`.

Two phase construction
----------------------

When a polymorphic class needs polymorphic initialization and destruction, it should
add the following two virtual functions:

 - virtual void `init()`
 - virtual void `deinit()`

`init()` should be called directly after the lifetime of the object has started. It should be called
from the same thread as its construction and the reference to the object should not have been shared
with others.

`deinit()` should be called directly before the lifetime of the object is finished. It should be called
from the same thread as the object's destructor and no reference to the object should exist anymore outside
of the current function.

This means that like in the constructor and destructor inside `init()` and `deinit()` there is no need
for handling multithreading issues.

Delegates
---------

Delegates are polymorphic class instances that are passed to an object that is being managed.
The managed object will call into the delegate to send messages and retrieve information.

The managed object should hold a `std::weak_ptr` to the base class of the delegate. This allows the
delegate to be deallocated with the managed object functioning.

When calling the delegate, the first argument `sender` should be a reference to the managed object.

Delegates should at least have the following two function to handle the lifetime of the managed object:

 - virtual void init(managed\_object &sender)
 - virtual void deinit(managed\_object &sender)

The two functions mirror the two phase construction and are often called from `init()` and `deinit()` of the managed
object. However, they may be called from the constructor and destructor of the object as well.

Subsystems
----------

A subsystem will have a name in the `system_status_type` enum, and will have a global function under the same name
suffixed with \_start. This subsystem\_start function is used to initialize and register the deinitialize function
with the system\_status. The subsystem\_start function must be low latency so that it can be called very often to make
sure the subsystem is started when functionality of the subsystem is used for the first time.

Paths
-----
Remember if you are converting between `std::string` and `std::filesystem::path` you are doing something wrong.
With std::filesystem::path a `char` is the native-narrow-encoding, this is different from the rest of the
C++ standard where `char` is in the execution-encoding.

The execution-encoding on HikoGUI is set using compiler options to UTF-8. The native-narrow-encoding is
Window's currently configured code-page for this process and is compatible with the `*A()` win32 functions.

It is highly recommended NOT to use the `*A()` win32 functions and instead use the `*W()` win32 functions.
The `*A()` win32 function do extra encoding and decoding from the current configured code-page to UTF-16-like
encoding and then call the matching `*W()` function internally.

So use the `std::filesystem::path` explicit `std::u8string`, `std::u16string` and `std::u32string` conversions
and then convert to and from `std::string` using the `hi::to_*string()` functions.
