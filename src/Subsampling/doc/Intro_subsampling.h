/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Clement Jamin
 *
 *    Copyright (C) 2016 INRIA
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DOC_SUBSAMPLING_INTRO_SUBSAMPLING_H_
#define DOC_SUBSAMPLING_INTRO_SUBSAMPLING_H_

// needs namespace for Doxygen to link on classes
namespace Gudhi {
// needs namespace for Doxygen to link on classes
namespace subsampling {

/**  \defgroup subsampling Subsampling
 * 
 * \author Cl&eacute;ment Jamin, Siargey Kachanovich
 * 
 * @{
 * 
 * \section introduction Introduction
 * 
 * This Gudhi component offers methods to subsample a set of points.
 * 
 * \section sparsifyexamples Example: sparsify_point_set
 * 
 * This example outputs a subset of the input points so that the 
 * squared distance between any two points
 * is greater than or equal to 0.4.
 * 
 * \include Subsampling/example_sparsify_point_set.cpp
 * 
 * \copyright GNU General Public License v3.                         
 * \verbatim  Contact: gudhi-users@lists.gforge.inria.fr \endverbatim
 */
/** @} */  // end defgroup subsampling

}  // namespace subsampling

}  // namespace Gudhi

#endif  // DOC_SUBSAMPLING_INTRO_SUBSAMPLING_H_
