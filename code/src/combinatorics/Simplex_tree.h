/*
 *  Simplex_tree.h
 *  Gudhi
 *
 *  Created by Clément Maria on 1/7/14.
 *  Copyright 2014 INRIA. All rights reserved.
 *
 */

#ifndef GUDHI_SIMPLEX_TREE_H
#define GUDHI_SIMPLEX_TREE_H

#include <boost/container/flat_map.hpp>
#include "Simplex_tree_node_explicit_storage.h"
#include "Simplex_tree_siblings.h"
#include "Simplex_tree_iterators.h" // implementation of the iterators
                                    // for Simplex_tree.
 #include "boost/iterator/transform_iterator.hpp"

/**
 * \brief Simplex tree data structure.
 *
 * Every simplex \f$[v_0, \cdots ,v_d]\f$ admits a canonical orientation
 * induced by the order relation on vertices \f$ v_0 < \cdots < v_d \f$,
 $ furnished by MetricSpace.
 *
 * \implements SimplexDataFilteredSimplicialComplexDS
 */
template < class MetricSpace >
class Simplex_tree {
  public:
// Metric Space types  
  typedef typename MetricSpace::FT                              Filtration_value         ;
  typedef typename MetricSpace::Vertex                          Vertex                   ;   
  typedef typename MetricSpace::Space_vertex_iterator           Space_vertex_iterator    ;
  typedef typename MetricSpace::Space_vertex_range              Space_vertex_range       ;

  /** \brief Type of data store in each simplex. */
  typedef int                                                   Simplex_data;
  /** \brief Node in the simplex tree.*/ 
  typedef Simplex_tree_node_explicit_storage < Simplex_tree >   Node          ;
 /** \brief Must be an ordered range. */
 // typedef boost::flat_map< Vertex, Node >                        Dictionary    ;

  /** \brief Set of nodes sharing a same parent in the simplex tree. */
  typedef Simplex_tree_siblings < Vertex
                                , Filtration_value
                                , Node
                                , boost::container::flat_map< Vertex, Node > 
                                >                               Siblings      ;
  /** \todo Didn't manage to define the Dictionary type in Simplex_tree because
  * of the mutual dependence of Siblings and Node.*/
  typedef typename Siblings::Dictionary                         Dictionary;
  typedef typename Dictionary::iterator                         Dictionary_it;
  typedef typename Dictionary::iterator                         Simplex_handle;
  typedef typename Dictionary_it::value_type                    Dit_value_t;

// Simplex Tree Iterators
  typedef Simplex_vertex_iterator < Simplex_tree >              Simplex_vertex_iterator    ;
  typedef boost::iterator_range < Simplex_vertex_iterator >     Simplex_vertex_range       ;
  typedef Boundary_simplex_iterator < Simplex_tree >            Boundary_simplex_iterator  ;
  typedef boost::iterator_range < Boundary_simplex_iterator >   Boundary_simplex_range     ;
  typedef Complex_simplex_iterator < Simplex_tree >             Complex_simplex_iterator   ;
  typedef boost::iterator_range < Complex_simplex_iterator >    Complex_simplex_range      ;
  typedef typename std::vector < Simplex_handle >::iterator     Filtration_simplex_iterator; 
  typedef boost::iterator_range < Filtration_simplex_iterator > Filtration_simplex_range   ;
 
  struct return_first {Vertex operator()(const Dit_value_t& p_sh) const {return p_sh.first;}};
  typedef boost::transform_iterator< return_first,
                                     Dictionary_it >            Complex_vertex_iterator;
  typedef boost::iterator_range < Complex_vertex_iterator >     Complex_vertex_range;

/** \brief Returns a range over the vertices of the complex.
*
* The iterators have value type Vertex.*/
Complex_vertex_range complex_vertex_range()
{  return Complex_vertex_range(boost::make_transform_iterator(root_.members_.begin(),return_first()),
                               boost::make_transform_iterator(root_.members_.end(),return_first())  );}

void assign_data(Simplex_handle sh, Simplex_data key)
{ sh->second.assign_data(key);}

/** \brief Returns the dimension of a simplex.*/
int dimension(Simplex_handle sh)
{ Siblings curr_sib = self_siblings(sh);
  int dim = 0;
  while(curr_sib != NULL) { ++dim; curr_sib = curr_sib->oncles(); }
  return dim-1;}
/** \brief Returns true iff the simplex is a vertex (dim 0).*/
bool is_vertex(Simplex_handle sh)
{ return (self_siblings(sh)->oncles() == NULL);}
/** \brief Returns true iff the simplex is an edge (dim 1).*/
bool is_edge(Simplex_handle sh)
{ Siblings sib = self_siblings(sh)->oncles();
  return (sib != NULL && sib->oncles() == NULL);}

bool has_children(Simplex_handle sh)
{ return (sh->second.children()->parent() == sh->first); }

// Returns the Siblings containg a simplex.*/
Siblings * self_siblings(Simplex_handle sh)
{ if(sh->second.children()->parent() == sh->first) return sh->second.children()->oncles();
  else                                             return sh->second.children(); }

/**
* \brief Returns a range over the sequence of all simplices in
* the filtered simplicial complex, in the order of a filtration.
*
* The iterators have 'value type' Simplex_handle. If the filtration
* has not been initialized yet, the method initializes it (i.e.
* order the simplices).
*/
Filtration_simplex_range filtration_simplex_range() 
{ if(filtration_vect_.size() == 0) { initialize_filtration(); }
  return Filtration_simplex_range(filtration_vect_.begin(),
                                  filtration_vect_.end());}
/** \brief Returns a range over the sequence of vertices of a simplex.
 *
 * The iterators have 'value type' Vertex. The order in which the 
 * vertices are visited is the decreasing order, which is consequenlty
 * equal to \f$(-1)^{\text{dim} \sigma}\f$ the 
 * canonical orientation on the simplex. */
  Simplex_vertex_range simplex_vertex_range(Simplex_handle sh)
  { return Simplex_vertex_range (Simplex_vertex_iterator(this,sh),
                                 Simplex_vertex_iterator(this));}
/** \brief Returns a range over the simplices of the boundary of a simplex, i.e.
 * the set of codimension \f$1\f$ subsimplices of the simplex.
 *
 * If the simplex is \f$[v_0, \cdots ,v_d]\f$, with canonical orientation
 * induced by \f$ v_0 < \cdots < v_d \f$, the iterator enumerates the 
 * simplices of the boundary in the order: 
 * \f$[v_0,\cdots,\widehat{v_i},\cdots,v_d]\f$ for \f$i\f$ from \f$0\f$ to \f$d\f$,
 * where \f$\widehat{v_i}\f$ means that the vertex \f$v_i\f$ is omitted.
 *
 * We note that the alternate sum of the simplices given by the iterator
 * gives \f$(-1)^{\text{dim} \sigma}\f$ the chains corresponding to the boundary of the simplex.
 */
  Boundary_simplex_range boundary_simplex_range(Simplex_handle sh)
  { return Boundary_simplex_range (Boundary_simplex_iterator(this,sh),
                                   Boundary_simplex_iterator(this) );}
/**
 * Returns a Complex_simplex_range over all simplices
 * of dimension > 0 
 * in the simplicial complex stored in the simplex
 * tree.
 */
Complex_simplex_range complex_simplex_range()
{ return Complex_simplex_range (Complex_simplex_iterator(this),
                                Complex_simplex_iterator() ); }

void print(Simplex_handle sh, std::ostream& os = std::cout)
{ for(auto v : simplex_vertex_range(sh)) {os << v << " ";} 
  os << std::endl;}

/** \brief Empty constructor.*/
Simplex_tree( MetricSpace & ms ) :
ms_(&ms),
nb_vertices_(0),
size_cpx_(0),
root_(), //how to put null vertex here ?
filtration_vect_()
//NULL_sh_(ms_->NULL_vertex,Node())
{root_.parent_ = null_vertex();}

Simplex_data simplex_data(Simplex_handle sh)
{ return sh->second.data(); }


/** \todo iterative version ~Simplex_tree*/
~Simplex_tree() 
{ 
  for(auto sh = root_.members().begin(); sh != root_.members().end(); ++sh)
    { if(has_children(sh)) {rec_delete(sh->second.children());} }
}
void rec_delete(Siblings * sib)
{ for(auto sh = sib->members().begin(); sh != sib->members().end(); ++sh)
    { if(has_children(sh)) {rec_delete(sh->second.children());} }
  delete sib;
}

/** 
* \brief Given a sequence of Vertices, returns the
* Simplex_handle in the simplex tree corresponding 
* to the simplex with this set of Vertices.
* 
* The sequence of Vertices must be sorted in 
* increasing order.
*
* If the simplex is not in the simplex tree, returns end().
*
* \todo Simplex_tree find and insert.
*/
//template <class RandomAccessVertexRange >
Simplex_handle find(std::vector< Vertex > & s)
{ 
  if(s.begin() == s.end()) std::cerr << "Empty simplex \n";

  sort(s.begin(),s.end());

  Siblings *     tmp_sib = &root_;
  Dictionary_it  tmp_dit;
  Vertex last = s[s.size()-1];
  for(auto v : s) {
    tmp_dit = tmp_sib->members_.find(v);
    if(tmp_dit == tmp_sib->members_.end())   {return null_simplex();}
    if( !has_children(tmp_dit) && v != last) {return null_simplex();}
    tmp_sib = tmp_dit->second.children();
  }
return tmp_dit;
}   

/** \brief Faster way to find a vertex*/
Simplex_handle find_vertex(Vertex v)
{ return root_.members_.begin()+v; }

/** \todo Simplex_tree::insert() */
//Simplex_handle insert(); //input a vertex_range

/** \todo flat_map and valid iterators ?*/
Simplex_handle null_simplex() {return root_.members_.end(); }

Vertex null_vertex() { return -1; }

/** Returns a pointer to the root nodes of the simplex tree.*/
Siblings *      root()          { return &root_; }
/** Returns a pointer to the geometry traits.*/
MetricSpace *   ms()            { return ms_; }
/** Returns the number of vertices in the complex.*/
size_t          nb_vertices()   { return root_.members_->size(); }
/** \brief Returns the number of faces of the complex.*/
size_t          nb_simplices()  { return size_cpx_; }

/** \brief Initializes the filtrations, i.e. inserts a Simplex_handle 
* for every simplex in the simplicial complex and sort the
* simplices according to their order in the filtration.
*
* The use of a depth-first traversal of the simplex tree, provided by 
* complex_simplex_range(), combined with
* a stable sort is meant to optimize the order of simplices with same
* filtration value. The heuristic consists in inserting the cofaces of a
* simplex as soon as possible.
*
* \todo Check if the heuristic is efficient. How should we deal with vertices? 
*
* Will be automatically called when calling filtration_simplex_range()
* if the filtration has not been initialized yet.*/ 
void initialize_filtration()
{ filtration_vect_.reserve(size_cpx_);
  for(auto sh : complex_simplex_range()) { filtration_vect_.push_back(sh);}
  stable_sort(filtration_vect_.begin(),filtration_vect_.end(),is_before_in_filtration(this));};
/** \brief Returns true iff sh1 is a subface of sh2.*/
bool is_subface(Simplex_handle sh1, Simplex_handle sh2)
{ Simplex_vertex_range    rg1 = simplex_vertex_range(sh1);
  Simplex_vertex_range    rg2 = simplex_vertex_range(sh2);
  Simplex_vertex_iterator it1 = rg1.begin();
  Simplex_vertex_iterator it2 = rg2.begin();
  while(it1 != rg1.end() && it2 != rg2.end()) {
      if(*it1 < *it2) {++it2;}
      else { if(*it1 == *it2) {++it1; ++it2;}
             else {return false;} }
    }
  return (it1 == rg1.end());}
/** \brief Strict weak ordering corresponding to the partial order
* induced by the filtration.*/
struct is_before_in_filtration {
  is_before_in_filtration(Simplex_tree * st) : st_(st) {}
  bool operator()(const Simplex_handle sh1,
                  const Simplex_handle sh2) const 
  { if(sh1->second.filtration() != sh2->second.filtration())
    {return sh1->second.filtration() < sh2->second.filtration();}
    return !(st_->is_subface(sh1,sh2)); }  //is sh1 a subface of sh2
    
  Simplex_tree * st_;
};

/**
 * \brief Inserts a 1-skeleton in an empty Simplex_tree.
 *
 * Inserts all vertices and edges given by NeighborGraph, 
 * which is a model of boost::AdjacencyGraph whose graph has
 * vertices the Vertices of MetricSpace.
 *
 * \todo With a flat_map, ms_->space_vertex_range needs to be
 * a boost::container::ordered_unique_range.
 *
 * \todo Works on a range of points. + easily parallelizable.
 * attention static vector => not thread safe in intersection.
 */
template< class NeighborGraph >
void insert_graph( NeighborGraph & ng )
{
  root_.members_.reserve(ng.size_graph());
  for(auto v : ms_->space_vertex_range())
  { root_.members_.insert(root_.members_.end(),
                          std::pair<Vertex,Node>(v,Node(&root_,0.))); }
  for(Dictionary_it sh = root_.members().begin(); //all root simplices, ie vertices.
      sh != root_.members().end(); ++sh) 
  {  //traverses the neighbor vertices
    for(auto neigh : ng.adjacent_vertices(sh->first)) 
    {
      if(sh->first < neigh) 
      {
        if(! has_children(sh))
        { sh->second.assign_children(new Siblings(&root_,sh->first)); }
        sh->second.children()->members()[neigh] = 
                                        Node(sh->second.children(),
                                             ms_->distance(sh->first,neigh));
      }
    }
  }
 // Update size of the complex
  this->size_cpx_ += this->root_.size();
  for(Simplex_handle sh = root_.members().begin(); //all root simplices, ie vertices.
    sh != root_.members().end(); ++sh) 
  { if(has_children(sh)) 
    { size_cpx_ += sh->second.children()->size(); }  }
}
/**
  * \brief Expands the Simplex_tree containing only a graph
  * until dimension max_dim.
  *
  * The expanded simplicial complex until dimension \f$d\f$ 
  * attached to a graph \f$G\f$ is the maximal simplicial complex of 
  * dimension at most \f$d\f$ admitting the \f$G\f$ as \f$1\f$-skeleton.
  * The filtration value assigned to a simplex is the maximal filtration
  * value of one of its edges.
  *
  * \todo What if more than a graph in the complex?
  */
void expansion(int max_dim)
{
  for(Dictionary_it root_it = root_.members_.begin();
      root_it != root_.members_.end(); ++root_it)
  {
    if(has_children(root_it)) 
    { siblings_expansion(root_it->second.children(), max_dim-1); }
  }
}
private:
// Recursive expansion of the simplex tree.
/** \todo Not thread-safe: use non-static?*/
void siblings_expansion(Siblings * siblings, //must contain elements
                        int k)
{
  if(k == 0) return;
  Dictionary_it next = siblings->members().begin(); ++next;

  static std::vector< std::pair<Vertex , Node> > inter; // <-------static
  for(Dictionary_it s_h = siblings->members().begin();
      s_h != siblings->members().end(); ++s_h,++next)
    {
      Simplex_handle root_sh = find_vertex(s_h->first);//root_.members_[s_h->first]; //<-- simplex ahndle 
      if(has_children(root_sh))
      {
        intersection(inter,  //output intersection
                     next,                     //begin
                     siblings->members().end(),//end
                     root_sh->second.children()->members().begin(),
                     root_sh->second.children()->members().end(),
                     s_h->second.filtration());
        if(inter.size() != 0)
        { this->size_cpx_ += inter.size();
          Siblings * new_sib = new Siblings(siblings,   //oncles
                                            s_h->first, //parent
                                            inter);     //boost::container::ordered_unique_range_t
          inter.clear();
          s_h->second.assign_children(new_sib);
          siblings_expansion(new_sib,k-1);}
        else { s_h->second.assign_children(siblings); //ensure the children property
               inter.clear();}
      }
    }
}
// Intersects Dictionary 1 [begin1;end1) with Dictionary 2 [begin2,end2) 
// and assigns the maximal possible Filtration_value to the Nodes.
void intersection(std::vector< std::pair< Vertex, Node > > &   intersection,
                  Dictionary_it                                begin1,
                  Dictionary_it                                end1,
                  Dictionary_it                                begin2,
                  Dictionary_it                                end2,
                  Filtration_value                             filtration)
{
  if(begin1 == end1 || begin2 == end2) return;// 0;
  while( true ) 
  {
    if( begin1->first == begin2->first )
    {
      intersection.push_back(std::pair< Vertex, Node >(begin1->first,
                             Node(NULL,
                                  maximum(begin1->second.filtration(),
                                          begin2->second.filtration(),
                                          filtration))));
      ++begin1;  ++begin2;
      if( begin1 == end1 || begin2 == end2 ) return;
    }
    else 
    { 
      if( begin1->first < begin2->first ) 
      { ++begin1; if(begin1 == end1) return; }
      else { ++begin2; if(begin2 == end2) return;}
    }
  }
}    
// Maximum over 3 values.
Filtration_value maximum(Filtration_value a, 
                         Filtration_value b, 
                         Filtration_value c )
{ Filtration_value max = ( a < b ) ? b : a;
  return ( ( max < c ) ? c : max ); }

/** \brief Pointer to a metric space. */
  MetricSpace *                    ms_             ;
/** \brief Threshold for the filtration function. */
//  Filtration_value                 rho_max_        ;
/** \brief Number of vertices. The set of vertices is static.*/      
  int                              nb_vertices_    ;
/** \brief Total number of simplices in the complex, without the empty simplex.*/
  int                              size_cpx_       ;
/** \brief Set of simplex tree Nodes representing the vertices.*/  
//  std::vector< Node >              root_           ;
  Siblings                         root_;
/** \brief Simplices ordered according to a filtration*/  
  std::vector< Simplex_handle >    filtration_vect_;
/** \brief A NULL Simplex_handle; useful for the implementation.*/  
//    Simplex_handle                   NULL_sh_        ;
};

#include "Simplex_tree.hpp"         //implementation of the methods in
                                    // Simplex_tree.
#endif // GUDHI_FLAG_SIMPLEX_TREE_H
