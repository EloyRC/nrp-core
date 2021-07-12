
#include <nav_msgs/GetMapFeedback.h>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>


namespace nav_msgs
{


using namespace boost::python;
using boost::shared_ptr;
using std::vector;


// Dummy equality check to avoid compilation error for vector_indexing_suite
bool operator== (const GetMapFeedback& m1, const GetMapFeedback& m2)
{
  return false;
}

void exportGetMapFeedback ()
{
  using nav_msgs::GetMapFeedback;
  class_<GetMapFeedback, shared_ptr<GetMapFeedback> >("GetMapFeedback", "# ====== DO NOT MODIFY! AUTOGENERATED FROM AN ACTION DEFINITION ======\n# no feedback\n")
    ;

}

} // namespace