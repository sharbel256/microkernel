
Phase 1
```
microkernel/
├── src/
│   ├── kernel/
│   │   ├── kernel.cpp
│   │   ├── scheduler.cpp
│   │   └── ipc.cpp
│   ├── services/
│   │   ├── service_a.cpp
│   │   └── service_b.cpp
│   └── main.cpp
├── include/
│   └── kernel/
│       ├── kernel.hpp
│       ├── scheduler.hpp
│       ├── ipc.hpp
│       └── types.hpp
└── CMakeLists.txt
```