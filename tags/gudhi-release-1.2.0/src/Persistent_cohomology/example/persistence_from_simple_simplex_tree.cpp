/*    This file is part of the Gudhi Library. The Gudhi library
 *    (Geometric Understanding in Higher Dimensions) is a generic C++
 *    library for computational topology.
 *
 *    Author(s):       Vincent Rouvreau
 *
 *    Copyright (C) 2014  INRIA Sophia Antipolis-Méditerranée (France)
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

#include <gudhi/graph_simplicial_complex.h>
#include <gudhi/Simplex_tree.h>
#include <gudhi/Persistent_cohomology.h>

#include <iostream>
#include <ctime>
#include <utility>
#include <vector>

using namespace Gudhi;
using namespace Gudhi::persistent_cohomology;

typedef std::vector< Vertex_handle > typeVectorVertex;
typedef std::pair<typeVectorVertex, Filtration_value> typeSimplex;
typedef std::pair< Simplex_tree<>::Simplex_handle, bool > typePairSimplexBool;
typedef Simplex_tree<> typeST;

void usage(char * const progName) {
  std::cerr << "Usage: " << progName << " coeff_field_characteristic[integer > 0] min_persistence[float >= -1.0]\n";
  exit(-1);
}

int main(int argc, char * const argv[]) {
  // program args management
  if (argc != 3) {
    std::cerr << "Error: Number of arguments (" << argc << ") is not correct\n";
    usage(argv[0]);
  }

  int coeff_field_characteristic = 0;
  int returnedScanValue = sscanf(argv[1], "%d", &coeff_field_characteristic);
  if ((returnedScanValue == EOF) || (coeff_field_characteristic <= 0)) {
    std::cerr << "Error: " << argv[1] << " is not correct\n";
    usage(argv[0]);
  }

  Filtration_value min_persistence = 0.0;
  returnedScanValue = sscanf(argv[2], "%lf", &min_persistence);
  if ((returnedScanValue == EOF) || (min_persistence < -1.0)) {
    std::cerr << "Error: " << argv[2] << " is not correct\n";
    usage(argv[0]);
  }

  // TEST OF INSERTION
  std::cout << "********************************************************************" << std::endl;
  std::cout << "TEST OF INSERTION" << std::endl;
  typeST st;

  // ++ FIRST
  std::cout << "   - INSERT (0,1,2)" << std::endl;
  typeVectorVertex SimplexVector = {0, 1, 2};
  st.insert_simplex_and_subfaces(SimplexVector, 0.3);

  // ++ SECOND
  std::cout << "   - INSERT 3" << std::endl;
  SimplexVector = {3};
  st.insert_simplex_and_subfaces(SimplexVector, 0.1);

  // ++ THIRD
  std::cout << "   - INSERT (0,3)" << std::endl;
  SimplexVector = {0, 3};
  st.insert_simplex_and_subfaces(SimplexVector, 0.2);

  // ++ FOURTH
  std::cout << "   - INSERT (0,1) (already inserted)" << std::endl;
  SimplexVector = {0, 1};
  st.insert_simplex_and_subfaces(SimplexVector, 0.2);

  // ++ FIFTH
  std::cout << "   - INSERT (3,4,5)" << std::endl;
  SimplexVector = {3, 4, 5};
  st.insert_simplex_and_subfaces(SimplexVector, 0.3);

  // ++ SIXTH
  std::cout << "   - INSERT (0,1,6,7)" << std::endl;
  SimplexVector = {0, 1, 6, 7};
  st.insert_simplex_and_subfaces(SimplexVector, 0.4);

  // ++ SEVENTH
  std::cout << "   - INSERT (4,5,8,9)" << std::endl;
  SimplexVector = {4, 5, 8, 9};
  st.insert_simplex_and_subfaces(SimplexVector, 0.4);

  // ++ EIGHTH
  std::cout << "   - INSERT (9,10,11)" << std::endl;
  SimplexVector = {9, 10, 11};
  st.insert_simplex_and_subfaces(SimplexVector, 0.3);

  // ++ NINETH
  std::cout << "   - INSERT (2,10,12)" << std::endl;
  SimplexVector = {2, 10, 12};
  st.insert_simplex_and_subfaces(SimplexVector, 0.3);

  // ++ TENTH
  std::cout << "   - INSERT (11,6)" << std::endl;
  SimplexVector = {6, 11};
  st.insert_simplex_and_subfaces(SimplexVector, 0.2);

  // ++ ELEVENTH
  std::cout << "   - INSERT (13,14,15)" << std::endl;
  SimplexVector = {13, 14, 15};
  st.insert_simplex_and_subfaces(SimplexVector, 0.25);

  /* Inserted simplex:         */
  /*    1   6                  */
  /*    o---o                  */
  /*   /X\7/      4       2    */
  /*  o---o---o---o       o    */
  /*  2   0   3\X/8\  10 /X\   */
  /*            o---o---o---o  */
  /*            5   9\X/    12 */
  /*                  o---o    */
  /*                 11   6    */
  /* In other words:          */
  /*   A facet [2,1,0]        */
  /*   An edge [0,3]          */
  /*   A facet [3,4,5]        */
  /*   A cell  [0,1,6,7]      */
  /*   A cell  [4,5,8,9]      */
  /*   A facet [9,10,11]      */
  /*   An edge [11,6]         */
  /*   An edge [10,12,2]      */

  st.set_dimension(2);
  st.set_filtration(0.4);

  std::cout << "The complex contains " << st.num_simplices() << " simplices - " << st.num_vertices() << " vertices "
      << std::endl;
  std::cout << "   - dimension " << st.dimension() << "   - filtration " << st.filtration() << std::endl;
  std::cout << std::endl << std::endl << "Iterator on Simplices in the filtration, with [filtration value]:"
      << std::endl;
  std::cout << "**************************************************************" << std::endl;
  std::cout << "strict graph G { " << std::endl;

  for (auto f_simplex : st.filtration_simplex_range()) {
    std::cout << "   " << "[" << st.filtration(f_simplex) << "] ";
    for (auto vertex : st.simplex_vertex_range(f_simplex)) {
      std::cout << static_cast<int>(vertex) << " -- ";
    }
    std::cout << ";" << std::endl;
  }

  std::cout << "}" << std::endl;
  std::cout << "**************************************************************" << std::endl;

  // Compute the persistence diagram of the complex
  persistent_cohomology::Persistent_cohomology< Simplex_tree<>, Field_Zp > pcoh(st);
  // initializes the coefficient field for homology
  pcoh.init_coefficients(coeff_field_characteristic);

  pcoh.compute_persistent_cohomology(min_persistence);

  // Output the diagram in filediag
  pcoh.output_diagram();
  return 0;
}