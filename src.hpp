// Header-only implementation for Pokedex problem (ACMOJ 1277)
#pragma once

#include <bits/stdc++.h>

class BasicException {
protected:
    std::string msg;
public:
    explicit BasicException(const char *_message) : msg(_message ? _message : "") {}
    explicit BasicException(const std::string &_message) : msg(_message) {}
    virtual const char *what() const { return msg.c_str(); }
};

class ArgumentException: public BasicException {
public:
    explicit ArgumentException(const std::string &m) : BasicException(m) {}
};

class IteratorException: public BasicException {
public:
    explicit IteratorException(const std::string &m) : BasicException(m) {}
};

struct Pokemon {
    // name and id as required public members
    char name[12];
    int id;
    // internal: list of types (string identifiers)
    std::vector<std::string> types;
};

class Pokedex {
private:
    std::string filename;
    // map by id for stable ordering and fast lookup
    std::map<int, Pokemon> mp;
    // name index to enforce uniqueness
    std::unordered_map<std::string, int> name_to_id;

    static bool is_alpha_str(const char *s) {
        if (!s || *s == '\0') return false;
        for (const char *p = s; *p; ++p) {
            if (!std::isalpha(static_cast<unsigned char>(*p))) return false;
        }
        return true;
    }

    static std::vector<std::string> split_types(const std::string &types) {
        std::vector<std::string> res;
        std::string cur;
        for (char c : types) {
            if (c == '#') {
                if (!cur.empty()) res.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        if (!cur.empty()) res.push_back(cur);
        return res;
    }

    static const std::unordered_set<std::string>& valid_types() {
        static const std::unordered_set<std::string> t = {
            "fire", "water", "grass", "electric", "ground", "flying", "dragon"
        };
        return t;
    }

    static double type_effect_once(const std::string &atk, const std::string &def) {
        // Define effectiveness for the 7-type subset
        if (atk == "fire") {
            if (def == "grass") return 2.0;
            if (def == "fire" || def == "water" || def == "dragon") return 0.5;
            return 1.0;
        } else if (atk == "water") {
            if (def == "fire" || def == "ground") return 2.0;
            if (def == "water" || def == "grass" || def == "dragon") return 0.5;
            return 1.0;
        } else if (atk == "grass") {
            if (def == "water" || def == "ground") return 2.0;
            if (def == "fire" || def == "grass" || def == "flying" || def == "dragon") return 0.5;
            return 1.0;
        } else if (atk == "electric") {
            if (def == "water" || def == "flying") return 2.0;
            if (def == "electric" || def == "grass" || def == "dragon") return 0.5;
            if (def == "ground") return 0.0;
            return 1.0;
        } else if (atk == "ground") {
            if (def == "fire" || def == "electric") return 2.0;
            if (def == "grass") return 0.5;
            if (def == "flying") return 0.0;
            return 1.0;
        } else if (atk == "flying") {
            if (def == "grass") return 2.0;
            if (def == "electric") return 0.5;
            return 1.0;
        } else if (atk == "dragon") {
            if (def == "dragon") return 2.0;
            return 1.0;
        }
        return 1.0;
    }

    static bool validate_types_or_throw(const std::vector<std::string> &types) {
        const auto &vt = valid_types();
        for (const auto &t : types) {
            if (!vt.count(t)) {
                throw ArgumentException(std::string("Argument Error: PM Type Invalid (") + t + ")");
            }
        }
        return true;
    }

    void load_from_file() {
        mp.clear();
        name_to_id.clear();
        std::ifstream fin(filename);
        if (!fin.is_open()) {
            // create file lazily on save
            return;
        }
        std::string line;
        while (std::getline(fin, line)) {
            if (line.empty()) continue;
            // format: name|id|type1#type2#...
            std::string name, idstr, types_str;
            size_t p1 = line.find('|');
            size_t p2 = (p1 == std::string::npos) ? std::string::npos : line.find('|', p1 + 1);
            if (p1 == std::string::npos || p2 == std::string::npos) continue;
            name = line.substr(0, p1);
            idstr = line.substr(p1 + 1, p2 - (p1 + 1));
            types_str = line.substr(p2 + 1);
            int id = 0;
            try { id = std::stoi(idstr); } catch (...) { continue; }
            Pokemon pm{};
            pm.id = id;
            std::snprintf(pm.name, sizeof(pm.name), "%s", name.c_str());
            pm.types = split_types(types_str);
            mp[id] = pm;
            name_to_id[name] = id;
        }
    }

    void save_to_file() const {
        std::ofstream fout(filename, std::ios::trunc);
        if (!fout.is_open()) return;
        for (const auto &kv : mp) {
            const Pokemon &pm = kv.second;
            std::string types_str;
            for (size_t i = 0; i < pm.types.size(); ++i) {
                if (i) types_str.push_back('#');
                types_str += pm.types[i];
            }
            fout << pm.name << '|' << pm.id << '|' << types_str << '\n';
        }
    }

public:
    explicit Pokedex(const char *_fileName) : filename(_fileName ? _fileName : "pokedex.txt") {
        load_from_file();
    }

    ~Pokedex() {
        save_to_file();
    }

    bool pokeAdd(const char *name, int id, const char *types) {
        if (!is_alpha_str(name)) {
            throw ArgumentException(std::string("Argument Error: PM Name Invalid (") + (name ? name : "") + ")");
        }
        if (!types) {
            throw ArgumentException("Argument Error: PM Type Invalid ()");
        }
        std::string types_s(types);
        auto vec = split_types(types_s);
        if (vec.empty()) {
            throw ArgumentException("Argument Error: PM Type Invalid ()");
        }
        validate_types_or_throw(vec);
        // no exception: proceed
        // reject duplicate id or name
        if (mp.count(id)) return false;
        if (name_to_id.count(name)) return false;
        Pokemon pm{};
        pm.id = id;
        std::snprintf(pm.name, sizeof(pm.name), "%s", name);
        pm.types = vec;
        mp[id] = pm;
        name_to_id[std::string(name)] = id;
        return true;
    }

    bool pokeDel(int id) {
        auto it = mp.find(id);
        if (it == mp.end()) return false;
        name_to_id.erase(std::string(it->second.name));
        mp.erase(it);
        return true;
    }

    std::string pokeFind(int id) const {
        auto it = mp.find(id);
        if (it == mp.end()) return std::string("None");
        return std::string(it->second.name);
    }

    std::string typeFind(const char *types) const {
        if (!types) {
            throw ArgumentException("Argument Error: PM Type Invalid ()");
        }
        auto query = split_types(std::string(types));
        if (query.empty()) {
            throw ArgumentException("Argument Error: PM Type Invalid ()");
        }
        validate_types_or_throw(query);
        // match pokemons whose types include ANY in query (OR semantics)
        std::vector<const Pokemon*> matches;
        for (const auto &kv : mp) {
            const Pokemon &pm = kv.second;
            bool ok_any = false;
            for (const auto &t : query) {
                for (const auto &pt : pm.types) if (pt == t) { ok_any = true; break; }
                if (ok_any) break;
            }
            if (ok_any) matches.push_back(&pm);
        }
        if (matches.empty()) return std::string("None");
        std::ostringstream oss;
        oss << matches.size() << '\n';
        // matches are already in id ascending by map iter
        for (const auto *p : matches) {
            oss << p->name << '\n';
        }
        std::string out = oss.str();
        if (!out.empty() && out.back() == '\n') out.pop_back();
        return out;
    }

    float attack(const char *type, int id) const {
        auto it = mp.find(id);
        if (it == mp.end()) return -1.0f;
        if (!type) {
            throw ArgumentException("Argument Error: PM Type Invalid ()");
        }
        std::string atk(type);
        const auto &vt = valid_types();
        if (!vt.count(atk)) {
            throw ArgumentException(std::string("Argument Error: PM Type Invalid (") + atk + ")");
        }
        double m = 1.0;
        for (const auto &def : it->second.types) {
            double e = type_effect_once(atk, def);
            if (e == 0.0) return 0.0f;
            m *= e;
        }
        return static_cast<float>(m);
    }

    int catchTry() const {
        if (mp.empty()) return 0;
        // Owned set initialized with smallest id
        std::set<int> owned;
        owned.insert(mp.begin()->first);
        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto &kv : mp) {
                int pid = kv.first;
                if (owned.count(pid)) continue;
                const Pokemon &target = kv.second;
                bool capturable = false;
                // Try each owned pokemon and each of its types to attack target
                for (int oid : owned) {
                    const Pokemon &opm = mp.find(oid)->second;
                    for (const auto &atk : opm.types) {
                        double dmg = 1.0;
                        for (const auto &def : target.types) {
                            double e = type_effect_once(atk, def);
                            if (e == 0.0) { dmg = 0.0; break; }
                            dmg *= e;
                        }
                        if (dmg >= 2.0 - 1e-9) { capturable = true; break; }
                    }
                    if (capturable) break;
                }
                if (capturable) { owned.insert(pid); changed = true; }
            }
        }
        return static_cast<int>(owned.size());
    }

    struct iterator {
        using OuterMap = std::map<int, Pokemon>;
        OuterMap *cont = nullptr;
        typename OuterMap::iterator it;

        iterator() = default;
        iterator(OuterMap *c, typename OuterMap::iterator i) : cont(c), it(i) {}

        iterator &operator++() {
            if (!cont) throw IteratorException("Iterator Error");
            if (it == cont->end()) throw IteratorException("Iterator Error");
            ++it; return *this;
        }
        iterator &operator--() {
            if (!cont) throw IteratorException("Iterator Error");
            if (it == cont->begin()) throw IteratorException("Iterator Error");
            --it; return *this;
        }
        iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }
        iterator operator--(int) { iterator tmp = *this; --(*this); return tmp; }
        iterator & operator = (const iterator &rhs) { cont = rhs.cont; it = rhs.it; return *this; }
        bool operator == (const iterator &rhs) const { return cont == rhs.cont && it == rhs.it; }
        bool operator != (const iterator &rhs) const { return !(*this == rhs); }
        Pokemon & operator*() const {
            if (!cont || it == cont->end()) throw IteratorException("Iterator Error");
            return it->second;
        }
        Pokemon *operator->() const {
            if (!cont || it == cont->end()) throw IteratorException("Iterator Error");
            return &(it->second);
        }
    };

    iterator begin() { return iterator(&mp, mp.begin()); }
    iterator end() { return iterator(&mp, mp.end()); }
};
