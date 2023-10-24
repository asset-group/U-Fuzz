# ☁️ Web API

The fuzzer starts a SocketIO server and exposes access to its functions to a SocketIO client through websockets. The Table below lists the current API available for the client. **More to be added in future updates**.

Two types of server is available. You can choose the server type on the configuration file by changing the `ServerModule` index number:

```json
"ServerOptions": {
    "APINamespace": "/", 			// Namespace (root path) of event. On REST, this path serves a basic API documentation
    "Enable": true,					// Enable Server
    "EnableEvents": false,  		// Enable SocketIO Asynchronous events (from server to clients)
    "ListenAddress": "127.0.0.1", 	// Server address
    "Logging": false,				// Enable Server logging
    "Port": 3000,					// Server port
    "ServerModule": 1,				// Server module index
    "ServerModulesList": [			// List of Server modules (from 0 to N)
        "SocketIOServer",			// modules/server/SocketIOServer.py (index 0)
        "RESTServer"				// modules/server/RESTServer.py     (index 1)
    ]
}
```

::: warning

SocketIO and REST Server events name are case sensitive. Requests from clients with wrong event names are ignored.

:::

## Request Events (Polling - [SocketIOServer](https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/blob/wdissector/modules/server/SocketIOServer.py) / [RESTServer](https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/blob/wdissector/modules/server/RESTServer.py))

| Event Name                          | Description                                                  | Input                     | Return                     | Example (SocketIO)                                           |
| ----------------------------------- | ------------------------------------------------------------ | ------------------------- | -------------------------- | ------------------------------------------------------------ |
| <small>**Summary**</small>          | <small>Returns summary of current fuzzing session, including statistics such as number of packets tarnsmitted and received</small> | <small>Nothing</small>    | <small>**String**</small>  | <small>scripts/server_test.py **-r** Summary</small>         |
| <small>**GraphDot**</small>         | <small>Returns current state machine in dot string format</small> | <small>Nothing</small>    | <small>**String**</small>  | <small>scripts/server_test.py **-r** GraphDot</small>        |
| <small>**GetModelConfig**</small>   | <small>Get fuzzer configuration</small>                      | <small>Nothing</small>    | <small>**JSON**</small>    | <small>scripts/server_test.py **-r** GetModelConfig</small>  |
| **<small>SetModelConfig</small>**   | <small>Set fuzzer configuration, returns parsing status</small> | <small>**JSON**</small>   | <small>**Boolean**</small> | <small>scripts/server_test.py **-r** SetModelConfig -d '{"config":{...}}'</small> |
| <small>**GetDefaultConfig**</small> | <small>Get fuzzer default configuration</small>              | <small>Nothing</small>    | <small>**JSON**</small>    | <small>scripts/server_test.py **-r** GetDefaultConfig</small> |
| <small>**ResetConfig**</small>      | <small>Reset fuzzer configuration to default. Requires restart</small> | <small>Nothing</small>    | <small>**JSON**</small>    | <small>scripts/server_test.py **-r** ResetConfig</small>     |
| <small>**Shutdown**</small>         | <small>Shutdown fuzzer (terminate process)</small>           | <small>Nothing</small>    | <small>Nothing</small>     | <small>scripts/server_test.py **-r** Shutdown</small>        |
| <small>**Start**</small>            | <small>Starts fuzzing the target defined in configuration file</small> | <small>Nothing</small>    | <small>Nothing</small>     | <small>scripts/server_test.py **-r** Start</small>           |
| <small>**Stop**</small>             | <small>Stops fuzzing</small>                                 | <small>Nothing</small>    | <small>Nothing</small>     | <small>scripts/server_test.py **-r** Stop</small>            |
| <small>**Scan**</small>             | <small>Start BT scan (Enquiry) for a fixed time of 15 seconds. Subscribe to async event `Scan` to get targets results as soon as they are found during scanning</small> | <small>Nothing</small>    | <small>Nothing</small>     | <small>scripts/server_test.py **-r** Scan</small>            |
| <small>**GetScanResults**</small>   | <small>Get scan results of all targets found during latest scanning request. Request event `Scan` needs to be called first</small> | <small>Nothing</small>    | <small>**JSON**</small>    | <small>scripts/server_test.py **-r** GetScanResults</small>  |
| <small>**StartExploit**</small>     | <small>Start exploit by name</small>                         | <small>**String**</small> | <small>**Boolean**</small> | <small>scripts/server_test.py **-r** StartExploit **-d** knob</small> |
| <small>**StopExploit**</small>      | <small>Stop exploit if it was previously running</small>     | <small>Nothing</small>    | <small>Nothing</small>     | <small>scripts/server_test.py **-r** StartExploit **-d** knob</small> |

::: warning

REST Server implements `GET` and `POST` methods for all events (endpoints). 

1. For `POST` requests, the argument must be sent in JSON type.
2. For `GET` the request data must be included in the URL the parameter `?args=`

:::

## Subscription Events (Asynchronous - [SocketIOServer](https://gitlab.com/asset-sutd/software/wireless-deep-fuzzer/-/blob/wdissector/modules/server/SocketIOServer.py))

| Event Name                     | Description                                                  | Return                    | Example (SocketIO)                                       |
| ------------------------------ | ------------------------------------------------------------ | ------------------------- | -------------------------------------------------------- |
| <small>**Anomaly**</small>     | <small>Returns information of anomaly (crash/deadlock/anomaly)</small> | <small>**JSON**</small>   | <small>scripts/server_test.py **-w** Anomaly</small>     |
| <small>**GraphUpdate**</small> | <small>Returns string of current state in dot format</small> | <small>**String**</small> | <small>scripts/server_test.py **-w** GraphUpdate</small> |
| <small>**Modules**</small>     | <small>Returns modules (exploits)  status messages. The returned json object contains `"level"` to indicate priority (Green `"G"`, Yellow `"Y"`,Red `"R"`) and `"msg"`  with the status text.</small> | <small>**JSON**</small>   | <small>scripts/server_test.py **-w** Modules</small>     |
| <small>**Scan**</small>        | <small>Returns scanned target information while scanning is in progress. The returned json object contains target `BDAddress`, `Name`, `RSSI` and `Class`.</small> | <small>**JSON**</small>   | <small>scripts/server_test.py **-w** Scan</small>        |

