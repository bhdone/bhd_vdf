#ifndef VDF_UTILS_H
#define VDF_UTILS_H

#include <string>

#include "vdf_types.h"

struct integer;

namespace vdf {

namespace utils {

Bytes ConnectBytes(Bytes const& lhs, Bytes const& rhs);

Bytes BytesFromStr(std::string const& str);

Bytes GetDefaultForm();

} // namespace utils
} // namespace vdf

#endif
