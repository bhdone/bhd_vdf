#include <gtest/gtest.h>

#include "picosha2.hpp"
#include "verifier.hpp"

#include "vdf_types.h"
#include "vdf_utils.h"

char const* SZ_CHALLENGE = "cc5ac4c68e9228f2487aa3d4a0ca067e150ad19f85934f5d97f4355c8c83fdbd";
char const* SZ_PLOT_ID = "4d3567ee9e16629611ffde36d23d8f183cbc01c60eb6a8edeec1c6257a1b68cf";
char const* SZ_PLOT_PROOF = "407f849c3b8fa9265751f34a72b57192cca83a5d7d7d2ce935cfde94e91ffa7567dadbe0cdd36e9da11c5ffd6b790b4acbe64a91d6e4c2f87b4e0b3f7d130222a3196fe705bbebf47817062f3deea06ea3c71dec4198ceaaa1f7fdad81e616c465bf4e8506a088ccd3ace16f1c0bdf9a9c73edcddc1cf0dcfacd8ef574809c442c9f8ffbd92defb3f520b27de1ae949201d63f618514af50994014f5a522bd5b67f6430fa927bda70c39b751c0a9a4a0a864889ed8202aecb283a708378002c5a6cf5f19fe05b31c";

Bytes RevertBytes(Bytes const& bytes) {
    Bytes res;
    res.reserve(bytes.size());
    std::copy(bytes.rbegin(), bytes.rend(), std::back_inserter(res));
    return res;
}

TEST(Verifier, VerifyPoS) {
    Bytes challenge = RevertBytes(vdf::utils::BytesFromStr(SZ_CHALLENGE));
    Bytes id = vdf::utils::BytesFromStr(SZ_PLOT_ID);
    Bytes proof = vdf::utils::BytesFromStr(SZ_PLOT_PROOF);
    Verifier verifier;
    LargeBits quality_string_bits = verifier.ValidateProof(id.data(), 25, challenge.data(), proof.data(), proof.size());
    EXPECT_NE(quality_string_bits.GetSize(), 0);
}
