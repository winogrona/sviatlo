#ifndef __WINOGRONA_NVM_HPP__
#define __WINOGRONA_NVM_HPP__

#include <color.hpp>
#include <vector>

#define NVM_NAMESPACE_NAME "ledcontrol"

namespace nvm {
     bool setup_nvm();
     void save_last_strip_state(std::vector<color::Color> colors);
     std::vector<color::Color> load_last_strip_state();
}

#endif