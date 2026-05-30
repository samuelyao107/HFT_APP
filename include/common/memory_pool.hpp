#pragma once

#include <thread>
#include <vector>
#include <string>
#include "macros.hpp"

template<typename T>
class MemoryPool final{
    private:
        struct ObjectBlock{
            T object_;
            bool is_free_ = true;
        };
        std::vector<ObjectBlock> pool_;
        size_t next_free_index_ = 0;

        auto updateNextFreeIndex() noexcept {
            const auto initial_free_index = next_free_index_;
            while(!pool_[next_free_index_].is_free_){
                next_free_index_ ++;
               if(next_free_index_ == pool_.size()) [[unlikely]] {
                next_free_index_ = 0;
               }
               if(next_free_index_ == initial_free_index) [[unlikely]]{
                ASSERT(next_free_index_ != initial_free_index, "Memory pool is full, no free blocks available");
               }
            }
            
        }
    public:
        explicit MemoryPool(std::size_t pool_size): pool_(pool_size, {T(), true}){ //We use explicit here to prevent implicit(ghost) conversions
            
            ASSERT(reinterpret_cast<const ObjectBlock*>
            (&(pool_[0].object_))== &(pool_[0]),
            "Memory layout is not as expected, object_ should be the first member of ObjectBlock");
        }

        template<typename... Args>
        T* allocate(Args&&... args) noexcept {
            auto obj_block = &(pool_[next_free_index_]);
            ASSERT(obj_block->is_free_, "Expected a free block, but found a used one at index "
                 + std::to_string(next_free_index_));
            T* ret = &(obj_block->object_);
            ret = new(ret) T(args...);    
            updateNextFreeIndex();
            return ret;
        }

        auto deallocate(const T* obj) noexcept {
            //ObjectBlock and T are layout-compatible, we can safely cast the pointer to ObjectBlock and calculate the index by pointer arithmetic (same address)
            //Index = (Adresse Cible - Adresse de Départ) / Taille d'un Bloc
            //reinterpret_cast sert à réinterpréter brutalement les bits d’une valeur comme s’ils appartenaient à un autre type, sans aucune conversion.
           auto element_index = reinterpret_cast<const ObjectBlock*>(obj) - &(pool_[0]);//cpp uses padding, the cpu only reads memory in units of bytes, so we can treat the memory as a byte array and calculate the index by pointer arithmetic
            ASSERT(element_index >= 0 && static_cast<size_t>(element_index) < pool_.size(), "Pointer does not belong to this memory pool");
            ASSERT(!pool_[element_index].is_free_, "Double free detected at index " + std::to_string(element_index));
            pool_[element_index].is_free_ = true;
            obj->~T();
        }

        MemoryPool() = delete;
        MemoryPool(const MemoryPool&) = delete;
        MemoryPool(const MemoryPool&&) = delete;
        MemoryPool& operator=(const MemoryPool&) = delete;
        MemoryPool& operator=(const MemoryPool&&) = delete;
};