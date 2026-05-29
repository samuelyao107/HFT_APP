#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <numeric>
#include "common/lock_free_queue.hpp" 
#include "common/macros.hpp"


namespace Common {
namespace test {

    // --- 1. TESTS FONCTIONNELS (MONOTHREAD) ---

    TEST(LockFreeQueueTest, InitialState) {
        LockFreeQueue<int> queue(10);
        EXPECT_EQ(queue.size(), 0);
        EXPECT_EQ(queue.getNextToRead(), nullptr);
    }

    TEST(LockFreeQueueTest, PushAndPopSingleElement) {
        LockFreeQueue<int> queue(5);

        // Écriture
        int* write_ptr = queue.getNextToWriteTo();
        ASSERT_NE(write_ptr, nullptr);
        *write_ptr = 42;
        queue.updateWriteIndex();

        EXPECT_EQ(queue.size(), 1);

        // Lecture
        int* read_ptr = queue.getNextToRead();
        ASSERT_NE(read_ptr, nullptr);
        EXPECT_EQ(*read_ptr, 42);
        
        queue.updateReadIndex();
        EXPECT_EQ(queue.size(), 0);
        EXPECT_EQ(queue.getNextToRead(), nullptr); // La file doit être vide à nouveau
    }

    TEST(LockFreeQueueTest, CircularBehaviorAndOverwrite) {
        // Test de la logique du modulo (%)
        LockFreeQueue<int> queue(2);

        // Remplissage complet
        *queue.getNextToWriteTo() = 100;
        queue.updateWriteIndex();
        *queue.getNextToWriteTo() = 200;
        queue.updateWriteIndex();

        EXPECT_EQ(queue.size(), 2);

        // On consomme le premier élément (index 0)
        EXPECT_EQ(*queue.getNextToRead(), 100);
        queue.updateReadIndex();

        // L'index d'écriture doit être revenu au début (index 0) grâce au modulo
        *queue.getNextToWriteTo() = 300;
        queue.updateWriteIndex();

        // On vérifie l'ordre (FIFO : d'abord 200, puis 300)
        EXPECT_EQ(*queue.getNextToRead(), 200);
        queue.updateReadIndex();
        EXPECT_EQ(*queue.getNextToRead(), 300);
        queue.updateReadIndex();

        EXPECT_EQ(queue.size(), 0);
    }


    // --- 2. TEST DE CONCURRENCE (SPSC MULTITHREAD) ---

    TEST(LockFreeQueueTest, ConcurrentProducerConsumerSPSC) {
        const size_t queue_capacity = 1024;
        LockFreeQueue<int> queue(queue_capacity);
        
        const int num_elements_to_send = 100'000; // Nombre d'éléments pour le stress test
        std::vector<int> received_data;
        received_data.reserve(num_elements_to_send);

        // Thread Producteur (Écriture)
        std::thread producer([&queue, num_elements_to_send]() {
            for (int i = 0; i < num_elements_to_send; ++i) {
                int* ptr = nullptr;
                
                // Boucle d'attente active (Spinlock) si la file est temporairement pleine
                // Dans ton implémentation, la file est pleine si size() == capacité
                while (queue.size() >= queue_capacity) {
                    std::this_thread::yield(); // Donne la main à un autre thread pour éviter de saturer le CPU inutilement
                }

                ptr = queue.getNextToWriteTo();
                *ptr = i;
                queue.updateWriteIndex();
            }
        });

        // Thread Consommateur (Lecture)
        std::thread consumer([&queue, num_elements_to_send, &received_data]() {
            int items_read = 0;
            while (items_read < num_elements_to_send) {
                int* ptr = queue.getNextToRead();
                
                if (ptr != nullptr) {
                    received_data.push_back(*ptr);
                    queue.updateReadIndex();
                    items_read++;
                } else {
                    // File temporairement vide, on attend un peu
                    std::this_thread::yield();
                }
            }
        });

        // Attente de la fin des deux threads
        producer.join();
        consumer.join();

        // --- VÉRIFICATIONS ---
        // 1. Est-ce qu'on a bien reçu le bon nombre d'éléments ?
        ASSERT_EQ(received_data.size(), num_elements_to_send);

        // 2. Est-ce que l'ordre FIFO a été strictement respecté et aucune donnée n'a été corrompue ?
        for (int i = 0; i < num_elements_to_send; ++i) {
            ASSERT_EQ(received_data[i], i) << "Erreur de séquence ou corruption à l'index " << i;
        }

        // 3. La file doit être vide à la fin
        EXPECT_EQ(queue.size(), 0);
    }

} // namespace test
} // namespace Common