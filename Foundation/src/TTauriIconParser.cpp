
#include "TTauri/Foundation/TTauriIconParser.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/placement.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/endian.hpp"

namespace tt {

struct little_fixed1_14_buf_t {
    little_int16_buf_t v;

    float value() const noexcept {
        return (v.value()) / 16384.0f;
    }
};

struct little_fixed1_13_buf_t {
    little_int16_buf_t v;

    float value() const noexcept {
        return (v.value() >> 1) / 8192.0f;
    }

    bool flag() const noexcept {
        return (v.value() & 1) > 0;
    }
};

struct little_point_buf_t {
    little_fixed1_13_buf_t x;
    little_fixed1_13_buf_t y;

    vec coord() const noexcept {
        return vec::point(x.value(), y.value());
    }

    BezierPoint::Type type() const noexcept {
        ttlet type = (x.flag() ? 1 : 0) | (y.flag() ? 2 : 0);
        switch (type) {
        case 0b00: return BezierPoint::Type::Anchor;
        case 0b11: return BezierPoint::Type::QuadraticControl;
        case 0b01: return BezierPoint::Type::CubicControl1;
        case 0b10: return BezierPoint::Type::CubicControl2;
        default: no_default;
        }
    }

    BezierPoint value() const noexcept {
        return {coord(), type()};
    }
};

/*! wide gamut scRGB color.
 */
struct little_scRGB_buf_t {
    little_uint16_buf_t red;
    little_uint16_buf_t green;
    little_uint16_buf_t blue;
    little_uint16_buf_t alpha;

    vec value() const noexcept {
        return vec{
            (static_cast<float>(red.value()) - 4096.0f) / 8192.0f,
            (static_cast<float>(green.value()) - 4096.0f) / 8192.0f,
            (static_cast<float>(blue.value()) - 4096.0f) / 8192.0f,
            static_cast<float>(alpha.value()) / 65535.0f
        };
    }
};

struct header_buf_t {
    uint8_t magic[4];
    little_uint16_buf_t nr_paths;
};

struct path_buf_t {
    little_scRGB_buf_t fill_color;
    little_scRGB_buf_t stroke_color;
    little_fixed1_13_buf_t stroke_width;
    little_uint16_buf_t nr_contours;
};

struct contour_buf_t {
    little_uint16_buf_t nr_points;
};

struct Layer {
    Path path;
    vec fillColor;
    vec strokeColor;
    float strokeWidth;
    LineJoinStyle lineJoinStyle;
};

static std::vector<BezierPoint> parseContour(nonstd::span<std::byte const> bytes, ssize_t &offset)
{
    ttlet header = make_placement_ptr<contour_buf_t>(bytes, offset);

    ttlet nr_points = int{header->nr_points.value()};

    auto contour = std::vector<BezierPoint>{};
    contour.reserve(nr_points);

    for (int i = 0; i < nr_points; i++) {
        ttlet point = make_placement_ptr<little_point_buf_t>(bytes, offset);

        contour.push_back(point->value());
    }

    return contour;
}

static Layer parsePath(nonstd::span<std::byte const> bytes, ssize_t &offset)
{
    ttlet header = make_placement_ptr<path_buf_t>(bytes, offset);

    auto layer = Layer{};
    layer.fillColor = header->fill_color.value();
    layer.strokeColor = header->stroke_color.value();
    layer.strokeWidth = header->stroke_width.value();
    layer.lineJoinStyle = header->stroke_width.flag() ? LineJoinStyle::Bevel : LineJoinStyle::Miter;

    ttlet nr_contours = int{header->nr_contours.value()};
    for (int i = 0; i < nr_contours; i++) {
        layer.path.addContour(parseContour(bytes, offset));
    }

    return layer;
}

Path parseTTauriIcon(nonstd::span<std::byte const> bytes)
{
    ssize_t offset = 0;

    ttlet header = make_placement_ptr<header_buf_t>(bytes, offset);

    if (!(
        header->magic[0] == 'T' &&
        header->magic[1] == 'T' &&
        header->magic[2] == 'I' &&
        header->magic[3] == 'C'
    )) {
        TTAURI_THROW(parse_error("Expected 'TTIC' magic in header"));
    }

    ttlet nr_paths = int{header->nr_paths.value()};

    auto drawing = Path{};
    for (int i = 0; i < nr_paths; i++) {
        ttlet layer = parsePath(bytes, offset);

        if (layer.fillColor.a() > 0.001) {
            drawing.addPath(layer.path, layer.fillColor);
        }
        if (layer.strokeColor.a() > 0.001) {
            drawing.addStroke(layer.path, layer.strokeColor, layer.strokeWidth, layer.lineJoinStyle);
        }
    }

    return drawing;
}

}
