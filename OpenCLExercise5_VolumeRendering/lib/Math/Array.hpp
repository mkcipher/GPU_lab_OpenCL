/*
 * Copyright (c) 2013 Steffen Kieß
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MATH_ARRAY_HPP_INCLUDED
#define MATH_ARRAY_HPP_INCLUDED

#include <Core/Util.hpp>
#include <Core/Assert.hpp>
//#include <Core/CheckedInteger.hpp>

#include <Math/Forward.hpp>

#include <boost/array.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_volatile.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/make_shared.hpp>
#include <boost/scoped_array.hpp>
#include <boost/utility/enable_if.hpp>

#if !HAVE_CXX11
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/control/if.hpp>

#include <stdint.h>

#include <limits>
#include <vector>

// Maximum dimension for ArrayView<const >/ArrayView<>/Array<>
#define MATH_ARRAY_MAX_ARRAY_DIM 10
#endif

namespace Math {
  struct Range {
    std::size_t start;
    std::size_t length;
    NVCC_HOST_DEVICE Range (std::size_t start, std::size_t length) : start (start), length (length) {
    }
  };
  struct OpenRange {
    std::size_t start;
    NVCC_HOST_DEVICE OpenRange (std::size_t start) : start (start) {
    }
  };
  NVCC_HOST_DEVICE inline Range range (std::size_t start, std::size_t length) {
    return Range (start, length);
  }
  NVCC_HOST_DEVICE inline OpenRange range (std::size_t start = 0) {
    return OpenRange (start);
  }

  template <>
  struct ArrayAssertions<true> {
    static const bool enabled = true;

    template <std::size_t dpos, std::size_t dim, typename Config>
    NVCC_HOST_DEVICE static inline void check (const ArrayViewBase<dim, Config>& view, std::size_t pos) {
#if !(defined (__CUDACC__) && defined (__CUDA_ARCH__)) // TODO
      ASSERT (pos < view.template size<dpos> ());
#endif
    }

    template <std::size_t dpos, std::size_t dim, typename Config>
    NVCC_HOST_DEVICE static inline void check (const ArrayViewBase<dim, Config>& view, Range range) {
#if !(defined (__CUDACC__) && defined (__CUDA_ARCH__)) // TODO
      ASSERT (range.start + range.length >= range.start);
      ASSERT (range.start + range.length <= view.template size<dpos> ());
#endif
    }

    template <std::size_t dpos, std::size_t dim, typename Config>
    NVCC_HOST_DEVICE static inline void check (const ArrayViewBase<dim, Config>& view, OpenRange range) {
#if !(defined (__CUDACC__) && defined (__CUDA_ARCH__)) // TODO
      ASSERT (range.start <= view.template size<dpos> ());
#endif
    }
  };
  template <>
  struct ArrayAssertions<false> {
    static const bool enabled = false;

    template <std::size_t dpos, std::size_t dim, typename Config>
    static inline void check (UNUSED const ArrayViewBase<dim, Config>& view, UNUSED std::size_t pos) {
    }

    template <std::size_t dpos, std::size_t dim, typename Config>
    static inline void check (UNUSED const ArrayViewBase<dim, Config>& view, UNUSED Range range) {
    }

    template <std::size_t dpos, std::size_t dim, typename Config>
    static inline void check (UNUSED const ArrayViewBase<dim, Config>& view, UNUSED OpenRange range) {
    }
  };

  // struct ArrayConfig is in Forward.hpp because of default template parameter for ArrayView
  template <typename T> struct ArrayConfig::Type {
    typedef T* Pointer;

    NVCC_HOST_DEVICE static inline ArithmeticPointer toArith (T* ptr) {
      return const_cast<char*> (reinterpret_cast<const volatile char*> (ptr));
    }

    NVCC_HOST_DEVICE static inline T* fromArith (ArithmeticPointer ptr) {
      return reinterpret_cast<T*> (ptr);
    }

    template <typename Config>
    static inline boost::shared_ptr<ArrayAllocator<Config, T> > getDefaultAllocator () {
      return boost::make_shared<DefaultArrayAllocator<Config, T> > ();
    }
  };

  template <typename Config, typename T> class ArrayAllocator {
  public:
    class Handle {
    public:
#define WORKAROUND_OCC_HANDLE_MACRO
      virtual ~Handle WORKAROUND_OCC_HANDLE_MACRO () {}
#undef WORKAROUND_OCC_HANDLE_MACRO
      virtual typename Config::ArithmeticPointer getPointer () = 0;
    };

    virtual ~ArrayAllocator () {}
    virtual std::size_t calculateStrides (std::size_t dim, std::ptrdiff_t* strides, const std::size_t* shape, bool fortranOrdering) const = 0;
    virtual boost::shared_ptr<Handle> allocate (std::size_t count) const = 0;
  };

  template <typename Config, typename T> class DefaultArrayAllocator : public ArrayAllocator<Config, T> {
    class MyHandle : public ArrayAllocator<Config, T>::Handle {
      boost::scoped_array<T> data;

    public:
      MyHandle (std::size_t count) : data (new T [count]) {}

      virtual ~MyHandle () {}
      virtual typename Config::ArithmeticPointer getPointer () {
        return Config::template Type<T>::toArith (data.get ());
      }
    };

  public:
    virtual ~DefaultArrayAllocator () {}
    virtual std::size_t calculateStrides (std::size_t dim, std::ptrdiff_t* strides, const std::size_t* shape, bool fortranOrdering) const {
      //Core::CheckedInteger<std::size_t> elements = 1;
      std::size_t elements = 1;
      for (std::size_t i = fortranOrdering ? 0 : dim - 1; i < dim; fortranOrdering ? i++ : i--) {
        std::size_t size = shape[i];
        ASSERT (sizeof (T) == 0 || std::numeric_limits<std::size_t>::max () / sizeof (T) >= elements);
        strides[i] = elements * sizeof (T);
        ASSERT (size == 0 || std::numeric_limits<std::size_t>::max () / size >= elements);
        elements *= size;
      }
      return elements;
    }
    virtual boost::shared_ptr<typename ArrayAllocator<Config, T>::Handle> allocate (std::size_t count) const {
      if (!count)
        return boost::shared_ptr<typename ArrayAllocator<Config, T>::Handle> ();
      return boost::make_shared<MyHandle> (count);
    }
  };

  namespace Intern {
    inline boost::false_type OpPArgInfoIsRange (UNUSED std::size_t size) {
      return boost::false_type ();
    }
    inline boost::true_type OpPArgInfoIsRange (UNUSED Range r) {
      return boost::true_type ();
    }
    inline boost::true_type OpPArgInfoIsRange (UNUSED OpenRange r) {
      return boost::true_type ();
    }

#if HAVE_CXX11
    template <typename... T> struct PackLen;
    template <typename T1, typename... T> struct PackLen<T1, T...> {
      static const std::size_t value = PackLen<T...>::value + 1;
    };
    template <> struct PackLen<> {
      static const std::size_t value = 0;
    };

    struct OpPSetup {
      template <std::size_t dim, typename Config, typename Assert, std::size_t dpos, std::size_t rDim, typename... ParT>
      NVCC_HOST_DEVICE static void setup (const ArrayViewBase<dim, Config>& view, typename Config::ArithmeticPointer* ptr, size_t* shape, ptrdiff_t* stridesBytes, size_t val, ParT... v) {
        Assert::template check<dpos> (view, val);
        *ptr += val * view.template strideBytes<dpos> ();
        setup<dim, Config, Assert, dpos + 1, rDim> (view, ptr, shape, stridesBytes, v...);
      }

      template <std::size_t dim, typename Config, typename Assert, std::size_t dpos, std::size_t rDim, typename... ParT>
      NVCC_HOST_DEVICE static void setup (const ArrayViewBase<dim, Config>& view, typename Config::ArithmeticPointer* ptr, size_t* shape, ptrdiff_t* stridesBytes, Range val, ParT... v) {
        BOOST_STATIC_ASSERT (rDim > 0);
        size_t start = val.start;
        size_t length = val.length;
        Assert::template check<dpos> (view, val);
        *ptr += start * view.template strideBytes<dpos> ();
        shape[0] = length;
        stridesBytes[0] = view.template strideBytes<dpos> ();
        setup<dim, Config, Assert, dpos + 1, rDim - 1> (view, ptr, shape + 1, stridesBytes + 1, v...);
      }

      template <std::size_t dim, typename Config, typename Assert, std::size_t dpos, std::size_t rDim, typename... ParT>
      NVCC_HOST_DEVICE static void setup (const ArrayViewBase<dim, Config>& view, typename Config::ArithmeticPointer* ptr, size_t* shape, ptrdiff_t* stridesBytes, OpenRange val, ParT... v) {
        BOOST_STATIC_ASSERT (rDim > 0);
        size_t start = val.start;
        Assert::template check<dpos> (view, val);
        size_t length = view.template size<dpos> () - start;
        *ptr += start * view.template strideBytes<dpos> ();
        shape[0] = length;
        stridesBytes[0] = view.template strideBytes<dpos> ();
        setup<dim, Config, Assert, dpos + 1, rDim - 1> (view, ptr, shape + 1, stridesBytes + 1, v...);
      }

      template <std::size_t dim, typename Config, typename Assert, std::size_t dpos, std::size_t rDim>
      NVCC_HOST_DEVICE static void setup (UNUSED const ArrayViewBase<dim, Config>& view, UNUSED typename Config::ArithmeticPointer* ptr, UNUSED size_t* shape, UNUSED ptrdiff_t* stridesBytes) {
        BOOST_STATIC_ASSERT (rDim == 0);
      }
    };

    template <typename T, std::size_t retDim, typename Config, typename Assert> struct OpPInfoDim {
      typedef ArrayView<T, retDim, Config, Assert> RetType;
      template <std::size_t dim, typename... ParT>
      NVCC_HOST_DEVICE static RetType access (const ArrayView<T, dim, Config, Assert>& view, ParT... v) {
        BOOST_STATIC_ASSERT ((PackLen<ParT...>::value == dim));
        size_t shape[retDim];
        ptrdiff_t stridesBytes[retDim];
        typename Config::ArithmeticPointer ptr = view.arithData ();
        OpPSetup::setup<dim, Config, Assert, 0, retDim> (view, &ptr, shape, stridesBytes, v...);
        return ArrayView<T, retDim, Config, Assert> (Config::template Type<T>::fromArith (ptr), shape, stridesBytes);
      }
    };
    template <typename T, typename Config, typename Assert> struct OpPInfoDim<T, 0, Config, Assert> {
      typedef typename Config::template Type<T>::Pointer PtrRetType;
      template <std::size_t dim, typename... ParT>
      NVCC_HOST_DEVICE static PtrRetType accessPtr (const ArrayView<T, dim, Config, Assert>& view, ParT... v) {
        BOOST_STATIC_ASSERT ((PackLen<ParT...>::value == dim));
        typename Config::ArithmeticPointer ptr = view.arithData ();
        OpPSetup::setup<dim, Config, Assert, 0, 0> (view, &ptr, NULL, NULL, v...);
        return Config::template Type<T>::fromArith (ptr);
      }
      typedef T& RetType;
      template <std::size_t dim, typename... ParT>
      NVCC_HOST_DEVICE static RetType access (const ArrayView<T, dim, Config, Assert>& view, ParT... v) {
        return *accessPtr<dim, ParT...> (view, v...);
      }
    };

    template <typename T, typename Config, typename Assert, typename... ParT> struct OpPInfo;
    template <typename T, typename Config, typename Assert, typename Par1, typename... ParT>
    struct OpPInfo<T, Config, Assert, Par1, ParT...> {
      typedef DECLTYPE(OpPArgInfoIsRange (*(Par1*)NULL)) Par1IsRange;
      static const std::size_t retDim = OpPInfo<T, Config, Assert, ParT...>::retDim + ((Par1IsRange::value) ? 1 : 0);

      typedef OpPInfoDim<T, retDim, Config, Assert> Info;
    };

    template <typename T, typename Config, typename Assert>
    struct OpPInfo<T, Config, Assert> {
      static const std::size_t retDim = 0;

      typedef OpPInfoDim<T, retDim, Config, Assert> Info;
    };
#else
    struct OpPSetup {
#define DEFINE_OVERLOADS(n)                                             \
      template <std::size_t dim, typename Config, typename Assert, std::size_t dpos, std::size_t rDim  BOOST_PP_ENUM_TRAILING_PARAMS (n, typename ParT)> \
      NVCC_HOST_DEVICE static void setup (const ArrayViewBase<dim, Config>& view, typename Config::ArithmeticPointer* ptr, size_t* shape, ptrdiff_t* stridesBytes, size_t val  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const ParT, & v)) { \
        Assert::template check<dpos> (view, val);                                \
        *ptr += val * view.template strideBytes<dpos> ();               \
        setup<dim, Config, Assert, dpos + 1, rDim> (view, ptr, shape, stridesBytes  BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
      }                                                                 \
                                                                        \
      template <std::size_t dim, typename Config, typename Assert, std::size_t dpos, std::size_t rDim  BOOST_PP_ENUM_TRAILING_PARAMS (n, typename ParT)> \
      NVCC_HOST_DEVICE static void setup (const ArrayViewBase<dim, Config>& view, typename Config::ArithmeticPointer* ptr, size_t* shape, ptrdiff_t* stridesBytes, Range val  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const ParT, & v)) { \
        BOOST_STATIC_ASSERT (rDim > 0);                                 \
        size_t start = val.start;                                       \
        size_t length = val.length;                                     \
        Assert::template check<dpos> (view, val);                                \
        *ptr += start * view.template strideBytes<dpos> ();             \
        shape[0] = length;                                              \
        stridesBytes[0] = view.template strideBytes<dpos> ();           \
        setup<dim, Config, Assert, dpos + 1, rDim - 1> (view, ptr, shape + 1, stridesBytes + 1  BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
      }                                                                 \
                                                                        \
      template <std::size_t dim, typename Config, typename Assert, std::size_t dpos, std::size_t rDim  BOOST_PP_ENUM_TRAILING_PARAMS (n, typename ParT)> \
      NVCC_HOST_DEVICE static void setup (const ArrayViewBase<dim, Config>& view, typename Config::ArithmeticPointer* ptr, size_t* shape, ptrdiff_t* stridesBytes, OpenRange val  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const ParT, & v)) { \
        BOOST_STATIC_ASSERT (rDim > 0);                                 \
        size_t start = val.start;                                       \
        Assert::template check<dpos> (view, val);                                \
        size_t length = view.template size<dpos> () - start;            \
        *ptr += start * view.template strideBytes<dpos> ();             \
        shape[0] = length;                                              \
        stridesBytes[0] = view.template strideBytes<dpos> ();           \
        setup<dim, Config, Assert, dpos + 1, rDim - 1> (view, ptr, shape + 1, stridesBytes + 1  BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
      }                                                                 \

#define DEFINE_OVERLOADS2(z, i, data) DEFINE_OVERLOADS (i)
      BOOST_PP_REPEAT (MATH_ARRAY_MAX_ARRAY_DIM, DEFINE_OVERLOADS2, NOTHING)
#undef DEFINE_OVERLOADS2
#undef DEFINE_OVERLOADS

      template <std::size_t dim, typename Config, typename Assert, std::size_t dpos, std::size_t rDim>
      NVCC_HOST_DEVICE static void setup (UNUSED const ArrayViewBase<dim, Config>& view, UNUSED typename Config::ArithmeticPointer* ptr, UNUSED size_t* shape, UNUSED ptrdiff_t* stridesBytes) {
        BOOST_STATIC_ASSERT (rDim == 0);
      }
    };

    template <typename T, std::size_t retDim, typename Config, typename Assert> struct OpPInfoDim {
      typedef ArrayView<T, retDim, Config, Assert> RetType;
#define DEFINE_OVERLOADS(n)                                             \
      template <std::size_t dim  BOOST_PP_ENUM_TRAILING_PARAMS (n, typename ParT)> \
      NVCC_HOST_DEVICE static RetType access (const ArrayView<T, dim, Config, Assert>& view  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const ParT, & v)) { \
        BOOST_STATIC_ASSERT ((n == dim));                               \
        size_t shape[retDim];                                           \
        ptrdiff_t stridesBytes[retDim];                                          \
        typename Config::ArithmeticPointer ptr = view.arithData ();                         \
        OpPSetup::setup<dim, Config, Assert, 0, retDim> (view, &ptr, shape, stridesBytes  BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
        return ArrayView<T, retDim, Config, Assert> (Config::template Type<T>::fromArith (ptr), shape, stridesBytes); \
      }                                                                 \

#define DEFINE_OVERLOADS2(z, i, data) DEFINE_OVERLOADS (BOOST_PP_ADD(i, 1))
      BOOST_PP_REPEAT (MATH_ARRAY_MAX_ARRAY_DIM, DEFINE_OVERLOADS2, NOTHING)
#undef DEFINE_OVERLOADS2
#undef DEFINE_OVERLOADS
    };                                                                  \

    template <typename T, typename Config, typename Assert> struct OpPInfoDim<T, 0, Config, Assert> {
      typedef typename Config::template Type<T>::Pointer PtrRetType;
      typedef T& RetType;
#define DEFINE_OVERLOADS(n)                                             \
      template <std::size_t dim  BOOST_PP_ENUM_TRAILING_PARAMS (n, typename ParT)> \
      NVCC_HOST_DEVICE static PtrRetType accessPtr (const ArrayView<T, dim, Config, Assert>& view  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const ParT, & v)) { \
        BOOST_STATIC_ASSERT ((n == dim));                               \
        typename Config::ArithmeticPointer ptr = view.arithData ();     \
        OpPSetup::setup<dim, Config, Assert, 0, 0> (view, &ptr, NULL, NULL  BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
        return Config::template Type<T>::fromArith (ptr);              \
      }                                                                 \
      template <std::size_t dim  BOOST_PP_ENUM_TRAILING_PARAMS (n, typename ParT)> \
      NVCC_HOST_DEVICE static RetType access (const ArrayView<T, dim, Config, Assert>& view  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const ParT, & v)) { \
        return *accessPtr<dim> (view  BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
      }                                                                 \

#define DEFINE_OVERLOADS2(z, i, data) DEFINE_OVERLOADS (BOOST_PP_ADD(i, 1))
      BOOST_PP_REPEAT (MATH_ARRAY_MAX_ARRAY_DIM, DEFINE_OVERLOADS2, NOTHING)
#undef DEFINE_OVERLOADS2
#undef DEFINE_OVERLOADS
    };

    template <typename T, typename Config, typename Assert>
    struct OpPInfo_0 {
      static const std::size_t retDim = 0;

      typedef OpPInfoDim<T, retDim, Config, Assert> Info;
    };

#define OpPInfo_n(n) OpPInfo_##n
#define OpPInfo_n_x(n) OpPInfo_n (n)
#define OpPInfo_n_plus_1(n) OpPInfo_n_x (BOOST_PP_ADD (n, 1))

#define DEFINE_OVERLOADS(n)                                             \
    template <typename T, typename Config, typename Assert, typename Par1  BOOST_PP_ENUM_TRAILING_PARAMS (n, typename ParT)> \
    struct OpPInfo_n_plus_1(n) {                                        \
      typedef DECLTYPE(OpPArgInfoIsRange (*(Par1*)NULL)) Par1IsRange;   \
      static const std::size_t retDim = OpPInfo_##n<T, Config, Assert  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)>::retDim + ((Par1IsRange::value) ? 1 : 0); \
                                                                        \
      typedef OpPInfoDim<T, retDim, Config, Assert> Info;               \
    };                                                                  \

#define DEFINE_OVERLOADS2(z, i, data) DEFINE_OVERLOADS (i)
      BOOST_PP_REPEAT (MATH_ARRAY_MAX_ARRAY_DIM, DEFINE_OVERLOADS2, NOTHING)
#undef DEFINE_OVERLOADS2
#undef DEFINE_OVERLOADS
#undef OpPInfo_n
#undef OpPInfo_n_x
#undef OpPInfo_n_plus_1
#endif

    template <typename T, std::size_t dim, typename Config, typename Assert> struct ArrayElementAccess {
      BOOST_STATIC_ASSERT (dim > 1);

      typedef typename Config::template Type<T>::Pointer Pointer;
      typedef typename Config::template Type<const T>::Pointer ConstPointer;

      typedef ArrayView<T, dim - 1, Config, Assert> Ref;
      NVCC_HOST_DEVICE static Ref access (const ArrayView<T, dim, Config, Assert>& view, size_t pos) {
        Assert::template check<0> (view, pos);
        typename Config::ArithmeticPointer ptr = view.arithData ();
        ptr = ptr + view.template strideBytes<0> () * pos;
        return ArrayView<T, dim - 1, Config, Assert> (Config::template Type <T>::fromArith (ptr), view.shape () + 1, view.stridesBytes () + 1);
      }

      static void assign (const size_t* shape,
                          const ptrdiff_t* targetStridesB, Pointer target,
                          const ptrdiff_t* sourceStridesB, ConstPointer source) {
        size_t size = shape[dim - 1];
        ptrdiff_t strideBT = targetStridesB[dim - 1];
        ptrdiff_t strideBS = sourceStridesB[dim - 1];
        for (size_t i = 0; i < size; i++)
          ArrayElementAccess<T, dim - 1, Config, Assert>::assign (shape, targetStridesB, Config::template Type<T>::fromArith (Config::template Type<T>::toArith (target) + strideBT * i), sourceStridesB, Config::template Type<const T>::fromArith (Config::template Type<const T>::toArith (source) + strideBS * i));
      }

      static void setTo (const size_t* shape,
                         const ptrdiff_t* stridesBytes, Pointer target,
                         T value) {
        size_t size = shape[dim - 1];
        ptrdiff_t strideB = stridesBytes[dim - 1];
        for (size_t i = 0; i < size; i++)
          ArrayElementAccess<T, dim - 1, Config, Assert>::setTo (shape, stridesBytes, Config::template Type<T>::fromArith (Config::template Type<T>::toArith (target) + strideB * i), value);
      }
    };

    template <typename T, typename Config, typename Assert> struct ArrayElementAccess<T, 1, Config, Assert> {
      typedef typename Config::template Type<T>::Pointer Pointer;
      typedef typename Config::template Type<const T>::Pointer ConstPointer;

      typedef T& Ref;
      NVCC_HOST_DEVICE static Ref access (const ArrayView<T, 1, Config, Assert>& view, size_t pos) {
        Assert::template check<0> (view, pos);
        Pointer ptr = view.data ();
        ptr = Config::template Type<T>::fromArith (Config::template Type<T>::toArith (ptr) + view.template strideBytes<0> () * pos);
        return *ptr;
      }

      static void assign (UNUSED const size_t* shape,
                          UNUSED const ptrdiff_t* targetStridesB, Pointer target,
                          UNUSED const ptrdiff_t* sourceStridesB, ConstPointer source) {
        size_t size = shape[0];
        ptrdiff_t strideBT = targetStridesB[0];
        ptrdiff_t strideBS = sourceStridesB[0];
        for (size_t i = 0; i < size; i++)
          *Config::template Type<T>::fromArith(Config::template Type<T>::toArith (target) + strideBT * i) = *Config::template Type<const T>::fromArith (Config::template Type<const T>::toArith (source) + strideBS * i);
      }

      static void setTo (UNUSED const size_t* shape,
                         UNUSED const ptrdiff_t* stridesBytes, Pointer target,
                         T value) {
        size_t size = shape[0];
        ptrdiff_t strideB = stridesBytes[0];
        for (size_t i = 0; i < size; i++)
          *Config::template Type<T>::fromArith (Config::template Type<T>::toArith (target) + strideB * i) = value;
      }
    };
  }

  template <std::size_t dim, typename Config_> class ArrayViewBase {
    BOOST_STATIC_ASSERT (dim > 0);

    template <typename T, std::size_t dim2, typename Config2, typename Assert2> friend class Array;
    template <typename T, std::size_t dim2, typename Config2, typename Assert2> friend class ArrayView;

    ERROR_ATTRIBUTE ("assignment operator not available")
    ArrayViewBase& operator= (const ArrayViewBase& other);

  public:
    typedef Config_ Config;
    typedef typename Config::ArithmeticPointer ArithmeticPointer;

    NVCC_HOST_DEVICE static std::size_t dimension () {
      return dim;
    }

  private:
    ArithmeticPointer ptr;
    //std::size_t shape_[dim];
    boost::array<std::size_t, dim> shape_;
    //std::ptrdiff_t stridesBytes_[dim];
    boost::array<std::ptrdiff_t, dim> stridesBytes_;

    ArrayViewBase () : ptr (0) {
    }

    NVCC_HOST_DEVICE ArrayViewBase (ArithmeticPointer ptr, const std::size_t* shape, const std::ptrdiff_t* stridesBytes) : ptr (ptr) {
      for (size_t i = 0; i < dim; i++) {
        // TODO: boost::array<>::operator[] does not work on Cuda device
        /*
        shape_[i] = shape[i];
        stridesBytes_[i] = stridesBytes[i];
        */
        ((std::ptrdiff_t*)&shape_)[i] = shape[i];
        ((std::size_t*)&stridesBytes_)[i] = stridesBytes[i];
      }
    }

  public:
    // arithData() may return NULL when the array/view is empty
    NVCC_HOST_DEVICE const ArithmeticPointer arithData () const {
      return ptr;
    }
    NVCC_HOST_DEVICE const std::size_t* shape () const {
      //return shape_;
      //return shape_.data ();
      return (const std::size_t*)&shape_; // TODO: boost::array<>::data() does not work on Cuda device
    }
    template <std::size_t n>
    NVCC_HOST_DEVICE std::size_t size () const {
      BOOST_STATIC_ASSERT (n < dim);
      return shape ()[n];
    }
    NVCC_HOST_DEVICE const std::ptrdiff_t* stridesBytes () const {
      //return stridesBytes_;
      //return stridesBytes_.data ();
      return (const std::ptrdiff_t*)&stridesBytes_; // TODO: boost::array<>::data() does not work on Cuda device
    }
    template <std::size_t n>
    NVCC_HOST_DEVICE std::ptrdiff_t strideBytes () const {
      BOOST_STATIC_ASSERT (n < dim);
      return stridesBytes ()[n];
    }
  };

  template <typename T_, std::size_t dim, typename Config_, typename Assert_> class ArrayView<const T_, dim, Config_, Assert_> : public ArrayViewBase<dim, Config_> {
    BOOST_STATIC_ASSERT (!boost::is_const<T_>::value);
    BOOST_STATIC_ASSERT (!boost::is_volatile<T_>::value); // TODO

    friend class ArrayView<T_, dim, Config_, Assert_>;
    friend class Array<T_, dim, Config_, Assert_>;

    typedef ArrayViewBase<dim, Config_> Base;

    ERROR_ATTRIBUTE ("assignment operator not available")
    ArrayView& operator= (const ArrayView& other);

  public:
    typedef Config_ Config;
    typedef Assert_ Assert;
    typedef T_ T;
    typedef const T_ QualifiedT;
    typedef typename Config::template Type<const T> TypeInfo;
    typedef typename TypeInfo::Pointer ConstPointer;
    NVCC_HOST_DEVICE static std::size_t dimension () {
      return dim;
    }

  private:
    ArrayView () {
    }

    class PrivateType {
      friend class ArrayView;
      NVCC_HOST_DEVICE PrivateType () {}
    };

  public:
    NVCC_HOST_DEVICE ArrayView (ConstPointer ptr, const std::size_t* shape, const std::ptrdiff_t* stridesBytes) : Base (TypeInfo::toArith (ptr), shape, stridesBytes) {
    }

    NVCC_HOST_DEVICE ArrayView (ConstPointer ptr, const boost::array<std::size_t, dim>& shape, const boost::array<std::ptrdiff_t, dim>& stridesBytes) : Base (TypeInfo::toArith (ptr), shape.data (), stridesBytes.data ()) {
    }

    // Different Assert parameter
    template <typename Assert2>
    NVCC_HOST_DEVICE ArrayView (const ArrayView<const T, dim, Config, Assert2>& orig, UNUSED typename boost::disable_if<boost::is_same<Assert, Assert2>, PrivateType>::type dummy = PrivateType ()) : Base (orig) {
    }

    // data() may return NULL when the array/view is empty
    NVCC_HOST_DEVICE ConstPointer data () const {
      return TypeInfo::fromArith (Base::arithData ());
    }
    using Base::shape;
    using Base::stridesBytes;

    typedef typename Intern::ArrayElementAccess<const T, dim, Config, Assert>::Ref ConstElementOperatorType;
    NVCC_HOST_DEVICE ConstElementOperatorType operator[] (std::size_t pos) const {
      return Intern::ArrayElementAccess<const T, dim, Config, Assert>::access (*this, pos);
    }

#if HAVE_CXX11
    template <typename... ParT>
    NVCC_HOST_DEVICE typename Intern::OpPInfo<const T, Config, Assert, ParT...>::Info::RetType operator() (ParT... pars) const {
      return Intern::OpPInfo<const T, Config, Assert, ParT...>::Info::template access<dim, ParT...> (*this, pars...);
    }
    template <typename... ParT>
    NVCC_HOST_DEVICE ConstPointer pointer (ParT... pars) const {
      return Intern::OpPInfo<const T, Config, Assert, ParT...>::Info::template accessPtr<dim, ParT...> (*this, pars...);
    }
#else
#define TEMPLATE(n) BOOST_PP_IF (n, template <, public:/*To avoid warnings*/) \
    BOOST_PP_ENUM_PARAMS (n, typename ParT)                             \
    BOOST_PP_IF (n, >, public:/*To avoid warnings*/)
#define DEFINE_OVERLOADS(n)                                             \
    TEMPLATE(n)                                                         \
    NVCC_HOST_DEVICE typename Intern::OpPInfo_##n<const T, Config, Assert  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)>::Info::RetType operator() (BOOST_PP_ENUM_BINARY_PARAMS (n, const ParT, & pars)) const { \
      return Intern::OpPInfo_##n<const T, Config, Assert  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)>::Info::template access<dim  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)> (*this  BOOST_PP_ENUM_TRAILING_PARAMS (n, pars)); \
    }                                                                   \
    TEMPLATE(n)                                                         \
    NVCC_HOST_DEVICE ConstPointer pointer (BOOST_PP_ENUM_BINARY_PARAMS (n, const ParT, & pars)) const { \
      return Intern::OpPInfo_##n<const T, Config, Assert  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)>::Info::template accessPtr<dim  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)> (*this  BOOST_PP_ENUM_TRAILING_PARAMS (n, pars)); \
    }                                                                   \

#define DEFINE_OVERLOADS2(n) DEFINE_OVERLOADS (n)
#define DEFINE_OVERLOADS3(z, i, data) DEFINE_OVERLOADS2 (BOOST_PP_ADD(i, 1))
      BOOST_PP_REPEAT (MATH_ARRAY_MAX_ARRAY_DIM, DEFINE_OVERLOADS3, NOTHING)
#undef DEFINE_OVERLOADS3
#undef DEFINE_OVERLOADS2
#undef DEFINE_OVERLOADS
#undef TEMPLATE
#endif
  };

  template <typename T_, std::size_t dim, typename Config_, typename Assert_> class ArrayView : public ArrayView<const T_, dim, Config_, Assert_> {
    friend class Array<T_, dim, Config_, Assert_>;
    typedef ArrayView<const T_, dim, Config_, Assert_> Base;

    ERROR_ATTRIBUTE ("assignment operator not available")
    ArrayView& operator= (const ArrayView& other);

  public:
    typedef Config_ Config;
    typedef Assert_ Assert;
    typedef T_ T;
    typedef T_ QualifiedT;
    typedef typename Config::template Type<T> TypeInfo;
    typedef typename TypeInfo::Pointer Pointer;
    NVCC_HOST_DEVICE static std::size_t dimension () {
      return dim;
    }

  private:
    ArrayView () {
    }

    class PrivateType {
      friend class ArrayView;
      NVCC_HOST_DEVICE PrivateType () {}
    };

  public:
    NVCC_HOST_DEVICE ArrayView (Pointer ptr, const std::size_t* shape, const std::ptrdiff_t* stridesBytes) : ArrayView<const T, dim, Config, Assert> (ptr, shape, stridesBytes) {
    }

    NVCC_HOST_DEVICE ArrayView (Pointer ptr, const boost::array<std::size_t, dim>& shape, const boost::array<std::ptrdiff_t, dim>& stridesBytes) : Base (ptr, shape, stridesBytes) {
    }

    // Different Assert parameter
    template <typename Assert2>
    NVCC_HOST_DEVICE ArrayView (const ArrayView<T, dim, Config, Assert2>& orig, UNUSED typename boost::disable_if<boost::is_same<Assert, Assert2>, PrivateType>::type dummy = PrivateType ()) : Base (orig) {
    }

    using Base::data;
    // data() may return NULL when the array/view is empty
    NVCC_HOST_DEVICE Pointer data () const {
      return TypeInfo::fromArith (this->arithData ());
    }
    using Base::shape;
    using Base::stridesBytes;

    typedef typename Intern::ArrayElementAccess<const T, dim, Config, Assert>::Ref ConstElementOperatorType;
    typedef typename Intern::ArrayElementAccess<T, dim, Config, Assert>::Ref ElementOperatorType;
    NVCC_HOST_DEVICE ElementOperatorType operator[] (std::size_t pos) const {
      return Intern::ArrayElementAccess<T, dim, Config, Assert>::access (*this, pos);
    }

#if HAVE_CXX11
    template <typename... ParT>
    NVCC_HOST_DEVICE typename Intern::OpPInfo<T, Config, Assert, ParT...>::Info::RetType operator() (ParT... pars) const {
      return Intern::OpPInfo<T, Config, Assert, ParT...>::Info::template access<dim, ParT...> (*this, pars...);
    }
    template <typename... ParT>
    NVCC_HOST_DEVICE Pointer pointer (ParT... pars) const {
      return Intern::OpPInfo<T, Config, Assert, ParT...>::Info::template accessPtr<dim, ParT...> (*this, pars...);
    }
#else
#define TEMPLATE(n) BOOST_PP_IF (n, template <, public:/*To avoid warnings*/) \
    BOOST_PP_ENUM_PARAMS (n, typename ParT)                             \
    BOOST_PP_IF (n, >, public:/*To avoid warnings*/)
#define DEFINE_OVERLOADS(n)                                             \
    TEMPLATE(n)                                                         \
    NVCC_HOST_DEVICE typename Intern::OpPInfo_##n<T, Config, Assert  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)>::Info::RetType operator() (BOOST_PP_ENUM_BINARY_PARAMS (n, const ParT, & pars)) const { \
      return Intern::OpPInfo_##n<T, Config, Assert  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)>::Info::template access<dim  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)> (*this  BOOST_PP_ENUM_TRAILING_PARAMS (n, pars)); \
    }                                                                   \
    TEMPLATE(n)                                                         \
    NVCC_HOST_DEVICE Pointer pointer (BOOST_PP_ENUM_BINARY_PARAMS (n, const ParT, & pars)) const { \
      return Intern::OpPInfo_##n<T, Config, Assert  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)>::Info::template accessPtr<dim  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)> (*this  BOOST_PP_ENUM_TRAILING_PARAMS (n, pars)); \
    }                                                                   \

#define DEFINE_OVERLOADS2(n) DEFINE_OVERLOADS (n)
#define DEFINE_OVERLOADS3(z, i, data) DEFINE_OVERLOADS2 (BOOST_PP_ADD(i, 1))
    BOOST_PP_REPEAT (MATH_ARRAY_MAX_ARRAY_DIM, DEFINE_OVERLOADS3, NOTHING)
#undef DEFINE_OVERLOADS3
#undef DEFINE_OVERLOADS2
#undef DEFINE_OVERLOADS
#undef TEMPLATE
#endif

    void assign (const ArrayView<const T, dim, Config, Assert>& orig) const {
      // TODO: allow disabling assertion?
      for (size_t d = 0; d < dim; d++)
        ASSERT (shape ()[d] == orig.shape ()[d]);
      Intern::ArrayElementAccess<T, dim, Config, Assert>::assign (shape (),
                                                  stridesBytes (), data (),
                                                  orig.stridesBytes (), orig.data ());
    }

    void setTo (T value) const {
      Intern::ArrayElementAccess<T, dim, Config, Assert>::setTo (shape (),
                                                 stridesBytes (), data (),
                                                 value);
    }

    void setToZero () const {
      setTo (T ());
    }
  };

  template <typename T_, std::size_t dim, typename Config_, typename Assert_> class Array {
    BOOST_STATIC_ASSERT (!boost::is_const<T_>::value);
    BOOST_STATIC_ASSERT (!boost::is_volatile<T_>::value); // TODO

    ERROR_ATTRIBUTE ("assignment operator not available")
    Array& operator= (const Array& other);

  public:
    typedef Config_ Config;
    typedef Assert_ Assert;
    typedef T_ T;
    static std::size_t dimension () {
      return dim;
    }

  private:
    typedef ArrayView<T, dim, Config, Assert> View;
    typedef boost::shared_ptr<ArrayAllocator<Config, T> > Allocator;

    Allocator allocator;
    boost::shared_ptr<typename ArrayAllocator<Config, T>::Handle> handle;
    View storage;

    void init (const std::size_t* shape, bool fortranOrdering) {
      storage.ptr = Config::getNull ();
      handle.reset (); // free memory before allocating new array

      std::size_t elements = allocator->calculateStrides (dim, storage.stridesBytes_.data (), shape, fortranOrdering);
      for (std::size_t i = 0; i < dim; i++)
        storage.shape_[i] = shape[i];
      handle = allocator->allocate (elements);
      if (handle)
        storage.ptr = handle->getPointer ();
    }


  public:
    explicit Array (Allocator allocator = Config::template Type<T>::template getDefaultAllocator<Config> ()) : allocator (allocator) {
      size_t zeros[dim] = {0};
      init (zeros, true);
    }

    explicit Array (const std::size_t* shape, bool fortranOrdering = true) : allocator (Config::template Type<T>::template getDefaultAllocator<Config> ()) {
      init (shape, fortranOrdering);
    }

    explicit Array (const boost::array<std::size_t, dim>& shape, bool fortranOrdering = true) : allocator (Config::template Type<T>::template getDefaultAllocator<Config> ()) {
      init (shape.data (), fortranOrdering);
    }

    explicit Array (const Array& orig, bool fortranOrdering = true) : allocator (Config::template Type<T>::template getDefaultAllocator<Config> ()) {
      init (orig.shape (), fortranOrdering);
      assign (orig);
    }

    explicit Array (const ArrayView<const T, dim, Config, Assert>& orig, bool fortranOrdering = true) : allocator (Config::template Type<T>::template getDefaultAllocator<Config> ()) {
      init (orig.shape (), fortranOrdering);
      assign (orig);
    }

    explicit Array (Allocator allocator, const std::size_t* shape, bool fortranOrdering = true) : allocator (allocator) {
      init (shape, fortranOrdering);
    }

    explicit Array (Allocator allocator, const boost::array<std::size_t, dim>& shape, bool fortranOrdering = true) : allocator (allocator) {
      init (shape.data (), fortranOrdering);
    }

    explicit Array (Allocator allocator, const Array& orig, bool fortranOrdering = true) : allocator (allocator) {
      init (orig.shape (), fortranOrdering);
      assign (orig);
    }

    explicit Array (Allocator allocator, const ArrayView<const T, dim, Config, Assert>& orig, bool fortranOrdering = true) : allocator (allocator) {
      init (orig.shape (), fortranOrdering);
      assign (orig);
    }

    void recreate () {
      size_t zeros[dim] = {0};
      init (zeros, true);
    }

    void recreate (const std::size_t* shape, bool fortranOrdering = true) {
      init (shape, fortranOrdering);
    }

    void recreate (const boost::array<std::size_t, dim>& shape, bool fortranOrdering = true) {
      init (shape.data (), fortranOrdering);
    }

    void recreate (const Array& orig, bool fortranOrdering = true) {
      init (orig.shape (), fortranOrdering);
      assign (orig);
    }

    void recreate (const ArrayView<const T, dim, Config, Assert>& orig, bool fortranOrdering = true) {
      init (orig.shape (), fortranOrdering);
      assign (orig);
    }

  private:
    static void getLen (UNUSED std::size_t* shape) {
    }
#if HAVE_CXX11
    template <typename... ParT>
    static void getLen (std::size_t* shape, std::size_t val, ParT... v) {
      shape[0] = val;
      getLen (shape + 1, v...);
    }

  public:
    template <typename... ParT>
    explicit Array (std::size_t val1, ParT... v) : allocator (Config::template Type<T>::template getDefaultAllocator<Config> ()) {
      BOOST_STATIC_ASSERT ((Intern::PackLen<std::size_t, ParT...>::value == dim));
      std::size_t shape[dim];
      getLen (shape, val1, v...);
      init (shape, true);
    }

    template <typename... ParT>
    explicit Array (Allocator allocator, std::size_t val1, ParT... v) : allocator (allocator) {
      BOOST_STATIC_ASSERT ((Intern::PackLen<std::size_t, ParT...>::value == dim));
      std::size_t shape[dim];
      getLen (shape, val1, v...);
      init (shape, true);
    }

    template <typename... ParT>
    void recreate (std::size_t val1, ParT... v) {
      BOOST_STATIC_ASSERT ((Intern::PackLen<std::size_t, ParT...>::value == dim));
      std::size_t shape[dim];
      getLen (shape, val1, v...);
      init (shape, true);
    }
#else
#define TEMPLATEP(n) BOOST_PP_IF (n, template <, private:/*To avoid warnings*/) \
    BOOST_PP_ENUM_PARAMS (n, typename ParT)                             \
    BOOST_PP_IF (n, >, private:/*To avoid warnings*/)
#define TEMPLATE(n) BOOST_PP_IF (n, template <, public:/*To avoid warnings*/) \
    BOOST_PP_ENUM_PARAMS (n, typename ParT)                             \
    BOOST_PP_IF (n, >, public:/*To avoid warnings*/)
#define DEFINE_GETLEN_OVERLOADS(n)                                      \
    private:                                                            \
    TEMPLATEP(n)                                                        \
    static void getLen (std::size_t* shape, std::size_t val  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const ParT, & v)) { \
      shape[0] = val;                                                   \
      getLen (shape + 1  BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
    }                                                                   \

#define DEFINE_OVERLOADS(n)                                             \
  public:                                                               \
  TEMPLATE(n)                                                           \
  explicit Array (std::size_t val1  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const ParT, & v)) : allocator (Config::template Type<T>::template getDefaultAllocator<Config> ()) { \
    BOOST_STATIC_ASSERT (((n + 1) == dim));                             \
    std::size_t shape[dim];                                             \
    getLen (shape, val1  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));         \
    init (shape, true);                                                 \
  }                                                                     \
                                                                        \
  TEMPLATE(n)                                                           \
  explicit Array (Allocator allocator, std::size_t val1  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const ParT, & v)) : allocator (allocator) { \
    BOOST_STATIC_ASSERT (((n + 1) == dim));                             \
    std::size_t shape[dim];                                             \
    getLen (shape, val1  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));         \
    init (shape, true);                                                 \
  }                                                                     \
                                                                        \
  TEMPLATE(n)                                                           \
  void recreate (std::size_t val1  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const ParT, & v)) { \
    BOOST_STATIC_ASSERT (((n + 1) == dim));                             \
    std::size_t shape[dim];                                             \
    getLen (shape, val1  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));         \
    init (shape, true);                                                 \
  }                                                                     \

#define DEFINE_GETLEN_OVERLOADS2(z, i, data) DEFINE_GETLEN_OVERLOADS (i)
#ifndef ARRAY_OMIT_ONEDIM_CTOR_OVERLOADS
#define DEFINE_OVERLOADS2(z, i, data) DEFINE_OVERLOADS (i)
#else
#define DEFINE_OVERLOADS2(z, i, data) DEFINE_OVERLOADS (BOOST_PP_ADD(i, 1))
#endif
      BOOST_PP_REPEAT (MATH_ARRAY_MAX_ARRAY_DIM, DEFINE_GETLEN_OVERLOADS2, NOTHING)
      BOOST_PP_REPEAT (MATH_ARRAY_MAX_ARRAY_DIM, DEFINE_OVERLOADS2, NOTHING)
#undef DEFINE_OVERLOADS2
#undef DEFINE_GETLEN_OVERLOADS2
#undef DEFINE_OVERLOADS
#undef DEFINE_GETLEN_OVERLOADS
#undef TEMPLATE
#undef TEMPLATEP
#endif

    const ArrayView<const T, dim, Config, Assert>& constView () const {
      return storage;
    }

    const ArrayView<const T, dim, Config, Assert>& view () const {
      return storage;
    }
    const ArrayView<T, dim, Config, Assert>& view () {
      return storage;
    } 

    operator const ArrayView<const T, dim, Config, Assert>& () const {
      return view ();
    }
    operator const ArrayView<T, dim, Config, Assert>& () {
      return view ();
    }

    // data() may return NULL when the array/view is empty
    typename View::ConstPointer data () const {
      return view ().data ();
    }
    typename View::Pointer data () {
      return view ().data ();
    }
    const std::size_t* shape () const {
      return view ().shape ();
    }
    template <std::size_t n>
    std::size_t size () const {
      return view ().template size<n> ();
    }
    const std::ptrdiff_t* stridesBytes () const {
      return view ().stridesBytes ();
    }
    template <std::size_t n>
    std::ptrdiff_t strideBytes () const {
      return view ().template strideBytes<n> ();
    }

    typedef typename View::ConstElementOperatorType ConstElementOperatorType;
    ConstElementOperatorType operator[] (std::size_t pos) const {
      return view ()[pos];
    }
    typedef typename View::ElementOperatorType ElementOperatorType;
    ElementOperatorType operator[] (std::size_t pos) {
      return view ()[pos];
    }

#if HAVE_CXX11
    template <typename... ParT>
    typename Intern::OpPInfo<const T, Config, Assert, ParT...>::Info::RetType operator() (ParT... pars) const {
      return view () (pars...);
    }
    template <typename... ParT>
    typename Intern::OpPInfo<T, Config, Assert, ParT...>::Info::RetType operator() (ParT... pars) {
      return view () (pars...);
    }
    template <typename... ParT>
    typename View::ConstPointer pointer (ParT... pars) const {
      return view ().pointer (pars...);
    }
    template <typename... ParT>
    typename View::Pointer pointer (ParT... pars) {
      return view ().pointer (pars...);
    }
#else
#define TEMPLATE(n) BOOST_PP_IF (n, template <, public:/*To avoid warnings*/) \
    BOOST_PP_ENUM_PARAMS (n, typename ParT)                             \
    BOOST_PP_IF (n, >, public:/*To avoid warnings*/)
#define DEFINE_OVERLOADS(n)                                             \
    TEMPLATE(n)                                                         \
    typename Intern::OpPInfo_##n<const T, Config, Assert  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)>::Info::RetType operator() (BOOST_PP_ENUM_BINARY_PARAMS (n, const ParT, & pars)) const { \
      return view () (BOOST_PP_ENUM_PARAMS (n, pars));                  \
    }                                                                   \
    TEMPLATE(n)                                                         \
    typename Intern::OpPInfo_##n<T, Config, Assert  BOOST_PP_ENUM_TRAILING_PARAMS (n, ParT)>::Info::RetType operator() (BOOST_PP_ENUM_BINARY_PARAMS (n, const ParT, & pars)) { \
      return view () (BOOST_PP_ENUM_PARAMS (n, pars));                  \
    }                                                                   \
    TEMPLATE(n)                                                         \
    typename View::ConstPointer pointer (BOOST_PP_ENUM_BINARY_PARAMS (n, const ParT, & pars)) const { \
      return view ().pointer (BOOST_PP_ENUM_PARAMS (n, pars));          \
    }                                                                   \
    TEMPLATE(n)                                                         \
    typename View::Pointer pointer (BOOST_PP_ENUM_BINARY_PARAMS (n, const ParT, & pars)) { \
      return view ().pointer (BOOST_PP_ENUM_PARAMS (n, pars));          \
    }                                                                   \

#define DEFINE_OVERLOADS2(n) DEFINE_OVERLOADS (n)
#define DEFINE_OVERLOADS3(z, i, data) DEFINE_OVERLOADS2 (BOOST_PP_ADD(i, 1))
    BOOST_PP_REPEAT (MATH_ARRAY_MAX_ARRAY_DIM, DEFINE_OVERLOADS3, NOTHING)
#undef DEFINE_OVERLOADS3
#undef DEFINE_OVERLOADS2
#undef DEFINE_OVERLOADS
#undef TEMPLATE
#endif

    void assign (const ArrayView<const T, dim, Config, Assert>& orig) {
      view ().assign (orig);
    }

    void setTo (T value) {
      view ().setTo (value);
    }

    void setToZero () {
      view ().setToZero ();
    }
  };

  template <typename T, typename Config, typename Assert>
  Math::ArrayView<T, 2, Config, Assert> transposed (const Math::ArrayView<T, 2, Config, Assert>& view) {
    std::size_t shape[] = { view.template size<1> (), view.template size<0> () };
    std::ptrdiff_t stridesBytes[] = { view.template strideBytes<1> (), view.template strideBytes<0> () };
    return Math::ArrayView<T, 2, Config, Assert> (view.data (), shape, stridesBytes);
  }

  // TODO: implement version with compile-time arguments
  template <typename T, std::size_t dim, typename Config, typename Assert>
  Math::ArrayView<T, dim, Config, Assert> reorderDimensions (const Math::ArrayView<T, dim, Config, Assert>& view, const std::vector<int32_t>& dims) {
    ASSERT (dims.size () == dim);
    bool done[dim] = { false };
    std::size_t shape[dim];
    std::ptrdiff_t stridesBytes[dim];
    std::ptrdiff_t offset = 0;
    for (size_t i = 0; i < dim; i++) {
      int32_t val = dims[i];
      bool invert = val < 0;
      if (invert)
        val = -val;
      ASSERT (val > 0 && (uint32_t) val <= dim);
      val = val - 1;

      ASSERT (!done[val]);
      done[val] = true;
      shape[i] = view.shape ()[val];
      if (!invert) {
        stridesBytes[i] = view.stridesBytes ()[val];
      } else {
        stridesBytes[i] = -view.stridesBytes ()[val];
        if (shape[i])
          offset += (shape[i] - 1) * view.stridesBytes ()[val];
      }
    }
    return Math::ArrayView<T, dim, Config, Assert> (Config::template Type<T>::fromArith (view.arithData () + offset), shape, stridesBytes);
  }
}

#endif // !MATH_ARRAY_HPP_INCLUDED
