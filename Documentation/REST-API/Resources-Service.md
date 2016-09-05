# Service Resource

A service resource represents a service inside MaxScale. A service is a
collection of network listeners, filters, a router and a set of backend servers.

## Resource Operations

### Get a service

Get a single service. The _:name_ in the URI must be a valid service name with
all whitespace replaced with hyphens. The service names are case-insensitive.

```
GET /services/:name
```

#### Response

```
Status: 200 OK

{
    "name": "My Service",
    "router": "readwritesplit",
    "router_options": {
        "disable_sescmd_history": "true"
    },
    "state": "started",
    "total_connections": 10,
    "current_connections": 2,
    "started": "2016-08-29T12:52:31+03:00",
    "filters": [
        "/filters/Query-Logging-Filter"
    ],
    "servers": [
        "/servers/db-serv-1",
        "/servers/db-serv-2",
        "/servers/db-serv-3"
    ]
}
```

### Get all services

Get all services.

```
GET /services
```

#### Response

```
Status: 200 OK

[
    {
        "name": "My Service",
        "router": "readwritesplit",
        "router_options": {
            "disable_sescmd_history": "true"
        },
        "state": "started",
        "total_connections": 10,
        "current_connections": 2,
        "started": "2016-08-29T12:52:31+03:00",
        "filters": [
            "/filters/Query-Logging-Filter"
        ],
        "servers": [
            "/servers/db-serv-1",
            "/servers/db-serv-2",
            "/servers/db-serv-3"
        ]
    },
    {
        "name": "My Second Service",
        "router": "readconnroute",
        "router_options": {
            "type": "master"
        },
        "state": "started",
        "total_connections": 10,
        "current_connections": 2,
        "started": "2016-08-29T12:52:31+03:00",
        "servers": [
            "/servers/db-serv-1",
            "/servers/db-serv-2"
        ]
    }
]
```

### Update a service

**Note**: The update mechanisms described here are provisional and most likely
  will change in the future. This description is only for design purposes and
  does not yet work.

Partially update a service. The _:name_ in the URI must map to a service name
and the request body must be a valid JSON Patch document which is applied to the
resource.

```
PATCH /services/:name
```

### Modifiable Fields

|Field         |Type        |Description                                        |
|--------------|------------|---------------------------------------------------|
|servers       |string array|Servers used by this service, must be relative links to existing server resources|
|state         |string      |State of the service, either `started` or `stopped`|
|router_options|object      |Router specific options                            |
|filters       |string array|Service filters, configured in the same order they are declared in the array (`filters[0]` => first filter, `filters[1]` => second filter)|

```
[
    { "op": "replace", "path": "/servers", "value": ["/servers/db-serv-2","/servers/db-serv-3"] },
    { "op": "replace", "path": "/state", "value": "started" },
    { "op": "add", "path": "/router_options/master_failover_mode", "value": "fail_on_write" },
    { "op": "remove", "path": "/filters" }
]
```

_TODO: Add common service parameters_

#### Response

Response contains the modified resource.

```
Status: 200 OK

    {
        "name": "My Service",
        "router": "readwritesplit",
        "router_options": {
            "disable_sescmd_history=false",
            "master_failover_mode": "fail_on_write"
        }
        "state": "started",
        "total_connections": 10,
        "current_connections": 2,
        "started": "2016-08-29T12:52:31+03:00",
        "servers": [
            "/servers/db-serv-2",
            "/servers/db-serv-3"
        ]
    }
```

### Get all sessions for a service

Get all sessions for a particular service.

```
GET /services/:name/sessions
```

#### Response

Relative links to all sessions for this service.

```
Status: 200 OK

[
    "/sessions/1",
    "/sessions/2"
]
```

### Close all sessions for a service

Close all sessions for a particular service. This will forcefully close all
client connections and any backend connections they have made.

```
DELETE /services/:name/sessions
```

#### Response

```
Status: 204 No Content
```
