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

#ifndef PROTO_FIELD_OPS_H
#define PROTO_FIELD_OPS_H

#include "google/protobuf/message.h"
#include <boost/python.hpp>
#include "nrp_general_library/utils/nrp_exceptions.h"

namespace bpy = boost::python;
namespace gpb = google::protobuf;

template<class T>
concept PROTO_MSG_C = (std::is_base_of_v<google::protobuf::Message, T>);

/*!
 * \brief Implement single field Get/Set operations using field descriptor and reflection interface.
 */
namespace proto_field_ops {

    /*!
     * \brief Get scalar field. Returns a copy of the field value
     */
    bpy::object GetScalarField(gpb::Message &m, const gpb::FieldDescriptor *field);

    /*!
     * \brief Get repeated scalar field. Returns a copy of the field value
     */
    bpy::object GetRepeatedScalarField(gpb::Message &m, const gpb::FieldDescriptor *field, int index);

    /*!
     * \brief Get message field. Returns a reference of the field value
     */
    template<PROTO_MSG_C MSG, PROTO_MSG_C ...REMAINING_MSGS>
    bpy::object GetMessageField(gpb::Message &m, const gpb::FieldDescriptor *field) {
        MSG *msg_field = dynamic_cast<MSG *>(m.GetReflection()->MutableMessage(&m, field));
        if(msg_field != nullptr) {
            typename bpy::reference_existing_object::apply<MSG *>::type convert;
            return bpy::object(bpy::handle<>(convert(msg_field)));
        }

        if constexpr (sizeof...(REMAINING_MSGS) > 0)
            return GetMessageField<REMAINING_MSGS...>(m, field);
        else
            throw NRPException::logCreate("Unable to get composite field with name " + field->name());
    }

    /*!
     * \brief Set scalar field
     */
    void SetScalarField(gpb::Message &m, const gpb::FieldDescriptor *field, const bpy::object &value);

    /*!
     * \brief Set repeated scalar field
     */
    void SetRepeatedScalarField(gpb::Message &m, const gpb::FieldDescriptor *field, const bpy::object &value, int index);

    /*!
     * \brief Append repeated scalar field
     */
    void AddRepeatedScalarField(gpb::Message &m, const gpb::FieldDescriptor *field, const bpy::object &value);

}

#endif // PROTO_FIELD_OPS_H