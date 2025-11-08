// Entity.hpp - Version améliorée et optimisée
#pragma once
#include "game/Offsets.hpp"
#include "Structs.hpp"
#include "Memory/Interface.hpp"
#include "globals.hpp"
#include <unordered_map>

class EntityManager {
private:
    // Cache avec timeout pour éviter les lectures répétées
    struct CachedEntity {
        uint64_t address;
        uint64_t timestamp;
        bool valid;
    };

    static constexpr size_t MAX_ENTITIES = 100;
    static constexpr uint64_t CACHE_DURATION_MS = 500; // Réduit pour plus de réactivité

    std::unordered_map<int, CachedEntity> cache;
    uint64_t lastCleanup = 0;

    // Nettoie le cache périodiquement
    void CleanupCache() {
        const uint64_t now = GetTickCount64();
        if (now - lastCleanup < 5000) return; // Cleanup toutes les 5s

        for (auto it = cache.begin(); it != cache.end();) {
            if (!it->second.valid || (now - it->second.timestamp) > CACHE_DURATION_MS * 10) {
                it = cache.erase(it);
            }
            else {
                ++it;
            }
        }
        lastCleanup = now;
    }

    // Lecture depuis la mémoire avec validation
    uint64_t ReadFromMemory(int id, uint64_t base) {
        if (id < 0 || id >= 100) return 0;

        const uint64_t entityList = base + OFF_ENTITY_LIST;
        const uint64_t entityAddr = I::Read<uint64_t>(entityList + (static_cast<uint64_t>(id) << 5));

        // Validation basique de l'adresse
        if (entityAddr < 0x1000 || entityAddr == 0xFFFFFFFFFFFFFFFF) {
            return 0;
        }

        return entityAddr;
    }

public:
    static EntityManager& Get() {
        static EntityManager instance;
        return instance;
    }

    // Récupère une entité avec système de cache
    uint64_t GetEntity(int id, uint64_t base) {
        if (id < 0 || id >= MAX_ENTITIES) {
            return ReadFromMemory(id, base);
        }

        const uint64_t now = GetTickCount64();

        // Vérifier le cache
        auto it = cache.find(id);
        if (it != cache.end() && it->second.valid) {
            if ((now - it->second.timestamp) < CACHE_DURATION_MS) {
                return it->second.address;
            }
        }

        // Lire depuis la mémoire et mettre en cache
        const uint64_t addr = ReadFromMemory(id, base);
        cache[id] = { addr, now, addr != 0 };

        CleanupCache();
        return addr;
    }

    // Invalide le cache (utile lors de changements de round)
    void Reset() {
        cache.clear();
        lastCleanup = 0;
    }

    // Précharge plusieurs entités (optimisation)
    void Preload(int start, int count, uint64_t base) {
        for (int i = start; i < start + count && i < MAX_ENTITIES; ++i) {
            GetEntity(i, base);
        }
    }
};

// Fonction helper globale
inline uint64_t GetEntityById(int id, uint64_t base) {
    return EntityManager::Get().GetEntity(id, base);
}