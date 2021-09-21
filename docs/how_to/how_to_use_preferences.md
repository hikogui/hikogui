How to use preferences
======================

Preferences
-----------

Opens the preferences file of the application.
The location of the preferences file is operating system depended. On
windows this file is located in:
`%USERPROFILE%\AppData\Local\<vendor>\<application name>\preferences.json`

```cpp
auto preferences = tt::preferences(tt::URL::urlFromApplicationPreferencesFile());
```

Observable values are linked to values in the preferences located using a json-path.
In the example below the json-path is simply "foo" which created an integer named
"foo" in the root object of the json-preferences-file.

```cpp
tt::observable<int> foo;
preferences.add("foo", foo); 
```

The example belows shows how to read the value from the observable and update it.
Since observables are monitored by the preferences object, the preferences object can
write changes to the preferences automatically.

```cpp
tt_log_info("old: {}", *foo);
foo += 1;
tt_log_info("new: {}", *foo);
```

Complex types
-------------

The example above will work with observers with types that can be natively
stored in a json file: integers, floating point, booleans, strings, vectors and maps.

For more complex types, you will need to add a template specialization for `tt::pickle`.

The example below shows how to specialize `tt::pickle` for the complex `bar` type:

```cpp
struct bar {
    int one;
    int two;

    friend bool operator==(bar const &, bar const &) = default;
};

template<>
struct tt::pickle<bar> {
    [[nodiscard]] datum encode(bar const& x) const noexcept {
        return datum::make_map("one", x.one, "two", x.two);
    }

    [[nodiscard]] bar decode(datum const& x) const {
        return bar{ static_cast<int>(x["one"]), static_cast<int>(x["two"]) };
    };
};

tt::observable<bar> foo;
preferences.add("foo", foo); 

tt_log_info("old: {}", foo->one);
foo->one += 1;
tt_log_info("new: {}", foo->one);
```

### Observable
An observable is a value that will use callbacks to notify listeners when its
value changes. Unlike other parts of the GUI system, observables may be read and
written from any thread.

### Datum


### Pickle


### JSON

