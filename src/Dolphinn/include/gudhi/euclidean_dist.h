#ifndef EUCLIDEAN_DIST_H
#define EUCLIDEAN_DIST_H

#include <vector>
#include <random>
#include <math.h>
#include <CGAL/Cartesian_d.h>



namespace Gudhi {

/** \brief Euclidean distance squared.
 *
 * @param it1       - first point
 * @param it1_end   - end of first point
 * @param it2       - second point
 * @return          - the Euclidean distance of p1-p2
 */
template<typename iterator>
float squared_Eucl_distance(iterator p1, iterator p2)
{
	float res=0;
	float tmp;
	for(size_t i=0;i<(*p1).size();++i){
		tmp = (*p1)[i] - (*p2)[i];
		res += tmp*tmp;
	}
  return res;
}

/** \brief Report a point's index (if any) that has Euclidean distance
 * less or equal than a given radius.
 *
 * @param pointset        - 1D vector of all points
 * @param points_idxs     - indices of candidate points
 * @param D               - dimension of points
 * @param query_point     - vector containing only the coordinates of the query point
 * @param squared_radius  - square value of given radius
 * @param threshold       - max number of points to check
 * @return                - the index of the point. -1 if not found.
 */
template <typename iterator>
int Euclidean_distance_within_radius(iterator pointset, const std::vector<int>& points_idxs,
 const int D, iterator query_point, const float squared_radius, const int threshold)
{
  const int size = points_idxs.size();
  for(int i = 0; i < threshold && i < size; ++i)
  {
    if(squared_Eucl_distance(query_point, pointset + points_idxs[i]) <= squared_radius)
      return points_idxs[i];
  }
  return -1;
}

/** \brief Report Nearest Neighbor's index, if something better than the current NN is found.
 *
 * @param pointset              - 1D vector of all points
 * @param points_idxs           - indices of candidate points
 * @param D                     - dimension of points
 * @param query_point           - vector containing only the coordinates of the query point
 * @param answer_point_idx_dist - current best NN point. Will be updated if a point closer to the query is found.
 * @param threshold             - max number of points to check
 */
template <typename iterator>
void find_Nearest_Neighbor_index(iterator pointset, const std::vector<int>& points_idxs,
 const int D, iterator query_point, std::pair<int, float>& answer_point_idx_dist, const int threshold)
{
  const int size = points_idxs.size();
  float current_dist;
  for(int i = 0; i < threshold && i < size; ++i)
  {
    current_dist = squared_Eucl_distance(query_point, pointset + points_idxs[i]);
    if(current_dist < answer_point_idx_dist.second)
    {
      answer_point_idx_dist.second = current_dist;
      answer_point_idx_dist.first = points_idxs[i];
    }
  }
}

/** \brief Report M Nearest Neighbors' indices, if something better than the current NN is found.
 *
 * @param pointset              - 1D vector of all points
 * @param points_idxs           - indices of candidate points
 * @param D                     - dimension of points
 * @param M
 * @param query_point           - vector containing only the coordinates of the query point
 * @param answer_point_idx_dist - current best NN points. Will be updated if a point closer to the query is found.
 * @param threshold             - max number of points to check
 */
template <typename iterator>
void find_M_Nearest_Neighbor_indices(iterator pointset, const std::vector<int>& points_idxs,
 const int D, const int M, iterator query_point, std::vector<std::pair<int, float>>& answer_point_idx_dist, const int threshold)
{
  const int size = points_idxs.size();
  float current_dist;
  current_dist = answer_point_idx_dist[0].second;
  int current_j=0;
  for(int j=1; j<M; ++j){
	  if(current_dist < answer_point_idx_dist[j].second){
		  current_dist=answer_point_idx_dist[j].second;
		  current_j=j;
	  }
  }
  for(int i = 0; i < threshold && i < size; ++i)
  {
    current_dist = squared_Eucl_distance(query_point, pointset + points_idxs[i]);
    if(current_dist < answer_point_idx_dist[current_j].second)
    {
      answer_point_idx_dist[current_j].second = current_dist;
      answer_point_idx_dist[current_j].first = points_idxs[i];
      for(int j=0; j<M; ++j){
		  if(current_dist < answer_point_idx_dist[j].second){
			  current_dist=answer_point_idx_dist[j].second;
			  current_j=j;
		  }

		}
    }
  }
}

/** \brief Report a point's index (if any) that has Euclidean distance
 * less or equal than a given radius. Usage in a parallel environment.
 *
 * @param pointset          - 1D vector of all points
 * @param points_idxs       - indices of candidate points
 * @param start_points_idxs - where to start checking
 * @param end_points_idxs   - where to stop checking
 * @param D                 - dimension of points
 * @param query_point       - vector containing only the coordinates of the query point
 * @param squared_radius    - square value of given radius
 * @param threshold         - max number of points to check
 * @param answer_idx        - the index of the point. -1 if not found.
 */
template <typename iterator>
void Euclidean_distance_within_radius(iterator pointset, const std::vector<int>& points_idxs,
  const int start_points_idxs, const int end_points_idxs,
  const int D, iterator query_point, const float squared_radius, const int threshold, int& answer_idx)
{
  answer_idx = -1;
  for(int i = start_points_idxs; i < threshold && i < end_points_idxs; ++i)
  {
    if(squared_Eucl_distance(query_point, pointset + points_idxs[i]) <= squared_radius)
    {
      answer_idx = points_idxs[i];
      break;
    }
  }
}
}

#endif /*EUCLIDEAN_DIST_H*/