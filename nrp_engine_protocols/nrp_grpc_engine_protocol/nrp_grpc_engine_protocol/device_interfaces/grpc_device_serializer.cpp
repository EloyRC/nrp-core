#include "nrp_grpc_engine_protocol/device_interfaces/grpc_device_serializer.h"


template<>
GRPCDevice DeviceSerializerMethods<GRPCDevice>::serializeID<GRPCDevice>(const DeviceIdentifier &devID)
{
	EngineGrpc::DeviceMessage msg;
	msg.mutable_deviceid()->set_devicename(devID.Name);
	msg.mutable_deviceid()->set_enginename(devID.EngineName);
	msg.mutable_deviceid()->set_devicetype(devID.Type);
	return msg;
}

template<>
EngineGrpc::DeviceMessage DeviceSerializerMethods<GRPCDevice>::serializeID<EngineGrpc::DeviceMessage>(const DeviceIdentifier &devID)
{
	EngineGrpc::DeviceMessage msg;
	msg.mutable_deviceid()->set_devicename(devID.Name);
	msg.mutable_deviceid()->set_enginename(devID.EngineName);
	msg.mutable_deviceid()->set_devicetype(devID.Type);
	return msg;
}

DeviceIdentifier DeviceSerializerMethods<GRPCDevice>::deserializeID(DeviceSerializerMethods<GRPCDevice>::deserialization_t data)
{
	const auto &devID = data->deviceid();
	return DeviceIdentifier(devID.devicename(), devID.enginename(), devID.devicetype());
}
