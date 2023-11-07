

module;
#include "../macros.hpp"

#include <coroutine>

export module hikogui_GUI : widget_intf;
import : hitbox;
import : keyboard_focus_group;
import : widget_id;
import : widget_layout;
import hikogui_GFX;
import hikogui_coroutine;
import hikogui_layout;
import hikogui_telemetry;

export namespace hi { inline namespace v1 {
class gui_window;

class widget_intf {
public:
    /** The numeric identifier of a widget.
     *
     * @note This is a uint32_t equal to the operating system's accessibility identifier.
     */
    widget_id id = {};

    /** Pointer to the parent widget.
     * May be a nullptr only when this is the top level widget.
     */
    widget_intf *parent = nullptr;

    /** Notifier which is called after an action is completed by a widget.
     */
    hi::notifier<void()> notifier;

    virtual ~widget_intf()
    {
        release_widget_id(id);
    }

    widget_intf(widget_intf const *parent) noexcept : id(make_widget_id()), parent(const_cast<widget_intf *>(parent)) {}

    /** Subscribe a callback to be called when an action is completed by the widget.
    */
    template<forward_of<void()> Func>
    [[nodiscard]] callback<void()> subscribe(Func&& func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return notifier.subscribe(std::forward<Func>(func), flags);
    }

    /** Await until an action is completed by the widget.
     */
    [[nodiscard]] auto operator co_await() const noexcept
    {
        return notifier.operator co_await();
    }

    /** Set the window for this tree of widgets.
     *
     * @param window A pointer to the window that will own this tree of widgets.
     *               or nullptr if the window must be removed.
     */
    virtual void set_window(gui_window *window) noexcept = 0;

    /** Get the window that the widget is owned by.
     *
     * @return window The window that owns this tree of widgets. Or nullptr
     *                if this tree of widgets is not owned by a window.
     */
    [[nodiscard]] virtual gui_window *window() const noexcept = 0;

    /** Get a list of child widgets.
     */
    [[nodiscard]] virtual generator<widget_intf&> children(bool include_invisible) noexcept = 0;

    /** Get a list of child widgets.
     */
    [[nodiscard]] virtual generator<widget_intf const&> children(bool include_invisible) const noexcept final
    {
        for (auto& child : const_cast<widget_intf *>(this)->children(include_invisible)) {
            co_yield child;
        }
    }

    /** Update the constraints of the widget.
     *
     * Typically the implementation of this function starts with recursively calling update_constraints()
     * on its children.
     *
     * If the container, due to a change in constraints, wants the window to resize to the minimum size
     * it should call `request_resize()`.
     *
     * @post This function will change what is returned by `widget::minimum_size()`, `widget::preferred_size()`
     *       and `widget::maximum_size()`.
     */
     [[nodiscard]] virtual box_constraints update_constraints() noexcept = 0;

    /** Update the internal layout of the widget.
     * This function is called when the size of this widget must change, or if any of the
     * widget request a re-layout.
     *
     * This function may be used for expensive calculations, such as geometry calculations,
     * which should only be done when the data or sizes change; it should cache these calculations.
     *
     * @post This function will change what is returned by `widget::size()` and the transformation
     *       matrices.
     * @param context The layout for this child.
     */
    virtual void set_layout(widget_layout const& context) noexcept = 0;

    /** Get the current layout for this widget.
     */
    virtual widget_layout const& layout() const noexcept = 0;

    /** Draw the widget.
     *
     * This function is called by the window (optionally) on every frame.
     * It should recursively call this function on every visible child.
     * This function is only called when `updateLayout()` has returned true.
     *
     * The overriding function should call the base class's `draw()`, the place
     * where the call this function will determine the order of the vertices into
     * each buffer. This is important when needing to do the painters algorithm
     * for alpha-compositing. However the pipelines are always drawn in the same
     * order.
     *
     * @param context The context to where the widget will draw.
     */
    virtual void draw(draw_context const& context) noexcept = 0;

    /** Find the widget that is under the mouse cursor.
     * This function will recursively test with visual child widgets, when
     * widgets overlap on the screen the hitbox object with the highest elevation is returned.
     *
     * @param position The coordinate of the mouse local to the widget.
     * @return A hit_box object with the cursor-type and a reference to the widget.
     */
    [[nodiscard]] virtual hitbox hitbox_test(point2 position) const noexcept = 0;

    /** Check if the widget will accept keyboard focus.
     *
     */
    [[nodiscard]] virtual bool accepts_keyboard_focus(keyboard_focus_group group) const noexcept = 0;

    /** Request the widget to be redrawn on the next frame.
     */
    virtual void request_redraw() const noexcept = 0;

    /** Send a event to the window.
     */
    virtual bool process_event(gui_event const& event) const noexcept = 0;

    /** Handle command.
     * If a widget does not fully handle a command it should pass the
     * command to the super class' `handle_event()`.
     */
    virtual bool handle_event(gui_event const& event) noexcept = 0;

    /** Handle command recursive.
     * Handle a command and pass it to each child.
     *
     * @param event The command to handle by this widget.
     * @param reject_list The widgets that should ignore this command
     * @return True when the command was handled by this widget or recursed child.
     */
    virtual bool handle_event_recursive(
        gui_event const& event,
        std::vector<widget_id> const& reject_list = std::vector<widget_id>{}) noexcept = 0;

    /** Find the next widget that handles keyboard focus.
     * This recursively looks for the current keyboard widget, then returns the
     * next (or previous) widget that returns true from
     * `accepts_keyboard_focus()`.
     *
     * @param current_keyboard_widget The widget that currently has focus; or
     *        nullptr to get the first widget that accepts focus.
     * @param group The group to which the widget must belong.
     * @param direction The direction in which to walk the widget tree.
     * @return A pointer to the next widget.
     * @retval current_keyboard_widget When current_keyboard_widget was found
     *         but no next widget that accepts keyboard focus was found.
     * @retval nullptr When current_keyboard_widget is not found in this widget.
     */
    [[nodiscard]] virtual widget_id find_next_widget(
        widget_id current_keyboard_widget,
        keyboard_focus_group group,
        keyboard_focus_direction direction) const noexcept = 0;

    /** Get a list of parents of a given widget.
     * The chain includes the given widget.
     */
    [[nodiscard]] std::vector<widget_id> parent_chain() const noexcept
    {
        std::vector<widget_id> chain;

        if (auto w = this) {
            chain.push_back(w->id);
            while (to_bool(w = w->parent)) {
                chain.push_back(w->id);
            }
        }

        return chain;
    }

    /** Scroll to show the given rectangle on the window.
     * This will call parents, until all parents have scrolled
     * the rectangle to be shown on the window.
     *
     * @param rectangle The rectangle in window coordinates.
     */
    virtual void scroll_to_show(hi::aarectangle rectangle) noexcept = 0;

    /** Scroll to show the important part of the widget.
     */
    void scroll_to_show() noexcept
    {
        scroll_to_show(layout().rectangle());
    }
};

widget_intf *get_if(widget_intf *start, widget_id id, bool include_invisible) noexcept
{
    hi_assert_not_null(start);

    if (start->id == id) {
        return start;
    }
    for (auto& child : start->children(include_invisible)) {
        if (hilet r = get_if(&child, id, include_invisible); r != nullptr) {
            return r;
        }
    }
    return nullptr;
}

widget_intf& get(widget_intf& start, widget_id id, bool include_invisible)
{
    if (auto r = get_if(std::addressof(start), id, include_invisible); r != nullptr) {
        return *r;
    }
    throw not_found_error("get widget by id");
}

}} // namespace hi::v1
