/*
 *	Copyright (C) 2016  Hannes Haberl
 *
 *	This file is part of GLMViz.
 *
 *	GLMViz is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	GLMViz is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with GLMViz.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Buffer.hpp"
#include <algorithm>
#include <array>
#include <type_traits>

template <typename T> Buffer<T>::Buffer(const size_t size) {
  static_assert(std::is_arithmetic<T>::value,
                "Buffer<T> only supports arithmetic types!");

  v_buffer.resize(size);
  this->size = size;
  new_data = true;
}

template <typename T> std::unique_lock<std::mutex> Buffer<T>::lock() {
  return std::unique_lock<std::mutex>(m);
}

// ceiling division
template <typename T> static inline T ceil_div(const T x, const T y) {
  return x / y + (x % y != 0);
}

template <typename T> inline void Buffer<T>::i_write(T buf[], const size_t n) {
  // move old data
  std::copy(v_buffer.begin() + n, v_buffer.end(), v_buffer.begin());
  // append new data
  std::copy(buf, buf + n, v_buffer.end() - n);
}

template <typename T>
inline void Buffer<T>::i_write(const std::vector<T> &buf, const size_t n) {
  // move old data
  std::copy(v_buffer.begin() + n, v_buffer.end(), v_buffer.begin());
  // append new data
  std::copy(buf.begin(), buf.begin() + n, v_buffer.end() - n);
}

template <typename T> void Buffer<T>::write(T buf[], const size_t n) {
  auto lock = this->lock();
  new_data = true;

  // limit data to write
  size_t length = std::min(n, size);
  i_write(buf, length);
}

template <typename T> void Buffer<T>::write(const std::vector<T> &buf) {
  auto lock = this->lock();
  new_data = true;

  // limit data to write
  size_t length = std::min(buf.size(), size);
  i_write(buf, length);
}

template <typename T>
void Buffer<T>::write_offset(T buf[], const size_t n, const size_t gap,
                             const size_t offset) {
  auto lock = this->lock();
  new_data = true;

  // limit data to write
  size_t length = std::min(ceil_div(n - offset, gap), size);
  size_t current = offset;

  // resize intermediate buffer
  if (ibuf.size() < length)
    ibuf.resize(length);
  for (size_t i = 0; i < length; i++) {
    ibuf[i] = buf[current];
    current += gap;
  }
  i_write(ibuf, length);
}

template <typename T>
void Buffer<T>::write_offset(const std::vector<T> &buf, const size_t gap,
                             const size_t offset) {
  auto lock = this->lock();
  new_data = true;

  // limit data to write
  size_t length = std::min(ceil_div(buf.size() - offset, gap), size);
  size_t current = offset;

  // resize intermediate buffer
  if (ibuf.size() < length)
    ibuf.resize(length);
  for (size_t i = 0; i < length; i++) {
    ibuf[i] = buf[current];
    current += gap;
  }
  i_write(ibuf, length);
}

template <typename T> void Buffer<T>::resize(const size_t n) {
  auto lock = this->lock();
  if (size != n) {
    size = n;
    v_buffer.resize(n);
    new_data = true;
  }
}

// calculate the rms value of all the audio data in the buffer
template <typename T> float Buffer<T>::rms() {
  auto lock = this->lock();
  float rms = 0;
  unsigned i = 0;

  constexpr size_t N = 4;
  // temp sum vector
  alignas(16) std::array<float, N> vrms;
  for (size_t k = 0; k < N; k++)
    vrms[k] = 0;

  for (; i < size / N; i++) {
    // load 4 integers and convert them to float
    alignas(16) std::array<float, N> vdata;
    for (size_t k = 0; k < N; k++)
      vdata[k] = (float)v_buffer[i * N + k];

#pragma omp simd
    for (size_t k = 0; k < N; k++)
      vrms[k] += vdata[k] * vdata[k];
  }
  // calculate sum
  for (size_t k = 0; k < N; k++)
    rms += vrms[k];

  // calculate remaining values
  i *= N;
  for (; i < size; i++) {
    rms += v_buffer[i] * v_buffer[i];
  }
  return rms;
}
