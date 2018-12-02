#include <gudhi/TowerAssembler.h>
#include <gudhi/Rips_complex.h>
#include <gudhi/Rips_edge_list.h>
#include <gudhi/distance_functions.h>
#include <gudhi/reader_utils.h>
#include <gudhi/PointSetGen.h>

using Fake_simplex_tree   = Gudhi::Fake_simplex_tree ;
using Simplex_tree        = Gudhi::Simplex_tree<>;
using Filtration_value    = Fake_simplex_tree::Filtration_value;
using Rips_complex        = Gudhi::rips_complex::Rips_complex<Filtration_value>;
using Rips_edge_list      = Gudhi::rips_edge_list::Rips_edge_list<Filtration_value>;
using Distance_matrix     = std::vector<std::vector<Filtration_value>>;

using Vector_of_points         = std::vector<Point>;
using Vector_of_SM_pointers    = std::vector<SparseMsMatrix*>;

class extract_sub_rips_complex
{
    public:
        template<class Filtered_sorted_edge_list, class Simplicial_complex, class Fil_vector >
        extract_sub_rips_complex(double threshold, Simplicial_complex & currentComplex, Filtered_sorted_edge_list & edge_t, Fil_vector & edge_filt ) {
            size_t end_idx =  (edge_t.size()-1);
            // std::cout << "The size of the edge list is " << (end_idx+1) << std::endl;
            // std::cout << "With filtration value range (" << std::get<0>(edge_t.at(0)) << ", " <<std::get<0>(edge_t.at(end_idx)) << ") " <<std::endl;
            auto bgn_it = std::lower_bound(edge_filt.begin(), edge_filt.end(), threshold); // find_index(edge_t, threshold, 0, end_idx);
            size_t bgn_idx = std::distance(edge_filt.begin(),bgn_it);
            // std::cout<< "The returned begin index is " << bgn_idx << std::endl;
            // std::cout << "The edge filteration values in the nbd of the returned index (" << std::get<0>(edge_t.at(bgn_idx-1)) << ", " << std::get<0>(edge_t.at(bgn_idx)) <<  ", " <<std::get<0>(edge_t.at(bgn_idx+1)) << ") " << std::endl;
            if(bgn_idx < edge_filt.size())
                for( ;end_idx >= bgn_idx; --end_idx) {
                   std::vector<size_t>  s = {std::get<1>(edge_t.at(end_idx)), std::get<2>(edge_t.at(end_idx))};
                   currentComplex.remove_simplex(s);
                   edge_t.erase(edge_t.begin()+end_idx);
                   edge_filt.erase(edge_filt.begin()+end_idx);
                }
        }
};


int main(int argc, char * const argv[]) {
	
    auto the_begin = std::chrono::high_resolution_clock::now();
    PointSetGen point_generator;
    std::string out_file_name   = "default";
    std::string in_file_name    = "default";
    std::size_t number_of_points;
    
    typedef size_t Vertex_handle;
    typedef std::vector< std::tuple<Filtration_value, Vertex_handle, Vertex_handle > > Filtered_sorted_edge_list;
    std::vector<Filtration_value> * edge_filt = new std::vector<Filtration_value>() ;

    int     dimension;
    double  begin_thresold;
    double  end_thresold;
    double  min_dist;
    double  max_dist;
    double  steps;
    int     repetetions = 1;
    char    manifold;

    Vector_of_points * point_vector;
    Vector_of_points file_all_points;

    Vector_of_SM_pointers allSparseMatrices;
    // Vector_of_SM_pointers allSparseMatrices_2;
    
    int dim_max = 20000; // pseudo variable... of no use
    std::string manifold_full = "sphere";
    
    double radius  = 1;
    double r_min = 0.6;
    double r_max = 1;

    point_generator.program_options(argc, argv, number_of_points, begin_thresold, steps, end_thresold, repetetions, manifold, dimension, in_file_name, out_file_name);
    
    std::cout << "The current input values to run the program is: "<< std::endl;
    std::cout << "number_of_points, begin_thresold, steps, end_thresold, repetetions, manifold, dimension, in_file_name, out_file_name" << std::endl;
    std::cout << number_of_points << ", " << begin_thresold << ", " << steps << ", " << end_thresold << ", " << repetetions << ", " << manifold << ", " << dimension << ", " << in_file_name << ", " << out_file_name << std::endl;
    
    if(manifold == 'f' || manifold =='F') {
        Gudhi::Points_off_reader<Point> off_reader(in_file_name);
        if (!off_reader.is_valid()) {
            std::cerr << "Unable to read file " << in_file_name << "\n";
            exit(-1);  // ----- >>
        }

        file_all_points = Vector_of_points(off_reader.get_point_cloud());
        dimension = file_all_points[0].dimension() ;
        std::cout << "Successfully read " << file_all_points.size() << " point_vector.\n";
        std::cout << "Ambient dimension is " << dimension << ".\n";
    }
   
    Map map_empty;

    std::string origFile ("./PersistenceOutput/original_tower_rips" );
    std::string collFile  ("./PersistenceOutput/collapsed_tower_rips") ;
    // std::string collFile_2  ("./PersistenceOutput/collapsed_tower_rips_2") ;
    
    std::string otherStats ("./PersistenceOutput/maximal_simplx_cnt");
    otherStats = otherStats+"_"+ out_file_name+ ".txt";

    double currentCollapseTime = 0.0;
    double totCollapseTime = 0.0;
    double maxCollapseTime = 0.0;
    double totAssembleTime = 0.0;

    double currentCreationTime = 0.0;
    // double totCreationTime = 0.0;
    double maxCreationTime     = 0.0;
    double maxCreationAndcollapseTime = 0.0;
   
    point_vector = new Vector_of_points();
    Distance_matrix distances;

    if(manifold == 's' || manifold == 'S'){
        point_generator.generate_points_sphere(*point_vector, number_of_points, dimension, radius);
        origFile = origFile+"_sphere_"+out_file_name+".txt";
        collFile = collFile+"_sphere_"+out_file_name+".txt";
        std::cout << number_of_points << " points successfully chosen randomly from "<< dimension <<"-sphere of radius " << radius << std::endl;
    }
    else if(manifold == 'b' || manifold == 'B'){
        point_generator.generate_points_ball(*point_vector, number_of_points, dimension, radius); 
        origFile = origFile+"_ball_"+out_file_name+".txt";
        collFile = collFile+"_ball_"+out_file_name+".txt";
        std::cout << number_of_points << " points successfully chosen randomly from "<< dimension <<"-ball of radius " << radius << std::endl;
    
    }
    else if( (manifold == 'a' || manifold == 'A')&& dimension == 2){
        point_generator.generate_points_2annulus(*point_vector, number_of_points, r_min, r_max); 
        origFile = origFile+"_annulus_"+out_file_name+".txt";
        collFile = collFile+"_annulus_"+out_file_name+".txt";
        std::cout << number_of_points << " points successfully chosen randomly from "<< 2 <<"-annulus of radii (" << r_min << ',' << r_max << ") " << std::endl;
    }
    else if( (manifold == 'a' || manifold == 'A') && dimension == 3){
        point_generator.generate_points_spherical_shell(*point_vector, number_of_points, r_min, r_max); 
        origFile = origFile+"_annulus_"+out_file_name+".txt";
        collFile = collFile+"_annulus_"+out_file_name+".txt";
        std::cout << number_of_points << " points successfully chosen randomly from spherical shell of radii (" << r_min << ',' << r_max << ") " << std::endl;
    }
    
    else if(manifold == 'f' || manifold =='f') {
        // Subsampling from all points for each iterations
        Gudhi::subsampling::pick_n_random_points(file_all_points, number_of_points, std::back_inserter(*point_vector));
        origFile = origFile+"_"+ out_file_name+ ".txt";
        collFile = collFile+"_"+ out_file_name+ ".txt";
        std::cout << number_of_points << " points succesfully chosen randomly of dimension "<< dimension << " ." << std::endl;
    }
    else if (manifold == 'm'){
        std::string csv_file_name(in_file_name);
        distances = read_lower_triangular_matrix_from_csv_file<Filtration_value>(csv_file_name);
        number_of_points = distances.size();
        std::cout << "Read the distance matrix succesfully, of size: " << number_of_points << std::endl;
        origFile = origFile+"_"+ out_file_name+ ".txt";
        collFile = collFile+"_"+ out_file_name+ ".txt";
    }
    else {
        std::cerr << "Wrong parameters for input manifold..." <<std::endl;  
        exit(-1); 
    }
    std::cout << "Point Set Generated."  <<std::endl;
    //Preparing the statsfile to record the reduction in num of maximal simplices and the dimension of the complex.
    std::ofstream statsfile (otherStats, std::ios::app);
    if(statsfile.is_open()){
        statsfile << " #number_of_points, begin_thresold, steps, end_thresold, repetetions, manifold, dimension, in_file_name, out_file_name" << std::endl;
        statsfile << " Original_maximal_simplex, Original_complex_dimension, Collapsed_maximal_simplex, Collapsed_complex_dimension" << std::endl;
    }
    else {
        std::cerr << "Unable to open stats file";
        exit(-1) ;
    }

    // for(int i = 0; i < number_of_points; i++ )
    //     point_generator.print_point(point_vector->at(i));

    Fake_simplex_tree * subComplex  = new Fake_simplex_tree();  
    Filtered_sorted_edge_list * edge_t = new Filtered_sorted_edge_list();
    TowerAssembler twr_assembler(number_of_points) ;
    
    std::cout << "Computing the one-skeleton and the Rips complex for threshold: " << end_thresold << std::endl; 
    
    auto begin_full_cmplx = std::chrono::high_resolution_clock::now();
    if(manifold == 'm'){ //Input is a distance 'm'atrix
        //Creating the edge list
        Rips_edge_list Rips_edge_list_from_file(distances, end_thresold);
        Rips_edge_list_from_file.create_edges(*edge_t);
        std::cout<< "Sorted edge list computed" << std::endl;

        //Creating the Rips Complex
        Rips_complex rips_complex_from_file(distances, end_thresold);
        rips_complex_from_file.create_complex(*subComplex, dim_max);
        std::cout<< "Rips complex computed" << std::endl;
 
        allSparseMatrices.emplace_back(new SparseMsMatrix(*subComplex));
    }
    else{ //Point cloud input
         //Creating the edge list
        Rips_edge_list Rips_edge_list_from_points(*point_vector, end_thresold, Gudhi::Euclidean_distance());
        Rips_edge_list_from_points.create_edges(*edge_t);
        std::cout<< "Sorted edge list computed" << std::endl;
        //Creating the Rips Complex
        Rips_complex rips_complex_from_points(*point_vector, end_thresold, Gudhi::Euclidean_distance());
        rips_complex_from_points.create_complex(*subComplex, dim_max);
        std::cout<< "Rips complex computed" << std::endl;

        allSparseMatrices.emplace_back(new SparseMsMatrix(*subComplex));
    }

    //An additional vector <edge_filt> to perform binary search to find the index of given threshold
    edge_filt->clear();
    for(auto edIt = edge_t->begin(); edIt != edge_t->end(); edIt++) {
        edge_filt->push_back(std::get<0>(*edIt));
    }
    min_dist = edge_filt->at(0);
    max_dist = edge_filt->at(edge_filt->size()-1);

    if(begin_thresold < min_dist){
        std::cout<< "Begin threshold re-set to the minimum filteration value, " << min_dist << "." <<std::endl;
    }
    int iterations = (end_thresold - min_dist)/steps;
    std::cout << "Total number of iterations to be run are: " << iterations << std::endl;

    auto end_full_cmplx = std::chrono::high_resolution_clock::now();
    currentCreationTime = std::chrono::duration<double, std::milli>(end_full_cmplx - begin_full_cmplx).count();
    maxCreationTime = currentCreationTime;
   
    //Extracting all the subcomplexes starting from the end_threshold-step till begin_thresold
    std::cout << "Extraction of all the subcomplexes begins. " << std::endl;
    auto threshold =  end_thresold-steps;
    
    while(threshold >= min_dist) {
        // std::cout << "Extracting for threshold: " << threshold << std::endl;
        auto begin_sub_cmplx = std::chrono::high_resolution_clock::now();
        extract_sub_rips_complex(threshold, *subComplex, *edge_t, *edge_filt);
        // std::cout << "Extraction completed. " << std::endl;
        // std::cout << "The new extracted subcomplex has: " << subComplex->num_simplices() << " maximal simplices. " << std::endl;
        allSparseMatrices.emplace_back(new SparseMsMatrix(*subComplex));
        
        auto end_sub_cmplx = std::chrono::high_resolution_clock::now();
        currentCreationTime = std::chrono::duration<double, std::milli>(end_sub_cmplx- begin_sub_cmplx).count();
        if(maxCreationTime < currentCreationTime)
            maxCreationTime = currentCreationTime;
        
        // std::cout << "  " << std::endl;
        threshold = threshold-steps;
    }

    auto end_all_cmplx = std::chrono::high_resolution_clock::now();
    auto totCreationTime = std::chrono::duration<double, std::milli>(end_all_cmplx- begin_full_cmplx).count();
    std::cout << "Extraction of all the subcomplexes completed. " << std::endl;
    //Adding one additional empty sparse_matrix at the end for tower assmebler
    allSparseMatrices.emplace_back(new SparseMsMatrix(number_of_points, 100*number_of_points));

    threshold = threshold+steps;
    int i = 1;
    size_t num_max_simp_unclp;
    int dim_unc;
    Map * redmap;

    for (auto it = allSparseMatrices.rbegin()+1; it != allSparseMatrices.rend(); ++it) {
        num_max_simp_unclp = (*it)->number_max_simplices();
        dim_unc = (*it)->initial_dimension();

        currentCollapseTime = (*it)->strong_collapse();

        redmap = new Map();
        *redmap = (*it)->reduction_map(); 
        
        // std::cout << " Subcomplex #" << i << " Collapsed" << std::endl;
        // std::cout << "Uncollapsed Rips complex is of dimension " << dim_unc << " with " << num_max_simp_unclp << " maximal simplices " << std::endl;
        // std::cout << "Collapsed Rips complex is of dimension " << (*it)->collapsed_dimension() << " with " <<  (*it)->number_max_simplices() << " maximal simplices" << std::endl;
        
        if( maxCollapseTime < currentCollapseTime)
         	maxCollapseTime = currentCollapseTime;
        totCollapseTime += currentCollapseTime ;

        if(statsfile.is_open()){
            statsfile << num_max_simp_unclp << "," << dim_unc << "," << (*it)->number_max_simplices() << "," << (*it)->collapsed_dimension() << std::endl;
        }
        else {
            std::cerr << "Unable to open stats file";
            exit(-1) ;
        }
        
        totAssembleTime += twr_assembler.print_tower_for_two_cmplxs(**(it-1), **(it), *redmap, threshold, collFile);
        
        // std::cout << "Tower updated for subcomplex #" << i << std::endl; 
        // std::cout << "** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** " << std::endl;
        
        threshold = threshold+steps;
        i++;
        delete redmap;
        delete *(it-1);
    }
    statsfile.close();
    maxCreationAndcollapseTime = maxCollapseTime+maxCreationTime;
    auto the_end = std::chrono::high_resolution_clock::now();
    auto totComputationTime = std::chrono::duration<double, std::milli>(the_end- the_begin).count();
    
    std::ofstream myfile (collFile, std::ios::app);
    if(myfile.is_open()){
        myfile << "# The input parameters for the experiment are: "<< std::endl;
        myfile << "# number_of_points, begin_thresold, steps, end_thresold, repetetions, manifold, dimension, in_file_name, out_file_name" << std::endl;
        myfile << "# "<< number_of_points << ", " << begin_thresold << ", " << steps << ", " << end_thresold << ", " << repetetions << ", " << manifold << ", " << dimension << ", " << in_file_name << ", " << out_file_name << std::endl;
        myfile << "# Maximum time taken for computation of all subcomplexes is: " << maxCreationTime << " ms" << std::endl;
        myfile << "# Total time taken for computation of all subcomplexes is: " << totCreationTime << " ms" << std::endl;
        myfile << "# Maximum time taken of all collapses is: " << maxCollapseTime << " ms" << std::endl;
        myfile << "# Total time taken in all collapses is: " << totCollapseTime << " ms" << std::endl;
        myfile << "# Total time taken to assemble the tower: " <<   totAssembleTime  <<" ms" << std::endl;
        myfile << "# Total Computation time by rips_persistence_with_sc is " <<   totComputationTime << " ms." <<std::endl;
        myfile.close();
    }
    else {
        std::cerr << "Unable to open file";
        exit(-1) ;
    }
    std::cout << "The output Tower is at the file location: " << collFile << std::endl;
    std::cout << "Maximum of (Creation + Collapse) TIME : " << maxCreationAndcollapseTime << " ms.\n";
    std::cout << "Maximum time taken for computation of all subcomplexes is: " << maxCreationTime << " ms" << std::endl;
    std::cout << "Total time taken for computation of all subcomplexes is: " << totCreationTime << " ms" << std::endl;
    std::cout << "Maximum time taken of all collapses is: " << maxCollapseTime << " ms" << std::endl;
    std::cout << "Total time taken in all collapses is: " << totCollapseTime << " ms." <<std::endl;
    std::cout << "Total time taken to assemble the tower: " <<   totAssembleTime << " ms." <<std::endl;
    std::cout << "** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** " << std::endl;
    std::cout << "Total Computation time by rips_persistence_with_sc is " <<   totComputationTime << " ms." <<std::endl;
    allSparseMatrices.clear();
    return 0;

}
  