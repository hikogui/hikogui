Signalling
==========

Notifier
--------
A `hi::notifier` is a standardized way of adding callback ability to a class.

You can subscribe callback-functions with a notifier, which will be called when the
function-call operator is called. 


A class can use a notifier in two ways:
 - Make the notifier a public member, which will allow for multiple notifiers.
 - Add a `subscribe()` function to the class that will forward to the
   private notifier's `subscribe()` function.
  

 


Observer
--------



Awaiter
-------


