#include <iostream>
#include <cmath>
#include <vector>
#include <map>

#include <gudhi/Points_off_io.h>
#include <gudhi/Ad_simplex.h>
#include <gudhi/Simplex_tree.h>
#include <gudhi/Persistent_cohomology.h>
#include <gudhi/output_tikz.h>

#include <CGAL/Epick_d.h>

//#include <Eigen/Dense>
#include <Eigen/Sparse>

#include "cxx-prettyprint/prettyprint.hpp"

using K = CGAL::Epick_d<CGAL::Dynamic_dimension_tag>;
using FT = K::FT;
using Point_d = K::Point_d;
using Point_vector = std::vector< Point_d >;

using Matrix = Eigen::SparseMatrix<FT>;
using Triplet = Eigen::Triplet<FT>;
using Simplex_id = std::vector<int>;
using Vertex_id = Simplex_id;
using Pointer_range = typename std::vector<Point_vector::iterator>;


struct Lexicographic {
  bool operator() (const Simplex_id &lhs, const Simplex_id &rhs) {
    assert (lhs.size() == rhs.size());
    auto l_it = lhs.begin();
    auto r_it = rhs.begin();
    for (; l_it != lhs.end(); ++l_it, ++r_it)
      if (*l_it < *r_it)
        return true;
      else if (*l_it > *r_it)
        return false;
    return false;
  }
};

using SPMap = std::map<Simplex_id, Pointer_range, Lexicographic>;
using SPointer_range = typename std::vector<SPMap::iterator>;
using VSMap = std::map<Vertex_id, std::vector<int>, Lexicographic>;

struct Lexicographic_ptr {
  bool operator() (const SPMap::iterator &lhs, const SPMap::iterator &rhs) {
    Lexicographic lx;
    return lx(lhs->first,rhs->first);
  }
};

using SiMap = std::map<SPMap::iterator, int, Lexicographic_ptr>;


struct Simplex_tree_options_no_persistence {
  typedef Gudhi::linear_indexing_tag Indexing_tag;
  typedef int Vertex_handle;
  typedef FT Filtration_value;
  typedef std::uint32_t Simplex_key;
  static const bool store_key = true;
  static const bool store_filtration = false;
  static constexpr bool contiguous_vertices = true;
};
using Simplex_tree = Gudhi::Simplex_tree<>;
using Field_Zp = Gudhi::persistent_cohomology::Field_Zp;
using Persistent_cohomology = Gudhi::persistent_cohomology::Persistent_cohomology<Simplex_tree, Field_Zp>;

std::vector<FT> bounding_box_dimensions(Point_vector& points) {
  std::vector<FT> lower, upper, difference;
  for (auto x: points[0]) {
    lower.push_back(x);
    upper.push_back(x);
  }
  for (auto p: points)
    for (unsigned i = 0; i < p.size(); i++) {
      if (p[i] < lower[i])
        lower[i] = p[i];
      if (p[i] > upper[i])
        upper[i] = p[i];
    }
  for (unsigned i = 0; i < lower.size(); i++)
    difference.push_back(upper[i]-lower[i]);
  return difference;
}

/** \brief Write triangles (tetrahedra in 3d) of a simplicial complex in a file, compatible with medit.
 *  `landmarks_ind` represents the set of landmark indices in W  
 *  `st` is the Simplex_tree to be visualized,
 *  `shr` is the Simplex_handle_range of simplices in `st` to be visualized
 *  `is2d` should be true if the simplicial complex is 2d, false if 3d
 *  `l_is_v` = landmark is vertex
 */
void write_coxeter_mesh(Point_vector& W, VSMap& vs_map, Matrix& root_t, std::string file_name = "coxeter.mesh")
{
  short d = W[0].size();
  if (d > 3);
  
  std::ofstream ofs (file_name, std::ofstream::out);
  if (d <= 2)
    ofs << "MeshVersionFormatted 1\nDimension 2\n";
  else
    ofs << "MeshVersionFormatted 1\nDimension 3\n";
  
  int num_vertices = (d+1)*W.size() + vs_map.size(), num_edges = 0, num_triangles = 0, num_tetrahedra = 0;
  // if (d <= 2) {
  //   num_triangles = W.size();
  //   num_edges = W.size()*3;
  //   num_vertices += W.size()*3;
  // }
  // else {
  //   num_tetrahedra = W.size();
  //   num_triangles = W.size()*4;
  //   num_edges = W.size()*6;
  //   num_vertices += W.size()*4;
  // }

  ofs << "Vertices\n" << num_vertices << "\n";

  W.clear();
  for (auto m: vs_map) {
    FT denom = m.first[0];
    Eigen::VectorXd b(d);
    for (int i = 0; i < d; i++) {
      b(i) = m.first[i+1]/denom;
    }
    Eigen::SimplicialLDLT<Matrix, Eigen::Upper> chol(root_t);
    Eigen::VectorXd x = chol.solve(b);
    if(chol.info()!=Eigen::Success) {
      std::cout << "solving failed\n";
    }
    std::vector<FT> np;
    for (int i = 0; i < d; i++)
      np.push_back(x(i));
    W.push_back(Point_d(np));
  }
  std::map<int,std::vector<int>> sv_map;
  int j = 1;
  for (auto m: vs_map) {
    for (auto s: m.second) {
      auto find_it = sv_map.find(s);
      if (find_it == sv_map.end())
        sv_map.emplace(s, std::vector<int>(1,j));
      else
        find_it->second.push_back(j);
    }
    j++;
  }

  
  FT p_prop = 0.001;
  int p_col = 208;
  std::vector<FT> bbox_dimensions = bounding_box_dimensions(W);
  std::cout << bbox_dimensions << "\n";
  for (auto p: W) {
    for (auto coord = p.cartesian_begin(); coord != p.cartesian_end() && coord != p.cartesian_begin()+3 ; ++coord) 
      ofs << *coord << " "; 
    ofs << "508\n";
    // for (int i = 0; i < d; i++) {
    //   int j = 0;
    //   for (auto coord = p.cartesian_begin(); coord != p.cartesian_end() && coord != p.cartesian_begin()+3 ; ++coord, j++)
    //     if (j == i)
    //       ofs << *coord + bbox_dimensions[i]*p_prop << " ";
    //     else
    //       ofs << *coord << " ";
    //   ofs << "108\n";
    // }
  }
  num_edges = ((d+1)*d/2)*W.size()/2;
  num_triangles = ((d+1)*d*(d-1)/6)*W.size();
  num_tetrahedra = W.size();
  
  // ofs << "Edges " << num_edges << "\n";
  // for (unsigned i = 0; i < W.size(); i++) {
  //   ofs << (d+1)*i+1 << " " << (d+1)*i+2 << " " << p_col << "\n";
  //   ofs << (d+1)*i+1 << " " << (d+1)*i+3 << " " << p_col << "\n";
  //   ofs << (d+1)*i+2 << " " << (d+1)*i+3 << " " << p_col << "\n";
  //   if (d == 3) {
  //     ofs << (d+1)*i+1 << " " << (d+1)*i+4 << " " << p_col << "\n";
  //     ofs << (d+1)*i+2 << " " << (d+1)*i+4 << " " << p_col << "\n";
  //     ofs << (d+1)*i+3 << " " << (d+1)*i+4 << " " << p_col << "\n";
  //   }
  // }
  if (d == 2) {
    ofs << "Triangles " << sv_map.size() << "\n";
    for (auto m: sv_map) {
      for (auto i: m.second)
        ofs << i << " ";
      ofs << p_col << "\n";
    }
  }
  if (d == 3) {
    ofs << "Tetrahedra " << sv_map.size() << "\n";  
    for (auto m: sv_map) {
      for (auto i: m.second)
        ofs << i << " ";
      ofs << p_col << "\n";
    }
  }
}

int gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

/** Common gcd simplification */
template <class Id>
Id reduced_id(Id& id) {
  int common_gcd = 0;
  for (auto i: id) {
    common_gcd = gcd(i, common_gcd);
    if (common_gcd == 1)
      return id;
  }
  Id id_red(id);
  for (auto i_it = id_red.begin(); i_it != id_red.end(); ++i_it) {
    *i_it = *i_it / common_gcd;
  }
  return id_red;
}

/** A conversion from Cartesian coordinates to root coordinates.
 *  The matrix' rows are root vectors (or normal vectors of a simplex in general).
 */
template <class Point,
          class Matrix>
Point root_coordinates(Point p, Matrix& root_t, short d)
{
  // short d = p.size();
  std::vector<double> p_r;
  for (int i = 0; i < d; i++) {
    FT sc_prod = 0;
    /* for now no root normalization takes place */
    // FT root_norm_sq = 0;
    // for (int j = 0; j < d; j++)
    //   root_norm_sq += root_t.coeff(i,j)*root_t.coeff(i,j);
    // FT root_norm = sqrt()
    for (int j = 0; j < d; j++) {
      sc_prod += root_t.coeff(i,j) * p[j];
    }
    p_r.push_back(sc_prod);
  }
  return Point(p_r);
}

/** A conversion from Cartesian coordinates to root coordinates in a point range.
 *  The matrix' rows are root vectors (or normal vectors of a simplex in general).
 *  The input point range is rewritten.
 */
template <class Point_list,
          class Matrix>
Point_list root_coordinates_range(Point_list& points, Matrix& root_t)
{
  short d = points[0].size();
  Point_list points_r;
  for (auto p: points) {
    points_r.push_back(root_coordinates(p,root_t,d));
  }
  return points_r;
}

/** A conversion from Cartesian coordinates to the coordinates of the alcove containing the point.
 *  The matrix' rows are simple root vectors.
 */
template <class Point,
          class Matrix>
Simplex_id alcove_coordinates(Point& p, Matrix& root_t)
{
  short d = p.size();
  Simplex_id s_id(1,1);
  Eigen::VectorXd p_vect(d);
  for (short i = 0; i < d; i++)
    p_vect(i) = p[i];
  Eigen::VectorXd scalprod_vect = root_t * p_vect;
  for (short i = 0; i < d; i++) {
    FT root_scalprod = 0;
    for (short j = i; j >= 0; j--) {
      root_scalprod += scalprod_vect(j);
      s_id.push_back(std::floor(root_scalprod));
    }
  }
  return s_id;
}

/** Add the vertices of the given simplex to a vertex-simplex map.
 * The size of si_it->first is d*(d+1)/2.
 */
template <class S_id_iterator>
void rec_add_vertices_to_map(Vertex_id& v_id, S_id_iterator s_it, int index, VSMap& vs_map, unsigned d)
{
  unsigned k = v_id.size();
  if (k == d+1) {
    Vertex_id v_id_red = reduced_id(v_id);
    auto find_it = vs_map.find(v_id_red);
    if (find_it == vs_map.end())
      vs_map.emplace(v_id_red, std::vector<int>(1, index));
    else
      find_it->second.push_back(index);    
    return;
  }
  int simplex_coord = *s_it; s_it++;
  v_id.push_back(simplex_coord);
  int sum = simplex_coord;
  S_id_iterator s_it_copy(s_it);
  bool valid = true;
  for (unsigned i = 1; i < k && valid; i++) {
    sum += v_id[k-i];
    if (sum < *s_it_copy || sum > *s_it_copy + 1)
      valid = false;
    s_it_copy++;
  }
  if (valid)
    rec_add_vertices_to_map(v_id, s_it_copy, index, vs_map, d);
  v_id.pop_back();

  v_id.push_back(simplex_coord + 1);
  sum = simplex_coord + 1;
  s_it_copy = s_it;
  valid = true;
  for (unsigned i = 1; i < k && valid; i++) {
    sum += v_id[k-i];
    if (sum < *s_it_copy || sum > *s_it_copy + 1)
      valid = false;
    s_it_copy++;
  }
  if (valid)
    rec_add_vertices_to_map(v_id, s_it_copy, index, vs_map, d);
  v_id.pop_back();
}
  
/** Add the vertices of the given simplex to a vertex-simplex map.
 * The size of si_it->first is d*(d+1)/2.
 */
template <class Si_map_iterator>
void add_vertices_to_map(Si_map_iterator si_it, VSMap& vs_map)
{
  unsigned d = (-1+std::round(std::sqrt(1+8*si_it->first->first.size())))/2;
  Vertex_id v_id(1,*si_it->first->first.begin());
  v_id.reserve(d+1);
  rec_add_vertices_to_map(v_id, si_it->first->first.begin()+1, si_it->second, vs_map, d);
}

/** Current state of the algorithm.
 *  Input: a point cloud 'point_vector'
 *  Output: a reconstruction (a simplicial complex?, a Czech-like complex?)
 */

int main(int argc, char * const argv[]) {
  std::cout << "Marching cube adaptation for Coxeter triangulations\n";
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0]
        << " path_to_off_point_file\n";
    return 0;
  }
  Point_vector point_vector;
  Gudhi::Points_off_reader<Point_d> off_reader(argv[1]);
  if (!off_reader.is_valid()) {
      std::cerr << "Coxeter triangulations - Unable to read file " << argv[1] << "\n";
      exit(-1);  // ----- >>
    }
  point_vector = Point_vector(off_reader.get_point_cloud());
  int N = point_vector.size();
  short d = point_vector[0].size();
  // short d = 2;
  std::cout << "Successfully read " << N << " points in dimension " << d << std::endl;

  // The A root vectors, computed as a matrix

  std::vector<Triplet> cartan_triplets;
  cartan_triplets.reserve(3*d-2);
  for (int i = 0; i < d; i++) {
    cartan_triplets.push_back(Triplet(i,i,2.0));
  }
  for (int i = 1; i < d; i++) {
    cartan_triplets.push_back(Triplet(i-1,i,-1.0));
    cartan_triplets.push_back(Triplet(i,i-1,-1.0));
  }
  Matrix cartan(d,d);
  cartan.setFromTriplets(cartan_triplets.begin(), cartan_triplets.end());
  std::cout << "cartan =" << std::endl << cartan << std::endl;

  Eigen::SimplicialLLT<Matrix, Eigen::Lower> chol(cartan);
  Matrix root_t = chol.matrixL();
  std::cout << "root^t =" << std::endl << root_t << std::endl;
    
  // Point_vector rc_point_vector = root_coordinates_range(point_vector, root_t);

  // The first fill of a map: simplex coordinates -> points
  std::cout << "Point cartesian coordinates: " << point_vector[0] << std::endl
            << "Simple root root coordinates: " << root_coordinates(point_vector[0], root_t, d) << std::endl
            << "Alcove coordinates: " << alcove_coordinates(point_vector[0], root_t) << std::endl;
  // std::cout << std::endl;
  SPMap sp_map;
  for (auto p_it = point_vector.begin(); p_it != point_vector.end(); ++p_it) {
    Simplex_id s_id = alcove_coordinates(*p_it, root_t); 
    auto find_it = sp_map.find(s_id);
    if (find_it == sp_map.end())
      sp_map.emplace(s_id, Pointer_range(1, p_it));
    else
      find_it->second.push_back(p_it);
  }
  // std::cout << "SPMap composition:\n";
  // for (auto m: sp_map) {
  //   std::cout << m.first << ": " << m.second.size() << " elements.\n";
  // }

  // // small test
  // Simplex_id p1 = {4,12,10,-8};
  // std::cout << "Non-reduced: " << p1 << ", reduced: " << reduced_id(p1) << ".\n";

  SiMap si_map;
  int si_index = 0;
  for (auto m_it = sp_map.begin(); m_it != sp_map.end(); ++m_it, si_index++)
    si_map.emplace(m_it, si_index);

  std::cout << "SIMap composition:\n";
  for (auto m: si_map) {
    std::cout << m.first->first << ": index " << m.second << ".\n";
  }
  
  // map : vertex coordinates -> simplex coordinates
  VSMap vs_map;
  // add_vertices_to_map(si_map.begin(), vs_map);
  for (auto si_it = si_map.begin(); si_it != si_map.end(); ++si_it)
    add_vertices_to_map(si_it, vs_map);
  std::cout << "VSMap composition:\n";
  for (auto m: vs_map) {
    std::cout << m.first << ": " << m.second << ".\n";
  }

  // simplex tree construction
  // Simplex_tree st;
  // for (auto m: vs_map) {
  //   st.insert_simplex_and_subfaces(m.second, m.first[0]);
  // }
  // std::cout << st;

  // Persistent_cohomology pcoh(st);
  // // initializes the coefficient field for homology
  // pcoh.init_coefficients(11);

  // pcoh.compute_persistent_cohomology(-0.1);
  // pcoh.output_diagram();

  write_coxeter_mesh(point_vector, vs_map, root_t, "coxeter.mesh");
}