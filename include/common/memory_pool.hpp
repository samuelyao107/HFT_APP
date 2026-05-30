#pragma once

#include <thread>
#include <vector>
#include <string>
#include <cstddef>
#include "macros.hpp"

template<typename T>
class MemoryPool final {
    private:
        struct ObjectBlock {
            /**
             * le C++ ajoute automatiquement du padding et gère l'alignement... mais uniquement quand il sait ce qu'il manipule !

                C'est ici qu'intervient le "piège" du std::byte. En utilisant std::byte, on a rendu le compilateur complètement aveugle.
             */
            alignas(T) std::byte raw_memory_[sizeof(T)]; // Mémoire brute alignée pour stocker un objet de type T
            bool is_free_ = true;
        };
        
        std::vector<ObjectBlock> pool_;
        size_t next_free_index_ = 0;
        
        // NOUVEAU : Compteur global de cases libres
        size_t free_blocks_count_; 

        auto updateNextFreeIndex() noexcept {
            // Cette boucle est maintenant 100% sécurisée. 
            // On ne l'appelle QUE si on sait qu'il reste de la place.
            while(!pool_[next_free_index_].is_free_) {
                next_free_index_++;
                if(next_free_index_ == pool_.size()) [[unlikely]] {
                    next_free_index_ = 0;
                }
            }
        }
        
    public:
        // Initialisation du compteur
        explicit MemoryPool(std::size_t pool_size) : pool_(pool_size), free_blocks_count_(pool_size) { 
            ASSERT(reinterpret_cast<const ObjectBlock*>(&(pool_[0].raw_memory_)) == &(pool_[0]),
            "Memory layout is not as expected");
        }

        template<typename... Args>
        T* allocate(Args&&... args) noexcept {
            // FIX TEST 9 : On crashe ICI seulement si on demande une case de trop
            ASSERT(free_blocks_count_ > 0, "Memory pool is full");
            
            auto obj_block = &(pool_[next_free_index_]);
            
            T* ret = reinterpret_cast<T*>(obj_block->raw_memory_);
            ret = new(ret) T(std::forward<Args>(args)...);   
            
            obj_block->is_free_ = false; 
            free_blocks_count_--; // On décrémente
            
            // On ne cherche la prochaine case que s'il en reste au moins une
            if (free_blocks_count_ > 0) {
                updateNextFreeIndex();
            }
            
            return ret;
        }

        auto deallocate(T* obj) noexcept {
            auto element_index = reinterpret_cast<const ObjectBlock*>(obj) - &(pool_[0]);
            
            ASSERT(element_index >= 0 && static_cast<size_t>(element_index) < pool_.size(), "Pointer does not belong to this memory pool");
            ASSERT(!pool_[element_index].is_free_, "Double free detected");
            
            obj->~T();
            pool_[element_index].is_free_ = true;
            
            free_blocks_count_++; // On remet une case dispo
            
            // FIX TEST 7 : Réutilisation immédiate ! 
            // La prochaine allocation se fera exactement sur cette case chaude en cache.
            next_free_index_ = element_index; 
        }

        MemoryPool() = delete;
        MemoryPool(const MemoryPool&) = delete;
        MemoryPool(MemoryPool&&) = delete;
        MemoryPool& operator=(const MemoryPool&) = delete;
        MemoryPool& operator=(MemoryPool&&) = delete;
};