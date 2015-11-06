//
// get_associated_allocator.h
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// Helper function to obtain an associated allocator.
//
// Copyright (c) 2014-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef EXPERIMENTAL_BITS_GET_ASSOCIATED_ALLOCATOR_H
#define EXPERIMENTAL_BITS_GET_ASSOCIATED_ALLOCATOR_H

namespace std {
namespace experimental {
namespace net {
inline namespace v1 {

template <class _T>
inline associated_allocator_t<_T> get_associated_allocator(const _T& __t)
{
  return associated_allocator<_T>::get(__t);
}

template <class _T, class _Alloc>
inline associated_allocator_t<_T, _Alloc>
get_associated_allocator(const _T& __t, const _Alloc& __a)
{
  return associated_allocator<_T, _Alloc>::get(__t, __a);
}

} // inline namespace v1
} // namespace net
} // namespace experimental
} // namespace std

#endif
