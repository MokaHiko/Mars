#ifndef RANDOMTOOLBOX_H
#define RANDOMTOOLBOX_H

#pragma once

#include <iostream>

#include <random>
#include <type_traits>
#include <assert.h>

#include <vector>

namespace tbx
{
    // Generates psuedo random values of integer or real types (double, float).
    // By default, the seed is set to the value of random device and the distribution is normalized from 0 to 100
    template <typename T>
    class PRNGenerator
    {
    public:
        // Checks for rand type
        template <typename T>
        using uniform_distribution = typename std::conditional<std::is_floating_point<T>::value, std::uniform_real_distribution<T>,
            typename std::conditional<std::is_integral<T>::value, std::uniform_int_distribution<T>, void>::type>::type;

        ~PRNGenerator() = default;

        PRNGenerator()
        {
            _dist = uniform_distribution<T>(0, 100);
            SetSeed(rand());
        };

        PRNGenerator(T lower, T upper, int32_t seed = rand())
        {
            _dist = uniform_distribution<T>(lower, upper);
            SetSeed(seed);
        };

        // Gets the next value
        const T Next()
        {
            return _dist(eng);
        };

        // Sets the lower and upper bound of the random values
        void SetDistribution(T low, T upper)
        {
            _dist = uniform_distribution<T>(low, upper);
        }

        // Sets the seed of the random engine
        void SetSeed(uint32_t seed)
        {
            eng = std::mt19937{ seed };
        }

    private:
        std::random_device _device;
        std::mt19937 eng{ _device() };
        uniform_distribution<T> _dist;
    };

    // Creates a psuedo random traversal of indices of n elements;
    class PrimeSearch
    {
    public:
        PrimeSearch(int n_elements);

        // Returns index to next element to traverse and -1 if complete
        int GetNext(bool restart = false);

        // Returns how many searches have occured
        inline const int GetSearchCount() const {return _searches;}

        // Restarts traversal back to 0 searches
        void Restart();

        // Returns prime number greater than n and 0 if none found
        static int FindNextPrime(int n);

        // Checks if n is prime
        static int IsPrime(int n);
    private:
        int _searches = 0;
        int _n_elements = 0;

        int _nextMember = 0;
        int _a, _b, _c = 0;
        int _prime = 0;
        int _skip = 0;

        tbx::PRNGenerator<int> _random_engine = { 1, 100 };
    };
}

#endif