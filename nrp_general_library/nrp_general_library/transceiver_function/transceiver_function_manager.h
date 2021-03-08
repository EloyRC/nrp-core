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

#ifndef TRANSCeivER_FUNCTION_MANAGER_H
#define TRANSCeivER_FUNCTION_MANAGER_H

#include "nrp_general_library/transceiver_function/transceiver_function_interpreter.h"

#include "nrp_general_library/utils/ptr_templates.h"

#include <set>
#include <list>

/*!
 * \brief Manages all available/active transfer functions
 */
class TransceiverFunctionManager
{
		/*!
		 * \brief Custom comparator function for json objects representing TF configurations
		 */
        struct less_tf_settings {
            bool operator() (const nlohmann::json &a, const nlohmann::json &b) const {
                return a.at("Name") < b.at("Name");
            }
        };

	public:

		using tf_settings_t = std::set<nlohmann::json, less_tf_settings>;
		using tf_results_t = std::list<TransceiverFunctionInterpreter::TFExecutionResult>;

		TransceiverFunctionManager() = default;
		TransceiverFunctionManager(boost::python::dict tfGlobals);

		/*!
		 * \brief Return list of devices that the TFs request
		 * \return Returns container with all requested device IDs
		 */
		EngineClientInterface::device_identifiers_set_t updateRequestedDeviceIDs() const;

		/*!
		 * \brief Load TF from given configuration
		 * \param tfConfig TF Configuration
		 * \exception Throws an exception if a TF with the same name is already loaded. Use updateTF to change loaded TFs
		 */
		void loadTF(const nlohmann::json &tfConfig);

		/*!
		 * \brief Updates an existing TF or creates a new one
		 * \param Name of old TF
		 * \param tfConfig TF Configuration
		 */
		void updateTF(const nlohmann::json &tfConfig);

		/*!
		 * \brief Executes all active TFs and records the results
		 * \param interpreter Python TF interpreter
		 * \return Returns the results of all Transfer Functions
		 */
		tf_results_t executeActiveTFs();

		/*!
		 * \brief Execute all TFs linked to an engine
		 * \param engineName Name of engine
		 * \return Returns results of linked TFs
		 */
		tf_results_t executeActiveLinkedTFs(const std::string &engineName);

		/*!
		 * \brief Is TF active
		 * \param tfName Name of TF
		 * \return Returns true if active, false otherwise.
		 * If TF settings with given name are not stored, returns false
		 */
		bool isActive(const std::string &tfName);

		/*!
		 * \brief Get TF Interpreter
		 */
		TransceiverFunctionInterpreter &getInterpreter();

	private:
		/*!
		 * \brief Set containing TF configurations
		 */
		tf_settings_t _tfSettings;

		/*!
		 * \brief Python Interpreter for TFs
		 */
		TransceiverFunctionInterpreter _tfInterpreter;
};

using TransceiverFunctionManagerSharedPtr = std::shared_ptr<TransceiverFunctionManager>;
using TransceiverFunctionManagerConstSharedPtr = std::shared_ptr<const TransceiverFunctionManager>;


#endif // TRANSCeivER_FUNCTION_MANAGER_H
