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

#ifndef GAZEBO_ENGINE_JSON_NRP_CLIENT_H
#define GAZEBO_ENGINE_JSON_NRP_CLIENT_H

#include "nrp_json_engine_protocol/nrp_client/engine_json_nrp_client.h"
#include "nrp_general_library/engine_interfaces/engine_client_interface.h"
#include "nrp_general_library/plugin_system/plugin.h"

#include "nrp_gazebo_devices/physics_camera.h"
#include "nrp_gazebo_devices/physics_joint.h"
#include "nrp_gazebo_devices/physics_link.h"

#include "nrp_gazebo_json_engine/config/gazebo_json_config.h"

#include <unistd.h>


/*!
 *  \brief NRP - Gazebo Communicator on the NRP side. Converts DeviceInterface classes from/to JSON objects
 */
class GazeboEngineJSONNRPClient
: public EngineJSONNRPClient<GazeboEngineJSONNRPClient, GazeboJSONConfigConst::EngineSchema, PhysicsCamera, PhysicsJoint, PhysicsLink>
{
	public:
		GazeboEngineJSONNRPClient(nlohmann::json &config, ProcessLauncherInterface::unique_ptr &&launcher);
		virtual ~GazeboEngineJSONNRPClient() override = default;

		virtual void initialize() override;

		virtual void shutdown() override;

        virtual const std::vector<std::string> engineProcStartParams() const override;

        virtual const std::vector<std::string> engineProcEnvParams() const override;
};

using GazeboEngineJSONLauncher = GazeboEngineJSONNRPClient::EngineLauncher<GazeboJSONConfigConst::EngineType>;

CREATE_NRP_ENGINE_LAUNCHER(GazeboEngineJSONLauncher);

/*! \defgroup gazebo_json_engine Gazebo REST JSON Engine

The following sections illustrate how to interface with Gazebo using a JSON interface

\section Usage

\subsection Devices
Gazebo devices are defined as follows:
\copydoc gazebo_devices

\subsection World File Setup
Gazebo world files must be modified to properly link them with the NRP. Additional plugins must be inserted for the NRP to recognize interfaces and link them to the correct devices:

\subsubsection NRPGazeboJSONCommunicationPlugin

General communication plugin. Sets up a JSON server and waits for NRP commands
\code{.xml}
<world>
...
	<plugin name="nrp_world_plugin" filename="NRPGazeboJSONWorldPlugin.so"/>
...
</world>
\endcode

\subsubsection NRPGazeboJSONCameraPlugin

Adds a PhysicsCamera device. The example below registers a device called 'camera_device'. This can then be accessed by TransceiverFunctions under said name
\code{.xml}
<sensor name='camera' type='camera'>
	...
	<plugin name='camera_device' filename='NRPGazeboJSONCameraControllerPlugin.so'/>
	...
</sensor>
\endcode

\subsubsection NRPGazeboJSONLinkPlugin
Adds PhysicsLink devices for each link in the given model. The example below registers four devices under the name of their respective links names
\code{.xml}
<model>
	...
	<plugin name="link_plugin" filename="NRPGazeboJSONLinkControllerPlugin.so" />
	...
	<link name="back_left_link">...</link>
	<link name="back_right_link">...</link>
	<link name="front_left_link">...</link>
	<link name="front_right_link">...</link>
	...
</model>
\endcode

\subsubsection NRPGazeboJSONJointPlugin
Adds PhysicsJoint devices. The example below registers four devices under the name of their respective
element names and set up target velocities and PID controllers. Currently, gazebo supports two target types, 'position' or 'velocity'. Note that
only those joints that are explicitly named in the plugin will be registered and made available to the NRP. In addition, this is currently the onlz gazebo device
that can be used to send data to the simulation.
\code{.xml}
<model>
	...
	<joint name="back_left_joint">...</joint>
	<joint name="back_right_joint">...</joint>
	<joint name="front_left_joint">...</joint>
	<joint name="front_right_joint">...</joint>
	...
	<plugin name='husky' filename='NRPGazeboGrpcJointControllerPlugin.so'>
		<back_left_joint   P='10' I='0' D='0' Type='velocity' Target='0' IMax='0' IMin='0'/>
		<back_right_joint  P='10' I='0' D='0' Type='velocity' Target='0' IMax='0' IMin='0'/>
		<front_left_joint  P='10' I='0' D='0' Type='velocity' Target='0' IMax='0' IMin='0'/>
		<front_right_joint P='10' I='0' D='0' Type='velocity' Target='0' IMax='0' IMin='0'/>
	</plugin>
	...
</model>
\endcode


\subsection Engine Configuration

\bold To use this engine type, set the EngineType to "gazebo_json"

The Gazebo JSON Engine uses the configuration options specified in \ref engine_config_section and in \ref engine_json_config_section, as well as the options outlined below:

<table>
<caption id="gazebo_json_engine_config_table">Gazebo Engine ConfigurationOptions</caption>
<tr><th>Name                     <th>Description                                                                <th>Type                <th>Default
<tr><td>GazeboPlugins            <td>Array of additional systems plugins that should be loaded on startup       <td>array of strings    <td>[]
<tr><td>GazeboRNGSeed            <td>RNG Seed for Gazebo. If not set, a random value will be used on startup    <td>unsigned int        <td>random value
<tr><td>GazeboWorldFile          <td>SDF World file to pass to Gazebo on startup                                <td>string              <td>no default value
</table>


\section Developer Documentation

The Gazebo JSON Engine is based on the \ref json_engine.
The client is defined in GazeboEngineJSONNRPClient.
 */

#endif // GAZEBO_ENGINE_JSON_NRP_CLIENT_H
