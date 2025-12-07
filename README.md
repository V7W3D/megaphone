# Megaphone

A client-server messaging system implementing a microblogging service with thread-based discussions and real-time notifications over IPv6 UDP multicast.

## Overview

Megaphone is a distributed messaging platform where users can:
- Register with a unique pseudo
- Post messages (billets) to discussion threads (fils)
- Subscribe to threads and receive real-time notifications via IPv6 multicast
- Share files within threads

The system uses a custom binary protocol over UDP with bitfield-packed headers for efficient communication.

## Architecture

- **Server** (`server.c`): Manages user registration, thread creation, message posting, and notification distribution
- **Client** (`client.c`): Handles user interactions, message composition, and receives multicast notifications
- **Protocol**: Custom binary message formats defined in `msgcli.h` and `msgsrv.h`
- **Data Structures**:
  - Users: Linked list of registered users (`user.h`)
  - Threads (fils): Linked list containing billets, files, and subscribers (`fil.h`)
  - Messages: Packed binary structures with 5-bit operation codes and 11-bit user IDs

## Protocol

Messages use 16-bit headers with:
- 5 bits for operation code (1=register, 2=post, 4=subscribe, 31=error, etc.)
- 11 bits for user ID (max 2047 users)

## Setup

### Prerequisites
- GCC compiler
- POSIX-compliant system (Linux/Unix)
- IPv6 network support

### Build
```bash
# Compile server
gcc -o server server.c msgsrv.c msgcli.c user.c fil.c -lpthread

# Compile client
gcc -o client client.c msgsrv.c msgcli.c user.c fil.c -lpthread
```

### Run
```bash
# Terminal 1: Start server (port 7777)
./server

# Terminal 2+: Start client(s)
./client
```

## Contributing

### Team
See `authors.md` for project contributors.

### Guidelines
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/your-feature`)
3. Follow existing code style (K&R C conventions)
4. Test with multiple clients and thread subscriptions
5. Commit changes (`git commit -m 'Add feature'`)
6. Push to branch (`git push origin feature/your-feature`)
7. Open a Pull Request

### Code Structure
- Keep protocol definitions in header files (`msgcli.h`, `msgsrv.h`)
- Use linked lists for dynamic data (users, threads, messages)
- Handle thread safety for multicast notifications
- Validate all network input to prevent buffer overflows

## License

Project developed as part of network programming coursework.
