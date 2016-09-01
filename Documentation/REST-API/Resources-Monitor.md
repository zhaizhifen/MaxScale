# Monitor Resource

A monitor resource represents a monitor inside MaxScale that monitors one or
more servers.

## Resource Operations

### Get a monitor

Get a single monitor. The _:name_ in the URI must be a valid monitor name with
all whitespace replaced with hyphens. The monitor names are case-insensitive.

```
GET /monitors/:name
```

#### Response

```
Status: 200 OK

{
    "name": "MySQL Monitor",
    "module": "mysqlmon",
    "state": "started",
    "monitor_interval": 2500,
    "connect_timeout": 5,
    "read_timeout": 2,
    "write_timeout": 3,
    "servers": [
        "/servers/db-serv-1",
        "/servers/db-serv-2",
        "/servers/db-serv-3"
    ]
}
```

### Get all monitors

Get all monitors.

```
GET /monitors
```

#### Response

```
Status: 200 OK

[
    {
        "name": "MySQL Monitor",
        "module": "mysqlmon",
        "state": "started",
        "monitor_interval": 2500,
        "connect_timeout": 5,
        "read_timeout": 2,
        "write_timeout": 3,
        "servers": [
            "/servers/db-serv-1",
            "/servers/db-serv-2",
            "/servers/db-serv-3"
        ]
    },
    {
        "name": "Galera Monitor",
        "module": "galeramon",
        "state": "started",
        "monitor_interval": 5000,
        "connect_timeout": 10,
        "read_timeout": 5,
        "write_timeout": 5,
        "servers": [
            "/servers/db-galera-1",
            "/servers/db-galera-2",
            "/servers/db-galera-3"
        ]
    }
]
```

### Update a monitor

Partially update a monitor. The _:name_ in the URI must map to a monitor name
and the request body must be a valid JSON Patch document which is applied to the
resource.

```
PATCH /monitor/:name
```

### Modifiable Fields

The following values can be modified with the PATCH method.

|Field            |Type        |Description                                        |
|-----------------|------------|---------------------------------------------------|
|servers          |string array|Servers monitored by this monitor                  |
|state            |string      |State of the monitor, either `started` or `stopped`|
|monitor_interval |number      |Monitoring interval in milliseconds                |
|connect_timeout  |number      |Connection timeout in seconds                      |
|read_timeout     |number      |Read timeout in seconds                            |
|write_timeout    |number      |Write timeout in seconds                           |

```
[
    { "op": "remove", "path": "/servers/0" },
    { "op": "replace", "path": "/state", "value": "started" },
    { "op": "replace", "path": "/monitor_interval", "value": 2000 },
    { "op": "replace", "path": "/connect_timeout", "value": 2 },
    { "op": "replace", "path": "/read_timeout", "value": 2 },
    { "op": "replace", "path": "/write_timeout", "value": 2 }
]
```

#### Response

Response contains the modified resource.

```
Status: 200 OK

{
    "name": "MySQL Monitor",
    "module": "mysqlmon",
    "servers": [
        "/servers/db-serv-2",
        "/servers/db-serv-3"
    ],
    "state": "started",
    "monitor_interval": 2000,
    "connect_timeout": 2,
    "read_timeout": 2,
    "write_timeout": 2
}
```
