

#pragma once

namespace tt {


class glyph_ids_long {
public:
    constexpr glyph_ids_long(glyphs_ids_long const &) noexcept = default;
    constexpr glyph_ids_long(glyphs_ids_long &&) noexcept = default;
    constexpr glyph_ids_long &operator=(glyphs_ids_long const &) noexcept = default;
    constexpr glyph_ids_long &operator=(glyphs_ids_long &&) noexcept = default;

    constexpr glyph_ids_long(size_t value) noexcept : _glyphs(), _size(0)
    {
        push_back(glyph_id{static_cast<uint16_t>(value)});

        if constexpr (sizeof(size_t) == 8) {
            push_back(glyph_id{static_cast<uint16_t>(value >> 16)});
            push_back(glyph_id{static_cast<uint16_t>(value >> 32)});
        }
    }

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _size;
    }

    [[nodiscard]] constexpr glyph_id operator[](size_t i) const noexcept
    {
        return _glyphs[i];
    }

    constexpr void push_back(glyph_id id) noexcept
    {
        if (_size < 18) {
            _glyphs[_size++] = id;
        }
    }

private:
    // "Compatibility mappings are guaranteed to be no longer than 18 characters, although most consist of just a few characters."
    // https://unicode.org/reports/tr44/ (TR44 5.7.3)
    std::array<glyph_id,18> _glyphs;
    uint8_t _size;
};

class glyph_ids {
public:
    constexpr size_t sgo_size_max = sizeof(void *) == 8 ? 3 : 1;
    constexpr size_t sgo_size_mask = sizeof(void *) == 8 ? 7 : 3;
    constexpr glyph_ids_long *sgo_null = reinterpret_cast<glyph_ids_long *>(1);

    constexpr ~glyph_ids()
    {
        if ((sgo_value() & sgo_size_mask) == 0) {
            delete _long;
        }
    }

    constexpr glyph_ids(glyph_ids const &other) noexcept : _long(other._long)
    {
        if ((sgo_value() & sgo_size_mask) == 0) {
            _long = new glyph_ids_long(*(other._long));
        }
    }

    constexpr glyph_ids &operator(glyph_ids const &other) noexcept
    {
        tt_return_on_self_assign(other);
        if ((sgo_value() & sgo_size_mask) == 0) {
            delete _long;
        }
        _long = other._long;
        if ((sgo_value() & sgo_size_mask) == 0) {
            _long = new glyph_ids_long(*(other._long));
        }
        return *this;
    }

    constexpr glyph_ids(glyph_ids &&other) noexcept
    {
        std::swap(_long, other._long);
    }

    constexpr glyph_ids &operator=(glyph_ids &&other) noexcept
    {
        std::swap(_long, other._long);
        return *this;
    }

    constexpr glyph_ids() noexcept : _long(sgo_null) {}

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        ttlet size_ = sgo_value() & sgo_size_mask;
        if (size_ == 0) {
            return _long->size();
        } else {
            return size_ - 1;
        }
    }

    [[nodiscard]] constexpr glyph_id operator[](size_t i) const noexcept
    {
        tt_axiom(i < size());

        ttlet value = sgo_value();
        if ((value & sgo_size_mask) == 0) {
            return (*_long)[i];
        } else {
            return glyph_id{static_cast<uint16_t>(value >> ((i + 1) * 16))}
        }
    }

    constexpr void push_back(glyph_id id) noexcept
    {
        ttlet value = sgo_value();
        ttlet size_code = value & sgo_size_mask;

        if (size_code == 0) {
            _long->push_back(id);

        } else if (size_code <= sgi_size_max) {
            set_sgo_value((size_code + 1) | (static_cast<size_t>(id) << (size_code * 16)));

        } else {
            _long = new glyph_ids_long(value >> 16);
            _long->push_back(id);
        }
    }

private:
    glyph_ids_long *_ptr;

    constexpr void set_sgo_value(size_t value) noexcept
    {
        _long = reinterpret_cast<glyph_ids_long *>(value);
    }

    [[nodiscard]] constexpr size_t sgo_value() const noexcept
    {
        return static_cast<size_t>(reinterpret_cast<ptrdiff_t>(_ptr));
    }
};

}

