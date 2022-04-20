Application Preferences
=======================

The preferences of an application are loaded and saved into a JSON file.
Whenever a configuration value is modified, the preferences are automatically saved.

Command Line
------------

The command line can be used to update the preferences.

Argument        | Description
----------------|:------------------------------------------------------------------
--help          | Show help information, implies --list and --exit
--list          | Show list of current and default values
--exit[=_code_] | Exit the application after handling all arguments.
--reset         | Reset the preferences to the default values.
--load=_path_   | Load a named preferences file and use it to update the preferences
--save=_path_   | Save the current preferences to a named preferences file
_key_=_value_   | Update the value
_key_           | Show current value
!_key_          | Set a value to its default

The order of operations is:

 1. Load application preferences
 2. Handle all arguments left-to-right
 3. Optionally exit the application
 4. Start application.
