# Service Resource

A service resource represents a service inside MaxScale. A service is a
collection of network listeners, filters, a router and a set of backend servers.

## Resource Operations

### Get a service

Get a single service. The _:name_ in the URI must be a valid service name in
lowercase with all whitespace replaced with underscores.

```
GET /service/:name
```

Get the owning service of a session. _:id_ must be a valid session ID.

```
GET /session/:id/service
```

#### Response

```
Status: 200 OK

[
    {
        "name": "My Service",
        "router": "readwritesplit",
        "router_options": "disable_sescmd_history=true",
        "state": "started",
        "total_connections": 10,
        "current_connections": 2,
        "started": "2016-08-29T12:52:31+03:00",
        "servers": [
            "db-serv-1",
            "db-serv-2",
            "db-serv-3"
        ]
    }
]
```

### Get all services

Get all services.

```
GET /service
```

#### Response

```
Status: 200 OK

[
    {
        "name": "My Service",
        "router": "readwritesplit",
        "router_options": "disable_sescmd_history=true",
        "state": "started",
        "total_connections": 10,
        "current_connections": 2,
        "started": "2016-08-29T12:52:31+03:00",
        "servers": [
            "db-serv-1",
            "db-serv-2",
            "db-serv-3"
        ]
    },
    {
        "name": "My Second Service",
        "router": "readconnroute",
        "router_options": "master",
        "state": "started",
        "total_connections": 10,
        "current_connections": 2,
        "started": "2016-08-29T12:52:31+03:00",
        "servers": [
            "db-serv-1",
            "db-serv-2"
        ]
    }
]
```

### Update a service

Partially update a service. The _:name_ in the URI must map to a service name.

```
PATCH /service/:name
```

### Input

At least one of the following fields must be provided in the request body. Only
the provided fields in the service are modified.

|Field         |Type        |Description                                        |
|--------------|------------|---------------------------------------------------|
|servers       |string array|Servers used by this service                       |
|state         |string      |State of the service, either `started` or `stopped`|
|router_options|string      |Router specific options                            |

```
{
    "servers": [
        "db-serv-2",
        "db-serv-3"
    ],
    "state": "started",
    "router_options": "disable_sescmd_history=false"
}
```

#### Response

```
Status: 200 OK

[
    {
        "name": "My Service",
        "router": "readwritesplit",
        "router_options": "disable_sescmd_history=false"
        "state": "started",
        "total_connections": 10,
        "current_connections": 2,
        "started": "2016-08-29T12:52:31+03:00",
        "servers": [
            "db-serv-2",
            "db-serv-3"
        ]
    }
]
```
