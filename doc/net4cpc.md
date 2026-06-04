# Net4CPC Ethernet emulation

Caprice32 supports emulation of the [Net4CPC](https://github.com/salafek/Net4CPC) expansion board, which connects a WIZnet W5100S Ethernet controller to the CPC expansion bus.

## Enabling it

Open the emulator menu (F1) → **Options** → **General** tab → tick **Enable Net4CPC ethernet emulation**. The setting is saved in `cap32.cfg` as `net4cpc=1`.

## How it works

The W5100S is accessed through four Z80 I/O ports using the chip's indirect parallel-bus protocol:

| Port   | Name    | Role |
|--------|---------|------|
| 0xFD20 | MR      | Mode Register — returns 0x03 (chip present, auto-increment on) |
| 0xFD21 | IDM_ARH | High byte of 16-bit indirect address |
| 0xFD22 | IDM_ARL | Low byte of 16-bit indirect address |
| 0xFD23 | IDM_DR  | Data register — address auto-increments after each access |

The full 64 KB W5100S register space is emulated in software:

| Range          | Contents |
|----------------|----------|
| 0x0000–0x002F  | Common registers (MR, GAR, SUBR, SHAR, SIPR, …) |
| 0x0400–0x07FF  | Socket 0–3 registers (256 bytes each) |
| 0x4000–0x5FFF  | TX ring buffers (2 KB per socket) |
| 0x6000–0x7FFF  | RX ring buffers (2 KB per socket) |

Socket commands (OPEN, CONNECT, SEND, RECV, CLOSE, …) are intercepted and mapped to host POSIX sockets — TCP via `SOCK_STREAM`, UDP via `SOCK_DGRAM`. All sockets are non-blocking; connection completion and incoming data are detected lazily when the CPC firmware reads the socket status or RX size registers.

For UDP receive, the emulator prepends the standard W5100S 8-byte header to each datagram in the RX ring buffer (4 bytes source IP + 2 bytes source port + 2 bytes payload length), matching the behaviour expected by W5100S firmware such as the Net4CPC DNS library.

## Source IP binding (SIPR)

When the CPC firmware writes its desired IP address into the W5100S SIPR registers (0x000F–0x0012) and issues an OPEN command, the emulator calls `bind()` on the host socket to that address. This allows the emulated CPC to appear on the local network under its own IP, provided that IP is assigned to a host interface. If the bind fails the socket falls back to the host's default IP silently.

## Limitations

- No hardware interrupts (IR pin) — firmware must poll Sn_SR / Sn_RX_RSR.
- TCP listen / server-side accept is not implemented.
- ICMP (ping) is not emulated.
- Windows support is untested. The socket layer uses `#ifdef _WIN32` to select Winsock2 vs POSIX APIs, but the Windows build path has not been verified.
