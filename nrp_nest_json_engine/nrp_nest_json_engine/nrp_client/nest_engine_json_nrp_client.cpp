#include "nrp_nest_json_engine/nrp_client/nest_engine_json_nrp_client.h"

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#include <chrono>

NestEngineJSONNRPClient::NestEngineJSONNRPClient(EngineConfigConst::config_storage_t &config, ProcessLauncherInterface::unique_ptr &&launcher)
    : EngineJSONNRPClient(config, std::move(launcher))
{}

NestEngineJSONNRPClient::~NestEngineJSONNRPClient()
{}

void NestEngineJSONNRPClient::initialize()
{
	const auto &nestConfig = this->engineConfig();
	nlohmann::json resp = this->sendInitCommand(nlohmann::json({{NestConfig::ConfigType.m_data, nestConfig->writeConfig()}}));
	if(!resp.at(NestConfig::InitFileExecStatus.data()).get<bool>())
	{
		// Write the error message
		this->_initErrMsg = resp.at(NestConfig::InitFileErrorMsg.data());
		NRPLogger::SPDErrLogDefault(this->_initErrMsg);

		throw NRPException::logCreate("Engine \"" + this->engineName() + "\" initialization failed: " + this->_initErrMsg);
	}
}

void NestEngineJSONNRPClient::shutdown()
{
	this->sendShutdownCommand(nlohmann::json());
}
