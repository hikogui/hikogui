
module;

#include <memory> // XXX #619

export module hikogui_observer;
export import : observed;
export import : observed_value;
export import : observer_intf;
export import : shared_state;
import hikogui_dispatch_socket_event; // XXX #616
export import hikogui_observer_group_ptr;
