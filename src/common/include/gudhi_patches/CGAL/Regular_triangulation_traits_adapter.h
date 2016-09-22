// Copyright (c) 2014 INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
// Author(s)     : Clement Jamin

#ifndef CGAL_REGULAR_TRIANGULATION_TRAITS_ADAPTER_H
#define CGAL_REGULAR_TRIANGULATION_TRAITS_ADAPTER_H

#include <CGAL/basic.h>

#include <boost/iterator/transform_iterator.hpp>

namespace CGAL {

// Wrapper class to make a model of `RegularTriangulationTraits` easily usable
// by the `Regular_triangulation` class. By using this class:
// - Point_d (used by `Triangulation` and the TDS) becomes a weighted point
// - Predicates and functors such as Less_coordinate_d or Orientation_d
//   can be called using weighted points instead of bare points (this is
//   needed because `Weighted_point_d` is not convertible to `Point_d`)
// This way, `Triangulation` works perfectly well with weighted points.

template <class K>
class Regular_triangulation_traits_adapter
  : public K
{
public:
  typedef K                                         Base;

  // Required by TriangulationTraits
  typedef typename K::Dimension                     Dimension;
  typedef typename K::FT                            FT;
  typedef typename K::Flat_orientation_d            Flat_orientation_d;
  typedef typename K::Weighted_point_d              Point_d;

  // Required by RegularTriangulationTraits
  typedef typename K::Point_d                       Bare_point_d;
  typedef typename K::Weighted_point_d              Weighted_point_d;
  typedef typename K::Point_drop_weight_d           Point_drop_weight_d;
  typedef typename K::Point_weight_d                Point_weight_d;
  typedef typename K::Power_side_of_power_sphere_d  Power_side_of_power_sphere_d;
  typedef typename K::In_flat_power_side_of_power_sphere_d 
                                                    In_flat_power_side_of_power_sphere_d;

  //===========================================================================
  // Custom types
  //===========================================================================

  // Required by SpatialSortingTraits_d
  class Less_coordinate_d
  {
    const K &m_kernel;

  public:
    typedef bool result_type;

    Less_coordinate_d(const K &kernel)
      : m_kernel(kernel) {}

    result_type operator()(
      Weighted_point_d const& p, Weighted_point_d const& q, int i) const
    {
      Point_drop_weight_d pdw = m_kernel.point_drop_weight_d_object();
      return m_kernel.less_coordinate_d_object() (pdw(p), pdw(q), i);
    }
  };

  //===========================================================================

  // Required by TriangulationTraits
  class Orientation_d
  {
    const K &m_kernel;

  public:
    typedef Orientation result_type;

    Orientation_d(const K &kernel)
      : m_kernel(kernel) {}

    template <typename ForwardIterator> 
    result_type operator()(ForwardIterator start, ForwardIterator end) const
    {
      Point_drop_weight_d pdw = m_kernel.point_drop_weight_d_object();
      return m_kernel.orientation_d_object() (
        boost::make_transform_iterator(start, pdw),
        boost::make_transform_iterator(end, pdw)
      );
    }
  };

  //===========================================================================

  // Required by TriangulationTraits
  class Construct_flat_orientation_d
  {
    const K &m_kernel;

  public:
    typedef Flat_orientation_d result_type;
    
    Construct_flat_orientation_d(const K &kernel)
      : m_kernel(kernel) {}

    template <typename ForwardIterator> 
    result_type operator()(ForwardIterator start, ForwardIterator end) const
    {
      Point_drop_weight_d pdw = m_kernel.point_drop_weight_d_object();
      return m_kernel.construct_flat_orientation_d_object() (
        boost::make_transform_iterator(start, pdw),
        boost::make_transform_iterator(end, pdw)
      );
    }
  };


  //===========================================================================

  // Required by TriangulationTraits
  class In_flat_orientation_d
  {
    const K &m_kernel;

  public:
    typedef Orientation result_type;
    
    In_flat_orientation_d(const K &kernel)
      : m_kernel(kernel) {}

    template <typename ForwardIterator> 
    result_type operator()(Flat_orientation_d orient, 
      ForwardIterator start, ForwardIterator end) const
    {
      Point_drop_weight_d pdw = m_kernel.point_drop_weight_d_object();
      return m_kernel.in_flat_orientation_d_object() (
        orient,
        boost::make_transform_iterator(start, pdw),
        boost::make_transform_iterator(end, pdw)
      );
    }
  };

  //===========================================================================
  
  // Required by TriangulationTraits
  class Contained_in_affine_hull_d
  {
    const K &m_kernel;

  public:
    typedef bool result_type;
    
    Contained_in_affine_hull_d(const K &kernel)
      : m_kernel(kernel) {}

    template <typename ForwardIterator> 
    result_type operator()(ForwardIterator start, ForwardIterator end, 
                           const Weighted_point_d & p) const
    {
      Point_drop_weight_d pdw = m_kernel.point_drop_weight_d_object();
      return m_kernel.contained_in_affine_hull_d_object() (
        boost::make_transform_iterator(start, pdw),
        boost::make_transform_iterator(end, pdw),
        pdw(p)
      );
    }
  };

  //===========================================================================

  // Required by TriangulationTraits
  class Compare_lexicographically_d
  {
    const K &m_kernel;

  public:
    typedef Comparison_result result_type;
    
    Compare_lexicographically_d(const K &kernel)
      : m_kernel(kernel) {}

    result_type operator()(
      const Weighted_point_d & p, const Weighted_point_d & q) const
    {
      Point_drop_weight_d pdw = m_kernel.point_drop_weight_d_object();
      return m_kernel.compare_lexicographically_d_object()(pdw(p), pdw(q));
    }
  };
  
  //===========================================================================

  // Only for Triangulation_off_ostream.h (undocumented)
  class Compute_coordinate_d
  {
    const K &m_kernel;

  public:
    typedef FT result_type;
    
    Compute_coordinate_d(const K &kernel)
      : m_kernel(kernel) {}

    result_type operator()(
      const Weighted_point_d & p, const int i) const
    {
      Point_drop_weight_d pdw = m_kernel.point_drop_weight_d_object();
      return m_kernel.compute_coordinate_d_object()(pdw(p), i);
    }
  };

  //===========================================================================

  // To satisfy SpatialSortingTraits_d
  // and also for Triangulation_off_ostream.h (undocumented)
  class Point_dimension_d
  {
    const K &m_kernel;

  public:
    typedef int result_type;
    
    Point_dimension_d(const K &kernel)
      : m_kernel(kernel) {}

    result_type operator()(
      const Weighted_point_d & p) const
    {
      Point_drop_weight_d pdw = m_kernel.point_drop_weight_d_object();
      return m_kernel.point_dimension_d_object()(pdw(p));
    }
  };
  
  //===========================================================================
  // Object creation
  //===========================================================================

  Less_coordinate_d less_coordinate_d_object() const
  {
    return Less_coordinate_d(*this);
  }
  Contained_in_affine_hull_d contained_in_affine_hull_d_object() const
  { 
    return Contained_in_affine_hull_d(*this); 
  }
  Orientation_d orientation_d_object() const
  {
    return Orientation_d(*this); 
  }
  Construct_flat_orientation_d construct_flat_orientation_d_object() const
  { 
    return Construct_flat_orientation_d(*this);
  }
  In_flat_orientation_d in_flat_orientation_d_object() const
  { 
    return In_flat_orientation_d(*this);
  }
  Compare_lexicographically_d compare_lexicographically_d_object() const
  { 
    return Compare_lexicographically_d(*this);
  }
  Compute_coordinate_d compute_coordinate_d_object() const
  { 
    return Compute_coordinate_d(*this);
  }
  Point_dimension_d point_dimension_d_object() const
  { 
    return Point_dimension_d(*this);
  }
};


} //namespace CGAL

#endif // CGAL_REGULAR_TRIANGULATION_TRAITS_ADAPTER_H