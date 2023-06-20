#!/usr/bin/env python3

# NRP Core - Backend infrastructure to synchronize simulations
#
# Copyright 2020-2023 NRP Team
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# This project has received funding from the European Union’s Horizon 2020
# Framework Programme for Research and Innovation under the Specific Grant
# Agreement No. 945539 (Human Brain Project SGA3).

from nrp_core.event_loop import run_event_loop_engine_app
from nrp_core.engines.python_json import parseAndValidateEngineConfig, JSONEngineWrapper, JSONEventLoopEngine


if __name__ == '__main__':
    run_event_loop_engine_app(parseAndValidateEngineConfig, JSONEngineWrapper, JSONEventLoopEngine)


