#ifndef SHA1_H
#define SHA1_H

#include <string>
#include <vector>
#include <cstdint>
using namespace std;

// ─────────────────────────────────────────
// SHA-1 — Secure Hash Algorithm 1
//
// Produces a 40-character hex string (160-bit hash)
// from any input string.
//
// How it works (explain this to your teacher):
//
//  1. PADDING
//     Append bit '1', then zeros, then original
//     message length as 64-bit big-endian integer
//     until total length ≡ 0 (mod 512 bits)
//
//  2. CHUNK PROCESSING
//     Break padded message into 512-bit (64-byte) chunks
//     Each chunk goes through 80 rounds of mixing
//
//  3. MESSAGE SCHEDULE
//     Expand 16 words (from the chunk) into 80 words
//     using XOR + left-rotate operations
//
//  4. COMPRESSION FUNCTION (80 rounds)
//     Each round applies a different nonlinear function F:
//       Round  0-19 : F = (B AND C) OR (NOT B AND D)
//       Round 20-39 : F = B XOR C XOR D
//       Round 40-59 : F = (B AND C) OR (B AND D) OR (C AND D)
//       Round 60-79 : F = B XOR C XOR D
//     + adds a round constant K (4 different values)
//
//  5. OUTPUT
//     After all chunks, concatenate h0–h4 as 8-char hex each
//     → 40-character final hash string
//
// Time complexity : O(n) where n = input length
// Output length   : always 40 hex characters (160 bits)
// Collision resist: 2^80 operations to find a collision
// ─────────────────────────────────────────
string sha1(const string& input);

// Convenience wrapper — same name as before
// so user.cpp calls hashPassword() unchanged
inline string hashPassword(const string& password) {
    return sha1(password);
}

#endif