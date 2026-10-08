#include "user.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

// ─────────────────────────────────────────
// stored as 40-char SHA-1 hex string
// ─────────────────────────────────────────
// string hashPassword(const string& password) {
//     const uint32_t FNV_PRIME        = 16777619u;
//     const uint32_t FNV_OFFSET_BASIS = 2166136261u;

//     uint32_t hash = FNV_OFFSET_BASIS;

//     for (unsigned char c : password) {
//         hash ^= c;          // XOR with byte
//         hash *= FNV_PRIME;  // multiply to spread
//     }

//     // Convert uint32 → zero-padded 8-char hex
//     ostringstream ss;
//     ss << hex << setw(8) << setfill('0') << hash;
//     return ss.str();
// }

UserRecord::UserRecord(string username, string role,
                       string unitName, string password)
    : username(username), role(role),
      unitName(unitName), password(password) {}

UserManager::UserManager() {}

// ─────────────────────────────────────────
// loadFromFile
// New 4-field format:
//   username role unitName hashedPassword
//   alice hospital HospitalA 5f4dcc3b
// ─────────────────────────────────────────
void UserManager::loadFromFile(string filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "No users file — starting fresh." << endl;
        return;
    }

    users.clear();
    string username, role, unitName, password;

    while (file >> username >> role >> unitName >> password) {
        users[username] = UserRecord(username, role, unitName, password);
    }

    file.close();
    cerr << "Loaded " << users.size() << " users." << endl;
}

void UserManager::saveToFile(string filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "ERROR: Cannot write " << filename << endl;
        return;
    }

    for (auto& [u, r] : users) {
        // password field already contains the SHA-1 hash
        // never write plain text passwords
        file << r.username << " "
             << r.role     << " "
             << r.unitName << " "
             << r.password << "\n";
    }
    file.close();
}

// ─────────────────────────────────────────
// login
// 1. O(1) hash map lookup by username
// 2. SHA-1 hash the input password
// 3. Compare against stored SHA-1 hash
//    → never compare plain text passwords
// ─────────────────────────────────────────
pair<string,string> UserManager::login(string username, string password) {
    auto it = users.find(username);   // O(1) hash map lookup

    if (it == users.end()) {
        cerr << "Login failed: user not found." << endl;
        return {"", ""};
    }

    // Hash the incoming password and compare hashes
    // The stored hash was created during registerUser()
    string inputHash = hashPassword(password);  // SHA-1 of input

    if (it->second.password != inputHash) {
        cerr << "Login failed: wrong password." << endl;
        return {"", ""};
    }

    return {it->second.role, it->second.unitName};
}

// ─────────────────────────────────────────
// registerUser
// Hashes password with SHA-1 BEFORE storing
// Plain text password never touches the file
// ─────────────────────────────────────────
bool UserManager::registerUser(string username, string role,
                               string unitName, string password) {
    if (users.count(username)) {
        cerr << "Username '" << username << "' taken." << endl;
        return false;
    }

    // SHA-1 hash password → store hash, discard plain text
    string hashed = hashPassword(password);
    users[username] = UserRecord(username, role, unitName, hashed);

    cerr << "Registered: " << username
         << " | " << role
         << " | " << unitName
         << " | sha1: " << hashed << endl;
    return true;
}

bool UserManager::exists(string username) {
    return users.count(username) > 0;
}

void UserManager::displayAll() {
    cerr << "\n--- Users ---" << endl;
    for (auto& [u, r] : users) {
        cerr << r.username << " | " << r.role
             << " | " << r.unitName
             << " | hash:" << r.password << endl;
    }
}