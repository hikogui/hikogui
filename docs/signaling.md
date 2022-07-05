Signalling
==========

Event Loop
----------
Each thread has an event loop which is accessed through the object returned from
`hi::loop::local()`. You can also get access to the event loop of a couple of other
threads: `hi::loop::main()` and `hi::loop::timer()`.

An event-loop is meant do dispatch events from multiple sources, like:
 * network events,
 * GUI events, such as mouse and keyboard,
 * window redraw events triggered by vertical-sync,
 * timers at a specific time, after time elapsed and possibly repeating, and
 * functions being posted from the current or other threads.

The event-loop implementation is designed for low-latency so that events get
processed quickly, and for efficiency with low CPU and therefor low power usage. 

Notifier
--------
A notifier is a object which dispatches events to multiple listeners when it is triggered.
It is the standard way of adding callback functionality to a class.

A listener can subscribe a function with the notifier. When the notifier is triggered the
subscribed functions are posted to the event-loop, to be called when the event-loop is resumed.

A class can use a notifier in two ways:
 - Make the notifier a public member, which will allow for multiple notifiers.
 - Add a `subscribe()` function to the class that will forward to the
   notifier's `subscribe()` function.
 
### Subscribing
To subscribe a function with a notifier you can use the `hi::notifier::subscribe()` function,
this function will return a token which is a RAII object managing the lifetime of the subscription.
When all the copies of the token are destroyed the function is automatically unsubscribed.

In the example below we subscribe `handle_callback()` through a lambda with another object's notifier.
The token for the callback is held in this, so that when this is destroyed the callback is
automatically unsubscribed:

```
this->token = other.some_notifier([this]{ this->handle_callback(); })
```

### Triggering
Triggering is done by using the `hi::notifier::post()` or `hi::notifier::post_on_main()` functions
which post the subscribed functions on the local- or main-event loops.

### Awaiting

Observer
--------



Awaiter
-------


