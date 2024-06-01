



hi_export_module(hikogui.theme : style_priority);

hi_export namespace hi::inline v1 {


class style_priority {
public:

    [[nodiscard]] constexpr friend auto operator<=>(style_priority const& lhs, style_priority const& rhs) noexcept
    {
        if (auto const r = lhs._importance <=> rhs._importance; r != 0) {
            return r;
        }
        if (auto const r = lhs._num_ids <=> rhs._num_ids; r != 0) {
            return r;
        }
        if (auto const r = lhs._num_psuedo_classes <=> rhs._num_psuedo_classes; r != 0) {
            return r;
        }
        return lhs._num_element_names <=> rhs._num_element_names;
    }

    constexpr void set_important(style_importance value) noexcept
    {
        _importance = std::to_underlying(value);
    }

    constexpr void set_inherint(bool value) noexcept
    {
        _inerint = static_cast<uint16_t>(value);
    }

    [[nodiscard]] constexpr bool inherint() const noexcept
    {
        return static_cast<bool>(_inherint);
    }

private:
    uint16_t _inherint : 1 = 1;
    uint16_t _importantance : 3 = 0;
    uint16_t _num_ids : 4 = 0;
    uint16_t _num_pseudo_classes : 4 = 0;
    uint16_t _num_element_names : 4 = 0;
};



}
