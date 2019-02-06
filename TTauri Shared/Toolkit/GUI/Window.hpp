//
//  Window.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once
#include <memory>
#include <unordered_set>
#include <boost/thread.hpp>
#include <vulkan/vulkan.hpp>
#include "Rectangle.hpp"
#include "View.hpp"
#include "BackingCache.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class Instance;
class Device;


enum class WindowType {
    WINDOW,
    PANEL,
    FULLSCREEN,
};

enum class SubpixelLayout {
    NONE,
    RGB_LEFT_TO_RIGHT,
    RGB_RIGHT_TO_LEFT,
    RGB_TOP_TO_BOTTOM,
    RGB_BOTTOM_TO_TOP,
};

enum class WindowState {
    NO_DEVICE,
    LINKED_TO_DEVICE,
    READY_TO_DRAW,
};

struct WindowStateError: virtual boost::exception, virtual std::exception {};

/*! A Window.
 * This Window is backed by a native operating system window with a Vulkan surface.
 * The Window should not have any decorations, which are to be drawn by the GUI Toolkit, because
 * modern design requires drawing of user interface elements in the border.
 */
class Window {
private:
    boost::shared_mutex m;
    WindowState state;

public:
    vk::SurfaceKHR intrinsic;

    Instance *instance;
    Device *device;

    //! Location of the window on the screen.
    Rectangle location;

    /*! Dots-per-inch of the screen where the window is located.
     * If the window is located on multiple screens then one of the screens is used as
     * the source for the DPI value.
     */
    float dpi;

    /*! Pixels-per-Point
     * A point references a typefraphic point, 1/72 inch.
     * Scale all drawing and sizing on the window using this attribute.
     * This value is rounded to an integer value for drawing clean lines.
     */
    float ppp;

    /*! Definition on how subpixels are oriented on the window.
     * If the window is located on multiple screens with different pixel layout then
     * `SubpixelLayout::NONE` should be selected.
     */
    SubpixelLayout subpixelLayout;

    //! The view covering the complete window.
    std::shared_ptr<View> view;

    /*! Type of window.
     * The type of window dictates the way the window-decoration and possibly the
     * rest of the user interface is drawn. This may switch during execution
     * for example switching to `FULLSCREEN` and back to `WINDOW`.
     */
    WindowType windowType;

    /*! A set of backings.
     */
    BackingCache backings;

    void buildSwapChainAndPipeline(void);

    void teardownSwapChainAndPipeline(void);

    void rebuildSwapChainAndPipeline(void) {
        teardownSwapChainAndPipeline();
        buildSwapChainAndPipeline();
    }

    void setDevice(Device *device);

    /*! Draw the complete window.
     * This method may be called from another thread.
     */
    void draw(void);

    Window(Instance *instance, vk::SurfaceKHR surface) :
        state(WindowState::NO_DEVICE), instance(instance), intrinsic(surface)
    {

    }



};

}}}
