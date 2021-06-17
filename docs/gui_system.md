How to GUI system 
=================

This document describes how to use widgets.


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

 - **true-value**: a checkmark is shown inside the box and the
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


