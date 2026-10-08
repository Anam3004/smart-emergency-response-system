#include "sha1.h"
#include <sstream>
#include <iomanip>
using namespace std;

// ─────────────────────────────────────────
// leftRotate
// Rotates a 32-bit integer left by 'bits' positions
// Used in the compression function every round
//
// Example: leftRotate(0b1000...0001, 1)
//        → 0b0000...0011
// The bits that fall off the left wrap to the right.
// This is what creates the "avalanche effect" in SHA-1
// ─────────────────────────────────────────
static uint32_t leftRotate(uint32_t val, int bits) {
    return (val << bits) | (val >> (32 - bits));
}

// ─────────────────────────────────────────
// sha1
// Full SHA-1 implementation per FIPS 180-4 spec
// Input  : any string (password in our case)
// Output : 40-character lowercase hex string
// ─────────────────────────────────────────
string sha1(const string& input) {

    // ── Step 1: Initial hash values ─────
    // These are the first 32 bits of the fractional
    // parts of the square roots of 2, 3, 5, 10
    // (chosen to have no special mathematical advantage
    //  — "nothing up my sleeve" numbers)
    uint32_t h0 = 0x67452301;
    uint32_t h1 = 0xEFCDAB89;
    uint32_t h2 = 0x98BADCFE;
    uint32_t h3 = 0x10325476;
    uint32_t h4 = 0xC3D2E1F0;

    // ── Step 2: Pre-processing (padding) ─
    // Goal: make the total message length
    //       a multiple of 512 bits (64 bytes)

    // Copy input into a byte vector
    vector<uint8_t> msg(input.begin(), input.end());

    // Remember original bit length for the padding footer
    uint64_t originalBitLen = (uint64_t)msg.size() * 8;

    // Append the '1' bit (as 0x80 byte — the rest of
    // this byte is '0' bits, which is fine)
    msg.push_back(0x80);

    // Append '0' bytes until length ≡ 56 (mod 64)
    // We need 8 bytes at the end for the length footer
    // so we pad to 56 bytes before the next 64-byte boundary
    while (msg.size() % 64 != 56) {
        msg.push_back(0x00);
    }

    // Append original bit length as 64-bit BIG-endian integer
    // Most significant byte first
    for (int i = 7; i >= 0; i--) {
        msg.push_back((uint8_t)((originalBitLen >> (i * 8)) & 0xFF));
    }

    // ── Step 3: Process each 512-bit chunk ─
    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {

        // ── Message schedule: w[0..79] ───
        // w[0..15]  = the 16 words from this chunk
        // w[16..79] = derived by XOR + left-rotate

        uint32_t w[80];

        // Load 16 words (each word = 4 bytes, big-endian)
        for (int j = 0; j < 16; j++) {
            w[j] = ((uint32_t)msg[chunk + j*4    ] << 24)
                 | ((uint32_t)msg[chunk + j*4 + 1] << 16)
                 | ((uint32_t)msg[chunk + j*4 + 2] <<  8)
                 | ((uint32_t)msg[chunk + j*4 + 3]      );
        }

        // Extend to 80 words
        // Each new word is XOR of 4 earlier words, then rotated left by 1
        // This is what "diffuses" the input bits through the schedule
        for (int j = 16; j < 80; j++) {
            w[j] = leftRotate(w[j-3] ^ w[j-8] ^ w[j-14] ^ w[j-16], 1);
        }

        // ── Initialise working variables ─
        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

        // ── 80 rounds of compression ─────
        // Each round mixes a, b, c, d, e with a word from
        // the message schedule and a round constant K
        for (int j = 0; j < 80; j++) {
            uint32_t f, k;

            if (j < 20) {
                // Choice function: if B then C else D
                f = (b & c) | (~b & d);
                k = 0x5A827999;   // floor(sqrt(2) * 2^30)
            } else if (j < 40) {
                // Parity function: XOR of all three
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;   // floor(sqrt(3) * 2^30)
            } else if (j < 60) {
                // Majority function: majority vote of B, C, D
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;   // floor(sqrt(5) * 2^30)
            } else {
                // Parity again
                f = b ^ c ^ d;
                k = 0xCA62C1D6;   // floor(sqrt(10) * 2^30)
            }

            // Core round operation
            uint32_t temp = leftRotate(a, 5) + f + e + k + w[j];
            e = d;
            d = c;
            c = leftRotate(b, 30);  // rotate B by 30
            b = a;
            a = temp;
        }

        // ── Add compressed chunk to hash ─
        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;
        h4 += e;
    }

    // ── Step 4: Produce final hash ───────
    // Concatenate h0–h4 as zero-padded 8-char hex each
    // Total = 5 × 8 = 40 characters
    ostringstream ss;
    ss << hex << setfill('0')
       << setw(8) << h0
       << setw(8) << h1
       << setw(8) << h2
       << setw(8) << h3
       << setw(8) << h4;

    return ss.str();
    // e.g. "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d"
}