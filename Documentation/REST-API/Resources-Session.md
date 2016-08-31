# Session Resource

A session consists of a client connection, any number of related backend
connections, a router module session and possibly filter module sessions. Each
session is created on a service and a service can have multiple sessions.

## Resource Operations

### Get a session

Get a single session. _:id_ must be a valid session ID.

```
GET /sessions/:id
```

#### Response

```
Status: 200 OK

[
    {
        "state": "Session ready for routing",
        "user": "jdoe",
        "address": "192.168.0.200",
        "service": "My Service",
        "connected": "Wed Aug 31 03:03:12 2016",
        "idle": 260
    }
]
```

### Get all sessions

Get all sessions.

```
GET /sessions
```

#### Response

```
Status: 200 OK

[
    {
        "state": "Session ready for routing",
        "user": "jdoe",
        "address": "192.168.0.200",
        "service": "My Service",
        "connected": "Wed Aug 31 03:03:12 2016",
        "idle": 260
    },
    {
        "state": "Session ready for routing",
        "user": "dba",
        "address": "192.168.0.201",
        "service": "My Service",
        "connected": "Wed Aug 31 03:10:00 2016",
        "idle": 1
    }
]
```

### Get all connections created by a session

Get all backend connections created by a session. _:id_ must be a valid session ID.

```
GET /sessions/:id/connections
```

#### Response

```
Status: 200 OK

[
    {
        "state": "DCB in the polling loop",
        "role": "Backend Request Handler",
        "server": "db-serv-01"
        "statistics": {
            "reads":             2197
            "writes":            1562
            "buffered_writes":   0
            "high_water_events": 0
            "low_water_events":  0
        }
    },
    {
        "state": "DCB in the polling loop",
        "role": "Backend Request Handler",
        "server": "db-serv-02"
        "statistics": {
            "reads":             0
            "writes":            0
            "buffered_writes":   0
            "high_water_events": 0
            "low_water_events":  0
        }
    }
]
```

### Close a session

Close a session. This will forcefully close the client connection and any
backend connections.

```
DELETE /sessions/:id
```

#### Response

```
Status: 204 No Content
```
