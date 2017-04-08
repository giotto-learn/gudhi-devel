#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

#include <gudhi/time_series_analysis/sliding_window.h>

using namespace Gudhi;
using namespace Gudhi_stat;


int main( int argc , char** argv )
{
	//this array contains value of sin(x_n) for x_0 = 0 and x_{n+1} = _{n} + 0.1 for n \in {0,115-23}.
	static const double arr[] = {0, 0.0998334166, 0.1986693308, 0.2955202067, 0.3894183423, 0.4794255386, 0.5646424734, 0.6442176872, 0.7173560909, 0.7833269096, 0.8414709848, 0.8912073601, 0.932039086, 0.9635581854, 0.98544973,
	0.9974949866, 0.999573603, 0.9916648105, 0.9738476309, 0.9463000877, 0.9092974268, 0.8632093666, 0.8084964038, 0.7457052122, 0.6754631806, 0.5984721441, 0.5155013718, 0.4273798802, 0.3349881502,
	0.2392493292, 0.1411200081, 0.0415806624, -0.0583741434, -0.1577456941, -0.255541102, -0.3507832277, -0.4425204433, -0.5298361409, -0.6118578909, -0.6877661592, -0.7568024953, -0.8182771111, -0.8715757724,
	-0.9161659367, -0.9516020739, -0.9775301177, -0.9936910036, -0.9999232576, -0.9961646088, -0.9824526126, -0.9589242747, -0.9258146823, -0.8834546557, -0.8322674422, -0.7727644876, -0.7055403256,
	-0.6312666379, -0.5506855426, -0.4646021794, -0.3738766648, -0.2794154982, -0.1821625043, -0.0830894028, 0.0168139005, 0.1165492049, 0.2151199881, 0.3115413635, 0.4048499206, 0.4941133511, 0.5784397644,
	0.6569865987, 0.7289690401, 0.7936678638, 0.8504366206, 0.8987080958, 0.9379999768, 0.967919672, 0.9881682339, 0.9985433454, 0.9989413418, 0.9893582466, 0.9698898108, 0.9407305567, 0.9021718338, 0.8545989081,
	0.7984871126, 0.7343970979, 0.6629692301, 0.5849171929, 0.5010208565, 0.4121184852, 0.3190983623, 0.2228899141, 0.1244544235, 0.0247754255, -0.0751511205, -0.1743267812, -0.2717606264, -0.3664791293,
	-0.4575358938, -0.5440211109, -0.6250706489, -0.6998746876, -0.7676858098, -0.8278264691, -0.87969576, -0.9227754216, -0.9566350163, -0.9809362301, -0.9954362533, -0.9999902066, -0.9945525882, -0.9791777292,
	-0.9540192499, -0.9193285257, -0.8754521747};
     vector< double > time_series (arr, arr + sizeof(arr) / sizeof(arr[0]) );
     
     //construct sliding window embeddign to dimension 3 based on time_series:
     Sliding_window_embedding swe( time_series , 3 );
     swe.create_point_cloud("pt_cloud");
     
     //persistent_cohomology::Persistent_cohomology<ST, Field_Zp >& a = 
     swe.compute_persistence_of_Vietoris_Rips_complex( 1 , 2 ); 
     
     
     return 0;
}