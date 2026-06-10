#pragma once

#include <iostream>
#include <vector>
#include <atomic>
#include "macros.hpp"

namespace Common {
    template<typename T>
    class LockFreeQueue final{
        private:
            std::vector<T> store_;
            alignas(64) std::atomic<size_t> next_write_index_ = {0};
            alignas(64) std::atomic<size_t> next_read_index_ = {0};
            alignas(64) std::atomic<size_t> num_elements_ = {0};
            char padding_[64 - sizeof(std::atomic<size_t>)];

            auto isFull() const noexcept {
                return num_elements_.load() == store_.size();
            }

            auto isEmpty() const noexcept {
                return num_elements_.load() == 0;
            }

       public:
           LockFreeQueue(std::size_t num_elements) : store_(num_elements, T()) {}     

            auto getNextToWriteTo() noexcept {
                return isFull() ? nullptr : &store_[next_write_index_];
            }

            auto updateWriteIndex() noexcept {
                next_write_index_ = (next_write_index_ + 1) % store_.size();
                num_elements_++;
            }

            auto getNextToRead() noexcept {
                return isEmpty() ? nullptr : &store_[next_read_index_];
            }

            auto updateReadIndex() noexcept {
                next_read_index_ = (next_read_index_ + 1) % store_.size();
                ASSERT(num_elements_ != 0, "Read an invalid element in:"+std::to_string(pthread_self()));
                num_elements_--;
            }

            auto size() const noexcept {
                return num_elements_.load();
            }

           LockFreeQueue()=delete;
           LockFreeQueue(const LockFreeQueue&) = delete;
           LockFreeQueue(const LockFreeQueue&&) = delete;
           LockFreeQueue& operator=(const LockFreeQueue&) = delete;
           LockFreeQueue& operator=(const LockFreeQueue&&) = delete;
    };
}