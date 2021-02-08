// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../tagged_id.hpp"

namespace tt {

struct font_family_id_tag {};

using font_family_id = tagged_id<uint16_t, font_family_id_tag>;

}