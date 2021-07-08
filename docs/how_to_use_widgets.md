How to use widgets
==================

The following widgets exists:

- **Simple widgets**
  - `tt::text_widget`: Displays text.
  - `tt::icon_widget`: Displays a small image.
  - `tt::label_widget`: Lays out and displays text and a icon together.

- **Container widgets**
  - `tt::grid_widget`: Lays out children in a grid of variable sized cells.
  - `tt::row_column_widget`: Lays out children in a row or column.
  - `tt::tab_widget`: Shows one child at a time.
  - `tt::scroll_widget`: Allows a larger child to be shown in less space.
  - `tt::overlay_widget`: Shows a child anywhere on the window, overlaying above
    any other widget.
  - `tt::toolbar_widget`: Lays out children in a toolbar.

- **Buttons**
  - `tt::momentary_button_widget`: A push button designed to be used with a
    callback function.
  - `tt::toolbar_button_widget`: A push button designed to be used with a
    callback function. Specifically for use inside a `tt::toolbar_widget`.
  - `tt::toggle_widget`: A button representing a binary choice and indiciating
    an immediate effect.
  - `tt::checkbox_widget`: A button representing a binary choice, but also being
    able to show a third 'other' state.
  - `tt::radio_button_widget`: A button representing one out of mutually
    exclusive choices.
  - `tt::toolbar_tab_button_widget`: A button representing one out of mutally
    exclusive tabs. Designed to control the `tt::tab_widget`. For use inside a
    `tt::toolbar_widget`.
  - `tt::menu_button_widget`: A button used inside menus.
- **Misc**
  - `tt::selection_widget`: A selection widget allows selecting one out of a set
    of choices.
  - `tt::text_field_widget`: A text field widgets allows a user to enter free
    form text.
  - `tt::window_widget`: A window widget is directly owned by a window.
  - `tt::window_traffic_light_widget`: This widget displays the minimize,
    maximize and close button of a window.
  - `tt::scroll_bar_widget`: This widget shows a scroll-bar and is part of a
    `scroll_widget`.
  - `tt::system_menu_widget`: The system menu is a logo to show for the window
    and also is the mouse target for the system-menu that every window has in
    Windows 10.


Creating Windows
----------------

You can create a window by calling the `tt::gui_system::make_window()` member
function of a global object that can be accessed using
`tt::gui_system::global()`.

The first time `tt::gui_system::global()` is used the GUI system is started.
From this point it is required that every call into the GUI system is done from
the same thread, called the `gui_thread`.

The first argument to `tt::gui_system::make_window()` is the title of the
window, of type `tt::label`, which consists of an icon and translatable text.

The second optional argument is a subclass of `tt::gui_window_delegate`. The window
delegate can be used to store data with the window and provide initialization of
the widgets.

The window will automatically determine its size based on the widgets that will
be added to window.

You can add widgets to the content area of a window, by calling
`tt::grid_widget::make_widget()` on the reference returned by
`tt::gui_window::content()`.

You can add widgets to the toolbar of a window, by calling
`tt::toolbar_widget::make_widget()` on the reference returned by
`tt::gui_window::toolbar()`.

After creating at least one window you should call `tt::gui_system::loop()` on
the global object. This function will enter the system's GUI-loop, monitor
keyboard & mouse events and render all the windows.


Widget
------

Widget are graphical elements which are shown inside a window, and often can be
interacted with using the mouse an keyboard.

Many widgets such as the grid layout widget, column layout widget or the scroll
view widget are containers for other widgets.

Most widgets are build out of other widgets, for example: a label widget
contains both a text and a icon widget. And a button widget will contain a label
widget.

Container widgets will have a `make_widget<T>([position], args...)` member
function to add widgets to it. This function will allocate a new widget of type
`T`, it will set the `window` and `parent` and forward `args...` to the
constructor of the new widget.

Certain container's `make_widget<T>([position], args...)` have one or more
`position` arguments, these are used to position the new widget inside the
container. The `tt::grid_widget` for example use a spreadsheet address for the
cell-location that the new widget is positioned in.

There are often two different ways to construct a widget: with a delegate or
with an observable.

### Observable

An observable is a value that will use callbacks to notify listeners when its
value changes. Unlike other parts of the GUI system, observables may be read and
written from any thread.

In the example below a checkbox monitors the observable `my_value`;
- when the value is `bar` the box is checked,
- when the value is `foo` the box is unchecked,
- when the value is anything else the box shows a dash.

```cpp
enum class my_type {foo, bar, baz};

observable<my_type> my_value;
window.content().make_widget<checkbox_widget>("A1", my_value, my_type::bar, my_type::foo);
```

As you can see the checkbox_widget will work with custom types. For the checkbox
the type needs to be equality comparable and assignable.

It is also possible to chain observables to each other. Chaining is done by
assigning an observable to another observable. In the example we make another
checkbox, but now it will listen to the `my_chain` observable. When `my_value`
gets assigned to `my_chain`, `my_chain` will start observing `my_value`. Any
modification of `my_value` will be observed by the checkbox through the chain of
observers.

```cpp
enum class my_type {foo, bar, baz};

observable<my_type> my_value;
observable<my_type> my_chain;
window.content().make_widget<checkbox_widget>("A1", my_chain, my_type::bar, my_type::foo);

my_chain = my_value;
my_value = my_type::bar;
```

Observables are used for many member variables of a widget, including the
`tt::widget::enabled`, `tt::widget::visible` members and various labels.

### Delegates

Widget may be controlled through a delegate object. The widget queries a
delegate for the data to display and sends messages to the delegate when a user
interacts with the widget.

Delegates are actually the primary way for controlling a widget, the
`tt::observable` examples above are implemented by templated default-delegates.

In the example below a user defined instance of `my_delegate` is passed to the
constructor of the `tt::checkbox_button`. `my_delegate` must inherit from
`tt::button_delegate`.

```cpp
auto delegate = std::make_shared<my_delegate>();
auto button = window.make_widget<checkbox_button>("A1", delegate));
```

The `tt::gui_system`, `tt::gui_window` and the widgets will retain only
a std::weak_ptr to the given delegate.
