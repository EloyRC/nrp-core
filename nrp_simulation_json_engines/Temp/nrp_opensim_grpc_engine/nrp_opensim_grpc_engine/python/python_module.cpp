//
// NRP Core - Backend infrastructure to synchronize simulations
//
// Copyright 2020 Michael Zechmair
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This project has received funding from the European Union’s Horizon 2020
// Framework Programme for Research and Innovation under the Specific Grant
// Agreement No. 945539 (Human Brain Project SGA3).
//

#include "nrp_opensim_grpc_engine/devices/grpc_opensim_physics_joint.h"

#include "nrp_general_library/device_interface/device.h"
#include "nrp_general_library/config/cmake_constants.h"

#include "nrp_opensim_grpc_engine/config/cmake_constants.h"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
using namespace boost::python;

/*BOOST_PYTHON_MODULE(OPENSIM_PYTHON_MODULE_NAME)
{
	import(PYTHON_MODULE_NAME_STR);

	class_<OpensimPhysicsJoint, bases<DeviceInterface> >("OpensimPhysicsJoint", init<const std::string &>())
	        .add_property("position", &OpensimPhysicsJoint::position, &OpensimPhysicsJoint::setPosition)
	        .add_property("velocity", &OpensimPhysicsJoint::velocity, &OpensimPhysicsJoint::setVelocity)
	        .add_property("acceleration", &OpensimPhysicsJoint::effort, &OpensimPhysicsJoint::setEffort);

	register_ptr_to_python<std::shared_ptr<OpensimPhysicsJoint> >();
	register_ptr_to_python<std::shared_ptr<const OpensimPhysicsJoint> >();
}*/