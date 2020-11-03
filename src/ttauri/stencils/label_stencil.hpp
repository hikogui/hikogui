// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Cell.hpp"
#include "../text/ShapedText.hpp"
#include "../text/TextStyle.hpp"
#include "../l10n_label.hpp"
#include <string_view>

namespace tt {

class label_cell : public cell {
public:
    TextStyle style;

    label_cell(l10n_label label, TextStyle style) noexcept;

    [[nodiscard]] vec preferred_extent() const noexcept override;

    void draw(DrawContext context, bool use_context_color = false) const noexcept override;

private:
};

} // namespace tt
