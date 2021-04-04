#pragma once
#include <cstddef>
#include <type_traits>

namespace _1D
{
/** @class
 * @brief A class that implements Simposon's (1/3) rule.
 * @author C.D. Clark III
 */
template<typename T, std::size_t NN = 0>
class SimpsonRule
{
 public:
  SimpsonRule() = default;

  // This version will integrate a callable between two points
  template<typename F, std::size_t NN_ = NN,
           typename SFINAE = typename std::enable_if<(NN_ == 0)>::type>
  T operator()(F f, T a, T b, std::size_t N) const;

  template<typename F, std::size_t NN_ = NN,
           typename SFINAE = typename std::enable_if<(NN_ > 0)>::type>
  T operator()(F f, T a, T b) const;

  /**
   * This version will integrate a discretized function with the x and y
   * values held in separate containers.
   *
   * @param x a vector-like container with operator[](int) and .size()
   * @param y a vector-like container with operator[](int)
   *
   * x and y must have the same size *and* contain 3 or more elements,
   * othersize the call is undefined behavior.
   */
  template<typename X, typename Y>
  auto operator()(const X &x, const Y &y) const -> decltype(x.size(),x[0],y[0],T())
  {
    T sum = 0;
    decltype(x.size()) i;
    for (i = 0; i < x.size() - 2; i += 2) {
      // Integrate segment using three points
      // \int_a^b f(x) dx = (b-a)/6 [ f(a) + 4*f(m) + f(b) ]
      // a = x[i]
      // b = x[i+2]
      // m = (a+b)/2
      //
      // f(a) = y[i]
      // f(b) = y[i+2]
      // and we need to interpolate f(m)
      // clang-format off
      T m = (x[i] + x[i+2])/2;
      T ym = y[i  ]*LagrangePolynomial(m, x[i+1], x[i+2], x[i  ])
           + y[i+1]*LagrangePolynomial(m, x[i  ], x[i+2], x[i+1])
           + y[i+2]*LagrangePolynomial(m, x[i  ], x[i+1], x[i+2]);

      sum += (x[i+2]-x[i])/6 * (y[i] + 4*ym + y[i+2]);
      // clang-format on
    }

    if( x.size() % 2 == 0) // note: this tests if the last *index* is odd. the size will be even.
    {
      if( x.size() > 2)
      {
      // there is on extra point at the end we need to handle
      // we will use the last *three* points to fit the polynomial
      // but then integrate between the last *two* points.
      i = x.size()-3;
      T m = (x[i+1] + x[i+2])/2;
      T ym = y[i  ]*LagrangePolynomial(m, x[i+1], x[i+2], x[i  ])
           + y[i+1]*LagrangePolynomial(m, x[i  ], x[i+2], x[i+1])
           + y[i+2]*LagrangePolynomial(m, x[i  ], x[i+1], x[i+2]);

      sum += (x[i+2]-x[i+1])/6 * (y[i+1] + 4*ym + y[i+2]);
      }
    }

    return sum;
  }

  /**
   * This version will integrate a uniformly discretized function with y
   * values held in a container.
   *
   * @param y a vector-like container with operator[](int) and .size()
   * @param dx a the spacing between consecutive function values.
   *
   * y must contain 3 or more elements, othersize the call is undefined behavior.
   */
  template<typename Y>
  auto operator()(const Y &y, T dx = 1) const -> decltype(y.size(),dx*y[0],T())
  {
    T sum = 0;
    decltype(y.size()) i;

    for(i = 0; i < y.size()-2; i+=2)
    {
      sum += y[i] + 4*y[i+1] + y[i+2];
    }
    sum *= dx/3.;

    // if the container size is even, there will be one extra interval we need to handle
    if( y.size() % 2 == 0)
    {
      // use the last three points to interpolate
      // the function to the midpoint of the last two
      // points. Then apply Simpson's rule
      // clang format off
      i = y.size()-3;
      T ym = y[i  ]*LagrangePolynomial(3.*dx/2 , dx , 2*dx , 0  )
           + y[i+1]*LagrangePolynomial(3.*dx/2 , 0  , 2*dx , dx )
           + y[i+2]*LagrangePolynomial(3.*dx/2 , 0  , dx   , 2*dx);

      sum += dx/6 * (y[i+1] + 4*ym + y[i+2]);
      // clang format on
    }


    return sum;
  }


 protected:
  T LagrangePolynomial(T x, T A, T B, T C) const;
};

template<typename T, std::size_t NN>
template<typename F, std::size_t NN_, typename SFINAE>
T SimpsonRule<T, NN>::operator()(F f, T a, T b, std::size_t N) const
{
  T sum = 0;
  T dx  = static_cast<T>(b - a) / N;  // size of each interval
  T x = a;
  for (std::size_t i = 0; i < N; i++, x += dx) {
    sum += f(x) + 4 * f(x + dx / 2) + f(x + dx);
  }
  // note that 2*h = dx
  sum *= dx / 6;
  return sum;
}

template<typename T, std::size_t NN>
template<typename F, std::size_t NN_, typename SFINAE>
T SimpsonRule<T, NN>::operator()(F f, T a, T b) const
{
  T sum = 0;
  T dx  = static_cast<T>(b - a) / NN;  // size of each interval
  T x = a;
  for (std::size_t i = 0; i < NN; i++, x += dx) {
    sum += f(x) + 4 * f(x + dx / 2) + f(x + dx);
  }
  // note that 2*h = dx
  sum *= dx / 6;
  return sum;
}
template<typename T, std::size_t NN>
T SimpsonRule<T, NN>::LagrangePolynomial(T x, T A, T B, T C) const
{
  return (x - A) * (x - B) / (C - A) / (C - B);
}

}  // namespace _1D
