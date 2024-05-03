#pragma once

#include <cassert>

template <typename T>
class MultiArray {
private:
    T* data_;

public:
    unsigned int* dimension_sizes;
    unsigned int array_size;
    unsigned int capacity = 1;

public:
    MultiArray(const std::vector<unsigned int>& sizes) {
        assert(sizes.size() != 0);

        array_size = sizes.size();
        dimension_sizes = new unsigned int[array_size];


        for (int i = 0; i < array_size; ++i) {
            dimension_sizes[i] = sizes[i];
            capacity *= sizes[i];
        }

        data_ = new T[capacity]{0};
    }

    T* Data() {
        return data_;
    }

    T& operator()(const std::vector<int>& indexes) {
        assert(indexes.size() == array_size);

        for (int i = 0; i < indexes.size(); ++i) {
            assert(0 <= indexes[i] && indexes[i] < dimension_sizes[i]);
        }

        int index = 0;

        for (int i = 0; i < indexes.size(); ++i) {
            index = index * dimension_sizes[i] + indexes[i];
        }

        return data_[index];
    }

    ~MultiArray() {
        delete[] dimension_sizes;
        delete[] data_;
    }
};