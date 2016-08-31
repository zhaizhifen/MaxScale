# Server Resource

A server resource represents a backend database server.

## Resource Operations

### Get a server

Get a single server. The _:name_ in the URI must be a valid server name in
lowercase with all whitespace replaced with underscores.

```
GET /servers/:name
```

#### Response

```
Status: 200 OK

[
    {
        "name": "db-serv-1",
        "address": "192.168.121.58",
        "port": 3306,
        "protocol": "MySQLBackend",
        "status": [
            "master",
            "running"
        ],
        "parameters": {
            "report_weight": 10,
            "app_weight": 2
        }
    }
]
```

**Note**: The _parameters_ field contains all non-standard parameters for
  servers, including the server weighting parameters.

### Get all servers

```
GET /servers
```

#### Response

```
Status: 200 OK

[
    {
        "name": "db-serv-1",
        "address": "192.168.121.58",
        "port": 3306,
        "protocol": "MySQLBackend",
        "status": [
            "master",
            "running"
        ],
        "parameters": {
            "report_weight": 10,
            "app_weight": 2
        }
    },
    {
        "name": "db-serv-2",
        "address": "192.168.121.175",
        "port": 3306,
        "status": [
            "slave",
            "running"
        ],
        "protocol": "MySQLBackend",
        "parameters": {
            "app_weight": 6
        }
    }
]
```

### Update a service

Partially update a server. The _:name_ in the URI must be a valid server name in
lowercase with all whitespace replaced with underscores.

```
PATCH /servers/:name
```

### Input

At least one of the following fields must be provided in the request body. Only
the provided fields in the server are modified and any existing values are
replaced with the ones provided in the response body.


|Field      |Type        |Description                                                                      |
|-----------|------------|---------------------------------------------------------------------------------|
|address    |string      |Server address                                                                   |
|port       |number      |Server port                                                                      |
|parameters |object      |Server extra parameters                                                          |
|state      |string array|New server state, list of `master`, `slave`, `synced`, `running` or `maintenance`|

```
{
    "address": "192.168.0.100",
    "port": 4006,
    "state": [
        "running",
        "maintenance"
    ],
    "parameters": {
        "report_weight": 1
    }
}
```

#### Response

```
Status: 200 OK

[
    {
        "name": "db-serv-1",
        "protocol": "MySQLBackend",
        "address": "192.168.0.100",
        "port": 4006,
        "state": [
            "running",
            "maintenance"
        ],
        "parameters": {
            "report_weight": 1
        }
    }
]
```

### Close all connections to a server

Close all connections to a particular server. This will forcefully close all
backend connections.

```
DELETE /servers/:name/connections
```

#### Response

```
Status: 204 No Content
```
