#include "vdf_computer.h"

#include <sstream>

int gcd_base_bits = 50;
int gcd_128_max_iter = 3;

#include "verifier.h"
#include "create_discriminant.h"

std::ostream& operator<<(std::ostream& os, __mpz_struct const* z) { return os; }

namespace vdf {

namespace types {

Integer::Integer(integer const& val) { val_.reset(new integer(val)); }

Integer::Integer(std::string const& str) { val_.reset(new integer(str)); }

integer Integer::Get_integer() const { return *val_; }

std::string Integer::FormatString() const {
    std::stringstream ss;
    ss << to_string(val_->impl);
    return ss.str();
}

}  // namespace types

namespace utils {

types::Integer CreateDiscriminant(Bytes const& challenge, int disc_size) {
    return types::Integer(::CreateDiscriminant(const_cast<Bytes&>(challenge), disc_size));
}

bool VerifyProof(
    types::Integer const& D_int, Bytes const& proof_data, uint64_t iters, uint8_t depth, Bytes const& x_data) {
    static const uint8_t DEFAULT_ELEMENT[1] = {0x08};
    integer D = D_int.Get_integer();
    return CheckProofOfTimeNWesolowski(
        D, x_data.empty() ? DEFAULT_ELEMENT : x_data.data(), proof_data.data(), proof_data.size(), iters,
        DEFAULT_DISC_SIZE, depth);
}

Bytes SerializeProof(types::Proof const& proof) { return ConnectBytes(proof.y, proof.proof); }

} // namespace utils

int FORM_SIZE() { return BQFC_FORM_SIZE; }

}  // namespace vdf
