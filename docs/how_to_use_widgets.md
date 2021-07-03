How to use widgets
==================

Creating Windows
----------------
You can create a window by calling the `tt::gui_system::make_window()`
member function of a global object that can be accessed using
`tt::gui_system::global()`.

The first time `tt::gui_system::global()` is used the GUI system is
started. From this point it is required that every call into the GUI
system is done from the same thread, called the `gui_thread`.

The first argument to make\_window() is the title of the window, of
type `tt::label`, which consists of an icon and translatable text.

The second optional argument is a subclass of `tt::window_delegate`.
The window delegate can be used to store data with the window
and provide initialization of the widgets.

The window will automatically determine its size based on the
widgets that will be added to window.

After creating at least one window and optionally populating
the window(s) with widgets; you should call
`tt::gui_system::global().loop()`. This function will enter
the system's GUI-loop, which will monitor keyboard, mouse events
and start rendering of the windows.

You can add new widgets to the window by using the
`tt::gui_window::make_widget<>()` and
`tt::gui_window::make_toolbar_widget()` functions.
More information on how to add widgets is in the next chapter,
specifics about window can be found in `tt::window_widget`
information below.

Widget
------
Widget are graphical elements which are shown inside a window,
and often can be interacted with using the mouse an keyboard.

Many widgets such as the grid layout widget, column layout widget
or the scroll view widget are containers for other widgets.

But most other widgets are build out of other widgets, for example:
a label widget contains both a text and a icon widget. And
a button widget will contain a label widget.

Most widgets will have a `make_widget<>()` member function to
add widgets to it. The template argument will
be the type of the widget to instantiate and the arguments
will be passed to the constructor of the widget.

Certain container widgets may have extra arguments for the
`make_widget<>()` function which are used as a position for
the new widget inside the container.

### Grid Layout Widget
The grid layout widget calculates the size of each
row and column, based on the minimum, preferred and
maximum size of each child widget.

This will also determine the minimum, preferred and
maximum size of the grid layout widget itself.

During layout each widget is positioned and sized based
on the width and height of each cell.

The `tt::grid_layout_widget()` constructor has one
optional argument, a `std::weak_ptr` to a subclass
of `tt::grid_layout_delegate`. It is mostly used to
populate the children of the grid during initialization.

The are two version of the
`tt::grid_layout_widget::make_widget<>()` function:
 - With the first argument a string literal with a
   spreadsheet-like cell coordinate, A column index as
   a combination of letters, followed by a row index
   as a combination of digits.
 - With the first argument the column index and
   the second argument the row index.

Observable
----------
The first concept to learn about is the `tt::observable<>` type.
An observable is a value that will use callbacks to notify listeners
when its value changes.

For example a `tt::widget::enable` flag is of type `tt::observable<bool>`,
the widget listens to any changes to this enable flag and change its appearance.
In the example below the button is disabled:

```
auto button = window.make_widget<button_widget>("A1", l10n("My Button"));
button->enable = false;
```

It is also possible to chain observables to each other. Chaining is done
by assigning an observable to another observable. In the example below
we again disable the button, but now through an second observable.

```
observable<bool> enable = true;

auto button = window.make_widget<button_widget>("A1", l10n("My Button"));
button->enable = enable;
enable = false;
```

Observables are used for many member variables of a widget, including
the `tt::widget::enabled`, `tt::widget::visible` members and various
labels. They are also used by value-delegates that some widget use
by default.

Delegates
---------
Many widget can be controlled through a delegate object. The widget
queries a delegate for the data to display and sends messages to
the delegate when a user interacts with the widget.

In the example below a user defined instance of `my_delegate` is
passed to the constructor of the `tt::checkbox_button`.
`my_delegate` must inherit from `tt::button_delegate`.

```
auto delegate = std::make_shared<my_delegate>();
auto button = window.make_widget<checkbox_button>("A1", delegate));
```

Many widgets also include a templated constructor allowing
an observable of user specified type to be passed. This constructor
will instantiate a value-delegate to control the widget. This
allows a widget to be controlled by a user-specified observable value.

In the example below we use the automatic value-delegate to control
a checkbox using an observable on our own enum type. The value-delegate
uses an observable and an `on_value` and `off_value`.

```
enum class my_enum_type { foo, bar, baz };
observable<my_enum_type> my_value = my_enum_type::foo;

auto button = window.make_widget<checkbox_button>("A1", my_value, my_enum_type::foo, my_enum_type::bar));
```


System
------


Window
------


Widget
------

### Label

```c++
widget.make_widget<label_widget>("A1", l10n("My Text"));
```

### Button

```c++
auto button = wiget.make_widget<button_widget<bool>>("A1", true);
button->label = label{elusive_icon::Wrench, l10n("Preferences")};
auto callback = button->subscribe([]{ foo(); });
```

### Checkbox

The checkbox is configured with a true-value and a false-value, to match
with the observed value.

When the observed value is equal to:

 - **true-value**: a check mark is shown inside the box and the
   true\_label is shown to the right of the box,
 - **false-value**: the box is empty and the
   false\_label is shown to the right of the box,
 - neither: a dash is shown inside the box and the
   other\_label is shown to the right of the box.

Each of these labels may be empty.

```c++
// Many button widget can work with custom types.
enum class value_t { A, B, C };

// A value which can be observed by widgets and other things.
observable<value_t> value;

// Create a check button: A is true, B is false, anything else is other.
auto button = widget.make_widget<checkbox_widget<value_t>>("A1", value_t::A, value_t::B, value);

// We can set a label for each checkbox value.
button->true_label = l10n("true");
button->false_label = l10n("false");
button->other_label = l10n("other");
```

### Radio button

In ttauri a set of radio buttons are a set of `tt::radio_button_widget`s.
Each of the widgets in a set observe the same value and each widget is configured with
a different true-value.

When a `tt::radio_button_widget`'s true-value matches the observed value the radio button
is checked, otherwise it is unchecked. When a radio button is clicked the observed value
is set to that widget's true value.

The label on the right side of a radio button is always shown, but the label may be set
to empty to not show a label.

```c++
// Many button widget can work with custom types.
enum class value_t { A, B, C };

// A value which can be observed by widgets and other things.
observable<value_t> value;

// Create a radio button which is active on A, with the label "A"
auto button1 = widget.make_widget<radio_button_widget<value_t>>("A1", value_t::A, value, l10n("A"));

// Create a radio button which is active on B, with the label "B"
auto button2 = widget.make_widget<radio_button_widget<value_t>>("A2", value_t::B, value, l10n("B")));

```

### Toggle


### Text field


### Selection


### Row/Column layout

### Grid layout

### Tab View

### Toolbar

### Toolbar Tab Button

### Menu Item


