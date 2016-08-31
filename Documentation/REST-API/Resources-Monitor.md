# Monitor Resource

A monitor resource represents a monitor inside MaxScale that monitors one or
more servers.

## Resource Operations

### Get a monitor

Get a single monitor. The _:name_ in the URI must be a valid monitor name in
lowercase with all whitespace replaced with underscores.

```
GET /monitors/:name
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
            "db-serv-1",
            "db-serv-2",
            "db-serv-3"
        ]
    }
]
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
            "db-serv-1",
            "db-serv-2",
            "db-serv-3"
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
            "db-galera-1",
            "db-galera-2",
            "db-galera-3"
        ]
    }
]
```

### Update a monitor

Partially update a monitor. The _:name_ in the URI must be a valid monitor name
in lowercase with all whitespace replaced with underscores.

```
PATCH /monitor/:name
```

### Input

At least one of the following fields must be provided in the request body. Only
the provided fields in the monitor are modified.

|Field            |Type        |Description                                        |
|-----------------|------------|---------------------------------------------------|
|servers          |string array|Servers monitored by this monitor                  |
|state            |string      |State of the monitor, either `started` or `stopped`|
|monitor_interval |number      |Monitoring interval in milliseconds                |
|connect_timeout  |number      |Connection timeout in seconds                      |
|read_timeout     |number      |Read timeout in seconds                            |
|write_timeout    |number      |Write timeout in seconds                           |

```
{
    "servers": [
        "db-serv-2",
        "db-serv-3"
    ],
    "state": "started",
    "monitor_interval": 2000,
    "connect_timeout": 2,
    "read_timeout": 2,
    "write_timeout": 2
}
```

#### Response

```
Status: 200 OK

[
    {
        "name": "MySQL Monitor",
        "module": "mysqlmon",
        "servers": [
            "db-serv-2",
            "db-serv-3"
        ],
        "state": "started",
        "monitor_interval": 2000,
        "connect_timeout": 2,
        "read_timeout": 2,
        "write_timeout": 2
    }
]
```
