
#include "FreeType.hpp"

namespace TTauri::Draw {

using namespace std;

std::shared_ptr<FreeType> FreeType::singleton;

FreeType::FreeType()
{
    FT_Init_FreeType(&intrinsic);
}

FreeType::~FreeType()
{
    FT_Done_FreeType(intrinsic);
}


}