/* Copyright 2007-2009 Brad King, Chuck Stewart
   Distributed under the Boost Software License, Version 1.0.
   (See accompanying file rtvl_license_1_0.txt or copy at
   http://www.boost.org/LICENSE_1_0.txt) */
#ifndef rtvl_votee_d_hxx
#define rtvl_votee_d_hxx

#include "rtvl_votee.hxx"

template <class T, unsigned int n>
class vnl_vector_fixed;
template <class T, unsigned int nr, unsigned int nc>
class vnl_matrix_fixed;

template <unsigned int N>
class rtvl_votee_d : public rtvl_votee<N>
{
public:
  typedef rtvl_votee<N> derived;
  rtvl_votee_d(vnl_vector_fixed<double, N> const& votee_location,
    vnl_matrix_fixed<double, N, N>& votee_tensor,
    vnl_matrix_fixed<double, N, N> (&votee_dtensor)[N]);
  void go(rtvl_vote_internal<N>& vi, double saliency) override;

private:
  vnl_matrix_fixed<double, N, N> (&dtensor_)[N];
};

#endif
