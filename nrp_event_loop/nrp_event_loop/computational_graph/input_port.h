/* * NRP Core - Backend infrastructure to synchronize simulations
 *
 * Copyright 2020-2021 NRP Team
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

#ifndef INPUT_PORT_H
#define INPUT_PORT_H

#include <functional>
#include <sstream>

#include "nrp_general_library/utils/nrp_exceptions.h"

#include "nrp_event_loop/computational_graph/port.h"
#include "nrp_event_loop/computational_graph/output_port.h"
#include "nrp_event_loop/computational_graph/computational_node.h"

/*!
 * \brief Template for data conversion functions used in InputPorts upon new message arrival.
 *
 * Concrete conversions are implemented as specializations
 */
template <class T_IN, class T_OUT>
struct dataConverter {
    static void convert(const T_IN *d1, T_OUT &d2) {
        std::stringstream s;
        s << "data conversion from " << typeid(T_IN).name() << " to type " << typeid(T_OUT).name()
          << " is not implemented.";
        throw NRPException::logCreate(s.str());
    }
};

/*!
 * \brief Implementation of an input port in the computation graph
 *
 * It converts and passes incoming msgs using a callback function
 */
template <class T_IN, class T_OUT>
class InputPort : public Port {
public:

    /*!
     * \brief Constructor
     */
    InputPort(const std::string& id, ComputationalNode* parent, std::function<void(const T_OUT*)> callback, std::size_t max_subs = 0) :
            Port(id, parent),
            _callback(callback),
            _max_subs(max_subs)
    {}

    /*!
     * \brief Subscribes this port to 'port'
     */
    void subscribeTo(OutputPort<T_IN>* port)
    {
        if(_max_subs && _n_subs >= _max_subs) {
            std::stringstream s;
            s << "Port \"" << this->id() << "\" of node \"" << this->parent()->id() << "\" can only have " << _max_subs << " subscriber(s)";
            throw NRPException::logCreate(s.str());
        }

        using std::placeholders::_1;
        std::function<void(const T_IN*)> receive_f = std::bind(&InputPort<T_IN,T_OUT>::receive, this, _1);
        port->add_subscriber(receive_f);

        _n_subs++;
    }

    /*!
     * \brief Return the number ports this port is subscribed to
     */
    size_t subscriptionsSize() override
    { return _n_subs; }

private:

    /*!
     * \brief Main function processing incoming msgs
     *
     * It is used for connecting this port to output ports
     */
    void receive(const T_IN* msg)
    {
        if constexpr ( std::is_same_v<T_IN, T_OUT> )
            _callback(msg);
        else {
            dataConverter<T_IN, T_OUT>::convert(msg, data);
            _callback(&data);
        }
    }

    /*! \brief Converted data owned by the port. Default constructor for T_OUT is assumed */
    T_OUT data;
    /*! \brief Callback function used to forward incoming msgs */
    std::function<void(const T_OUT*)> _callback;
    /*! \brief Maximum number of subscribers accepted by this node. 0 means no limit */
    std::size_t _max_subs;
    std::size_t _n_subs = 0;
};


#endif //INPUT_PORT_H
