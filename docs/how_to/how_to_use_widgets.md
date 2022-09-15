How to use widgets
==================

Creating Windows
----------------

In the following example, a new window is created. The `make_window()` function
first argument is the `hi::label` of the window. The label consists of both a
translatable text and an icon.

```cpp
auto gui = hi::gui_system::make_unique();
auto &window = gui->make_window(hi::tr("The window title"));
```

The second optional argument to `make_window()` is a subclass of
`hi::gui_window_delegate`. The window delegate can be used to store data with
the window and provide initialization of the widgets.

Unlike many other GUI libraries, a window is not given a size; its initial,
minimum and maximum size is determined by the constraints and the layout
algorithm of the widgets being displayed in the window.

The `hi::gui_system::make_unique()` function returns a `std::unique_ptr`
to a new instance of a configured GUI system. From this point forward, any
usage of the GUI system, its windows and their widgets must be done from
the same thread, called the `gui_thread`.

After creating at least one window, you should call `hi::gui_system::loop()`
member function. This function will enter the system's GUI-loop, monitor
keyboard & mouse events and render all the windows. Once all windows are
closed the `loop()` function will return with a return code, which may be
returned from the `main()` function.

```cpp
int hi_main(int argc, char *argv[])
{
    auto gui = hi::gui_system::make_unique();
    auto &window = gui->make_window(hi::tr("The window title"));
    window.content().make_widget<momentary_button_widget>("A1", tr("Does nothing"));
    return gui->loop();
}
```

How to add widgets
------------------

Widget are graphical elements which are shown inside a window and often can be
interacted with using the mouse and keyboard, such as various kinds of buttons,
text fields and selection boxes.

Many widgets such as the grid layout widget, column layout widget or the scroll
view widget are containers for other widgets. The top level widget of a window
for example contains two containers a `hi::grid_widget` for the content area and
a `hi::toolbar_widget`.

In the example below we are adding 4 widgets to the content area of the window.

The `hi::window::content()` function returns a reference to the `hi::grid_widget`,
 and we use its `make_widget<>()` function to add new widgets. The template
 argument is the type of widget to instantiate and the first argument is the
 position within the grid widget. The rest of the arguments are passed to the
 constructor of the new widget.

```cpp
int hi_main(int argc, char *argv[])
{
    observer<int> value = 0;

    auto gui = hi::gui_system::make_unique();
    auto &window = gui->make_window(hi::tr("Radio button example"));

    window.content().make_widget<label_widget>("A1", tr("radio buttons:"));
    window.content().make_widget<radio_button_widget>("B1", value, 1, tr("one"));
    window.content().make_widget<radio_button_widget>("B2", value, 2, tr("two"));
    window.content().make_widget<radio_button_widget>("B3", value, 3, tr("three"));

    return gui->loop();
}
```

There are often two different ways to construct a widget: with a delegate or
with an observer. In the example above we use an `hi::observer<int>` for the
radio buttons to monitor and update. Sharing the same observer allows the
radio buttons to act as a set.

### Layout using the grid widget

The `hi::grid_widget` is a powerful layout widget which allows adding of new widgets
using the `hi::grid_widget::make_widget<>()` member function.

The template parameter for `make_widget()` specifies the widget class to allocate and construct.
The first argument to `make_widget()` is a string specifying the location where the
new widget should be positioned. The rest of the arguments are passed to the constructor of
the new widget.

The location is specified using a spreadsheet-like address.

There are two forms:

 - single cell, examples: "A1", "C4", "AB45"
 - cell range, examples: "A1:E1", "C4:C7", "Z3:AA4"

A widget may span multiple rows and columns. Widget that spans multiple column or rows
are often widgets that are resizable; therefor the `grid_widget` will override the
preferred- and maximum constraint of other widgets in those rows or columns.


### Observer

An observer is a type that observers a value, it will use callbacks to notify listeners when the
observed value changes. Unlike other parts of the GUI system, observers are thread-save and
may be read and written from any thread.

In the example below a checkbox monitors the observer `my_value`:

- when the value is `bar` the box is checked,
- when the value is `foo` the box is unchecked,
- when the value is anything else the box shows a dash.

```cpp
enum class my_type {foo, bar, baz};

observer<my_type> my_value;
window.content().make_widget<checkbox_widget>("A1", my_value, my_type::bar, my_type::foo);
```

As you can see, the `checkbox_widget` will work with custom types. For the checkbox
the type needs to be equality comparable and assignable.

It is also possible to chain observers to each other. Chaining is done by
assigning an observer to another observer. You can also get a chained sub-observer,
selecting a member variable or the result of the index-operator from the observed value.

In the example below, we make another checkbox, but now it will listen to
the `my_chain` observer. When `my_value` gets assigned to `my_chain`,
`my_chain` will start observing `my_value`. Any modification of `my_value`
will be observed by the checkbox through the chain of observers.

```cpp
enum class my_type {foo, bar, baz};

observer<my_type> my_value;
observer<my_type> my_chain;
window.content().make_widget<checkbox_widget>("A1", my_chain, my_type::bar, my_type::foo);

my_chain = my_value;
my_value = my_type::bar;
```

Observers are used for many member variables of a widget, including the
`hi::widget::enabled`, `hi::widget::visible` members and various labels.

### Delegates

A widget may also be controlled through a delegate object. The widget queries a
delegate for the data to display and sends messages to the delegate when a user
interacts with the widget.

Delegates are actually the primary way for controlling a widget, the
`hi::observer` examples above are implemented by templated default-delegates.

In the example below, a user defined instance of `my_delegate` is passed to the
constructor of the `hi::checkbox_button`. `my_delegate` must inherit from
`hi::button_delegate`.

```cpp
auto delegate = std::make_shared<my_delegate>();
auto button = window.make_widget<checkbox_button>("A1", delegate));
```

A list of widgets
-----------------

- **Simple widgets**
  - `hi::text_widget`: Displays, select and edit text.
  - `hi::icon_widget`: Displays a small image.
  - `hi::label_widget`: Lays out and displays text and a icon together.

- **Container widgets**
  - `hi::grid_widget`: Lays out children in a grid of variable sized cells.
  - `hi::row_column_widget`: Lays out children in a row or column.
  - `hi::tab_widget`: Shows one child at a time.
  - `hi::scroll_widget`: Allows a larger child to be shown in less space.
  - `hi::overlay_widget`: Shows a child anywhere on the window, overlaying above
    any other widget.
  - `hi::toolbar_widget`: Lays out children in a toolbar.

- **Buttons**
  - `hi::momentary_button_widget`: A push button designed to be used with a
    callback function.
  - `hi::toolbar_button_widget`: A push button designed to be used with a
    callback function. Specifically for use inside a `hi::toolbar_widget`.
  - `hi::toggle_widget`: A button representing a binary choice and indiciating
    an immediate effect.
  - `hi::checkbox_widget`: A button representing a binary choice, but also being
    able to show a third 'other' state.
  - `hi::radio_button_widget`: A button representing one out of mutually
    exclusive choices.
  - `hi::toolbar_tab_button_widget`: A button representing one out of mutally
    exclusive tabs. Designed to control the `hi::tab_widget`. For use inside a
    `hi::toolbar_widget`.
  - `hi::menu_button_widget`: A button used inside menus.
- **Misc**
  - `hi::selection_widget`: A selection widget allows selecting one out of a set
    of choices.
  - `hi::text_field_widget`: A text field widgets allows a user to type in
    a value of different types: std::string, int, float, etc.
  - `hi::window_widget`: A window widget is directly owned by a window.
  - `hi::window_traffic_light_widget`: This widget displays the minimize,
    maximize and close button of a window.
  - `hi::scroll_bar_widget`: This widget shows a scroll-bar and is part of a
    `scroll_widget`.
  - `hi::system_menu_widget`: The system menu is a logo to show for the window
    and also is the mouse target for the system-menu that every window has in
    Windows 10.
