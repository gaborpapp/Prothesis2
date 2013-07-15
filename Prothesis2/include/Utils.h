#pragma once

#include <string>
#include <vector>

#include "cinder/Filesystem.h"
#include "cinder/gl/Texture.h"

namespace mndl
{

std::vector< std::pair< std::string, ci::gl::Texture > > loadTextures( const ci::fs::path &relativeDir );

} // namespace mndl

