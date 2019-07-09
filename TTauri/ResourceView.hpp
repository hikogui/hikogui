// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "FileView.hpp"
#include "StaticResourceView.hpp"
#include "Application.hpp"
#include <variant>

namespace TTauri {

using ResourceView_intrinsic = std::variant<FileView,StaticResourceView>;

static inline ResourceView_intrinsic loadView(URL const &location)
{
    if (location.scheme == "static-resource") {
        return StaticResourceView{ location };

    } else if (location.scheme == "resource") {
        return FileView{ get_singleton<Application>().resourceLocation.urlByAppendingPath(location) };

    } else if (location.scheme == "file") {
        return FileView{ location };

    } else {
        BOOST_THROW_EXCEPTION(FileError("Unknown scheme for loading a resource")
            << errinfo_url(location)
        );
    }
}

class ResourceView {
private:
    ResourceView_intrinsic intrinsic;

public:
    ResourceView(URL const &location) : intrinsic(loadView(location)) {}

    ~ResourceView() = default;
    ResourceView(ResourceView const &other) = delete;
    ResourceView(ResourceView &&other) = delete;
    ResourceView &operator=(ResourceView const &other) = delete;
    ResourceView &operator=(ResourceView &&other) = delete;

    size_t offset() const {
        if (std::holds_alternative<FileView>(intrinsic)) {
            let &_view = std::get<FileView>(intrinsic);
            return _view.offset;
        } else {
            return 0;
        }
    }

    gsl::span<std::byte const> bytes() const {
        if (std::holds_alternative<FileView>(intrinsic)) {
            let &_intrinsic = std::get<FileView>(intrinsic);
            return _intrinsic.bytes;
        } else if (std::holds_alternative<StaticResourceView>(intrinsic)) {
            let &_intrinsic = std::get<StaticResourceView>(intrinsic);
            return _intrinsic.bytes;
        } else {
            no_default;
        }
    }

    gsl::span<std::byte> writableBytes() {
        if (std::holds_alternative<FileView>(intrinsic)) {
            let &_intrinsic = std::get<FileView>(intrinsic);
            return _intrinsic.bytes;
        } else {
            no_default;
        }
    }

    size_t size() const {
        return bytes().size();
    }

    void const *data() const {
        return bytes().data();
    }

};

}