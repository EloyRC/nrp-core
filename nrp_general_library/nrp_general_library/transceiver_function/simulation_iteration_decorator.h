/*
 * NRP Core - Backend infrastructure to synchronize simulations
 *
 * Copyright 2020-2023 NRP Team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This project has received funding from the European Union’s Horizon 2020
 * Framework Programme for Research and Innovation under the Specific Grant
 * Agreement No. 945539 (Human Brain Project SGA3).
 */

#ifndef SIMULATION_ITERATION_DECORATOR_H
#define SIMULATION_ITERATION_DECORATOR_H

#include "nrp_general_library/transceiver_function/transceiver_datapack_interface.h"
#include "nrp_general_library/transceiver_function/function_manager.h"

#include <string>
#include <boost/python.hpp>

/*!
 * \brief Class for retrieving simulation iteration in Functions, mapped to SimulationIteration python decorator
 */
class SimulationIterationDecorator
    : public TransceiverDataPackInterface
{
    public:
        SimulationIterationDecorator(const std::string &keyword)
            : _keyword(keyword)
        {

        }

        boost::python::object runTf(boost::python::tuple &args, boost::python::dict &kwargs, datapacks_set_t dataPacks) override
        {
            kwargs[this->_keyword] = this->getFunctionManager()->getSimulationIteration();

            return TransceiverDataPackInterface::runTf(args, kwargs, dataPacks);
        }

    private:

        std::string _keyword;
};

#endif // SIMULATION_ITERATION_DECORATOR_H

// EOF
