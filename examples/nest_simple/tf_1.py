from NRPPythonModule import *
from NRPJSONEngineProtocolPython import *

@FromEngineDevice(keyword='voltage', id=DeviceIdentifier('voltage', 'nest'))
@TransceiverFunction("nest")
def transceiver_function(voltage):
  print(voltage.data[0]["events"])
  noise_device = JsonDevice("noise", "nest")
  noise_device.data["rate"] = 15000.0

  return [ noise_device ]