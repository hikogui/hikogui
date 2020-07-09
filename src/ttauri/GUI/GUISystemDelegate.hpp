// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace tt {

class GUISystemDelegate {
public:
    virtual void lastWindowClosed() = 0;
};

/** Delegate for GUI related events.
*/
inline GUISystemDelegate *guiDelegate = nullptr;

}
