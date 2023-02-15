#ifndef VDF_COMPUTER_H
#define VDF_COMPUTER_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "vdf_types.h"
#include "vdf_utils.h"

namespace vdf {

int const DEFAULT_DISC_SIZE = 1024;

namespace types {

class Integer {
    std::shared_ptr<integer> val_;

public:
    explicit Integer(integer const&);

    explicit Integer(std::string const& str);

    integer Get_integer() const;

    std::string FormatString() const;
};

struct Proof {
    Bytes y;
    Bytes proof;
    uint8_t witness_type{0};
};

} // namespace types

namespace utils {

types::Integer CreateDiscriminant(Bytes const& challenge, int disc_size = DEFAULT_DISC_SIZE);

bool VerifyProof(types::Integer const& D, Bytes const& proof, uint64_t iters, uint8_t depth = 0, Bytes const& x = {});

Bytes SerializeProof(types::Proof const& proof);

} // namespace utils

int FORM_SIZE();

}  // namespace vdf

#endif
