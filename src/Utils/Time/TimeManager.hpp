/*
** EPITECH PROJECT, 2025
** GameEngine
** File description:
** TimeManager - Generic time management abstraction
*/

#pragma once

#include <chrono>
#include <cstdint>

namespace GameEngine
{
    /**
     * @brief Generic time manager that provides timing functionality
     * without depending on specific libraries like SDL
     */
    class TimeManager {
      public:
        TimeManager() = default;
        ~TimeManager() = default;

        /**
         * @brief Get the current time in milliseconds since epoch
         * @return Time in milliseconds
         */
        static std::uint64_t getTimeMs();

        /**
         * @brief Get the current time in nanoseconds since epoch
         * @return Time in nanoseconds
         */
        static std::uint64_t getTimeNs();

        /**
         * @brief Sleep for a specified duration
         * @param milliseconds Duration to sleep in milliseconds
         */
        static void delay(std::uint32_t milliseconds);

        /**
         * @brief Get elapsed time since last call to this function
         * @return Elapsed time in seconds as float
         */
        static float getDeltaTime();

      private:
        static std::chrono::steady_clock::time_point _lastTime;
    };
} // namespace GameEngine
