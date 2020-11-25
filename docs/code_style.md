Code Style rules
================

Identifiers
-----------
The following identifiers are in snake\_case:
 - functions,
 - member functions,
 - variables,
 - member variables,
 - arguments,
 - enum values,
 - struct types,
 - union types,
 - enum types.

Capital letters within a snake\_case identifier are allowed for proper nouns and
abrivations.

Private or protected member variables are prefixed with "\_", so that
getter/setter member functions names will not alias with the variables.

Private or protected member functions may have the "\_" prefix when they
implement the functionality of a public function of the same name.

Macros, because they do not belong to a namespace, are prefixed with "tt\_".

The suffix "\_" may be used when a new variable needs to be introduced
when only its type has changed using casting or conversion.
If more than one such variable is needed the name of the type should be appended.

Global variables
----------------
Singletons (classes that can only be instantiated once) are discouraged.

Instead add a static-member-variable to the class, which is a raw-pointer to an instance
of that class. When there is only one such member, call it "global".

These "global" static-member-variables should be allocated and initiaized in the constructor
of the tt::application class. Using tt::application as the owner of the global instance of
these classes will make sure that these instances are cleaned up before main() has finished.

It is recommended that the "global" static-member-variables are unique\_ptr and
shared\_ptr.

If it is not possible to add a static-member-variable due to circular dependencies than
nameing a global variable as the name of the followed by the name of the global variable with
and underscore "\_" as separator.

Another interesting global variable, either as a static-member-variable or as a global
variable would be a mutex named "mutex".


