#ifndef USER_H
#define USER_H

#include <string>
#include <unordered_map>
#include "sha1.h" 
using namespace std;

// ─────────────────────────────────────────
// FNV-1a Hash (32-bit)
// Non-cryptographic hash from the DAA spec
// O(n) time, deterministic, collision-resistant
// for short inputs like passwords
// ─────────────────────────────────────────
string hashPassword(const string& password);

struct UserRecord {
    string username;
    string role;      // dispatcher / hospital / firedept / police
    string unitName;  // HQ / HospitalA / HospitalB / FireStationA / PolicePrecinct1
    string password;  // stored as 40-char SHA-1 hex string

    UserRecord() {}
    UserRecord(string username, string role, string unitName, string password);
};

class UserManager {
public:
    // unordered_map = hash table → O(1) avg lookup (your DAA hashing point)
    unordered_map<string, UserRecord> users;

    UserManager();

    void loadFromFile(string filename);
    void saveToFile(string filename);

    // Returns {role, unitName} on success, {"",""} on failure
    pair<string,string> login(string username, string password);

    // unitName added — which hospital/station this user belongs to
    bool registerUser(string username, string role,
                      string unitName, string password);

    bool exists(string username);
    void displayAll();
};

#endif