/* net4cpc.cpp - W5100S / Net4CPC hardware emulation for Caprice32
 *
 * Emulates the W5100S Ethernet controller as accessed through the Net4CPC
 * expansion board at I/O ports 0xFD20–0xFD23 (indirect parallel bus mode).
 *
 * The W5100S supports up to 4 independent, simultaneous hardware sockets
 * (Sockets 0–3) capable of TCP and UDP.
 *
 * Memory layout (same as real W5100S):
 *   0x0000–0x002F  Common registers (MR, GAR, SUBR, SHAR, SIPR, RTR, …)
 *   0x0400–0x04FF  Socket 0 registers
 *   0x0500–0x05FF  Socket 1 registers
 *   0x0600–0x06FF  Socket 2 registers
 *   0x0700–0x07FF  Socket 3 registers
 *   0x4000–0x5FFF  TX ring buffers (2 KB × 4 sockets)
 *   0x6000–0x7FFF  RX ring buffers (2 KB × 4 sockets)
 *
 * UDP sockets: the W5100S prepends an 8-byte header to each received
 * datagram in the RX buffer: 4 bytes source IP, 2 bytes source port,
 * 2 bytes payload length.  This emulation replicates that behaviour.
 */

#include "net4cpc.h"

#include <cerrno>
#include <cstring>

// ---------------------------------------------------------------------------
// Platform socket abstraction
// ---------------------------------------------------------------------------

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
   using sock_t = SOCKET;
   static const sock_t INVALID_SOCK = INVALID_SOCKET;
   static inline void close_fd(sock_t fd)      { closesocket(fd); }
   static inline void set_nonblocking(sock_t fd) {
       u_long mode = 1;
       ioctlsocket(fd, FIONBIO, &mode);
   }
   static inline bool connect_pending() { return WSAGetLastError() == WSAEWOULDBLOCK; }
#else
#  include <arpa/inet.h>
#  include <fcntl.h>
#  include <netinet/in.h>
#  include <sys/select.h>
#  include <sys/socket.h>
#  include <unistd.h>
   using sock_t = int;
   static const sock_t INVALID_SOCK = -1;
   static inline void close_fd(sock_t fd)      { close(fd); }
   static inline void set_nonblocking(sock_t fd) {
       int flags = fcntl(fd, F_GETFL, 0);
       fcntl(fd, F_SETFL, flags | O_NONBLOCK);
   }
   static inline bool connect_pending() { return errno == EINPROGRESS; }
#endif

// ---------------------------------------------------------------------------
// W5100S register-space constants
// ---------------------------------------------------------------------------

static const uint16_t SOCK_BASE[4] = { 0x0400, 0x0500, 0x0600, 0x0700 };
static const uint16_t TX_BASE[4]   = { 0x4000, 0x4800, 0x5000, 0x5800 };
static const uint16_t RX_BASE[4]   = { 0x6000, 0x6800, 0x7000, 0x7800 };
static const uint16_t BUF_MASK     = 0x07FFu; // 2 KB per socket
static const uint16_t BUF_SIZE     = 2048u;

// Socket register offsets from SOCK_BASE[n]
static const uint16_t SR_MR    = 0x00;
static const uint16_t SR_CR    = 0x01;
static const uint16_t SR_SR    = 0x03;
static const uint16_t SR_DIPR  = 0x0C; // 4 bytes, big-endian
static const uint16_t SR_DPORT = 0x10; // 2 bytes, big-endian
static const uint16_t SR_TX_FSR = 0x20; // 2 bytes
static const uint16_t SR_TX_RD  = 0x22; // 2 bytes (chip's read pointer)
static const uint16_t SR_TX_WR  = 0x24; // 2 bytes (CPC's write pointer)
static const uint16_t SR_RX_RSR = 0x26; // 2 bytes
static const uint16_t SR_RX_RD  = 0x28; // 2 bytes (CPC's read pointer)

// Socket modes
static const uint8_t SMODE_TCP = 0x01;
static const uint8_t SMODE_UDP = 0x02;

// Socket status values
static const uint8_t SSTAT_CLOSED      = 0x00;
static const uint8_t SSTAT_INIT        = 0x13;
static const uint8_t SSTAT_SYNSENT     = 0x15;
static const uint8_t SSTAT_ESTABLISHED = 0x17;
static const uint8_t SSTAT_CLOSE_WAIT  = 0x1C;
static const uint8_t SSTAT_UDP         = 0x22;

// Socket commands
static const uint8_t SCMD_OPEN    = 0x01;
static const uint8_t SCMD_CONNECT = 0x04;
static const uint8_t SCMD_DISCON  = 0x08;
static const uint8_t SCMD_CLOSE   = 0x10;
static const uint8_t SCMD_SEND    = 0x20;
static const uint8_t SCMD_RECV    = 0x40;

// ---------------------------------------------------------------------------
// Emulator state
// ---------------------------------------------------------------------------

static uint8_t  regs[65536];       // W5100S register/buffer space
static uint16_t idm_ar = 0;        // current indirect address register
static sock_t   sock_fd[4];        // host socket file descriptors
static uint16_t rx_wr[4];          // internal RX write pointers (free-running)

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void set16(uint16_t addr, uint16_t val) {
    regs[addr]   = static_cast<uint8_t>(val >> 8);
    regs[addr+1] = static_cast<uint8_t>(val & 0xFF);
}

static uint16_t get16(uint16_t addr) {
    return (static_cast<uint16_t>(regs[addr]) << 8) | regs[addr+1];
}

static void update_rx_rsr(int s) {
    uint16_t rsr = static_cast<uint16_t>(rx_wr[s] - get16(SOCK_BASE[s] + SR_RX_RD));
    set16(SOCK_BASE[s] + SR_RX_RSR, rsr);
}

static void update_tx_fsr(int s) {
    uint16_t used = static_cast<uint16_t>(
        get16(SOCK_BASE[s] + SR_TX_WR) - get16(SOCK_BASE[s] + SR_TX_RD));
    set16(SOCK_BASE[s] + SR_TX_FSR, BUF_SIZE - used);
}

static void close_sock(int s) {
    if (sock_fd[s] != INVALID_SOCK) {
        close_fd(sock_fd[s]);
        sock_fd[s] = INVALID_SOCK;
    }
}

// ---------------------------------------------------------------------------
// Receive polling
// ---------------------------------------------------------------------------

// Pull any available data from the host socket into the W5100S RX ring buffer.
// For UDP sockets the W5100S prepends an 8-byte header per datagram:
//   4 B source IP | 2 B source port | 2 B payload length
static void poll_rx(int s) {
    if (sock_fd[s] == INVALID_SOCK) return;

    uint8_t sr = regs[SOCK_BASE[s] + SR_SR];
    if (sr != SSTAT_ESTABLISHED && sr != SSTAT_UDP) return;

    uint16_t used  = static_cast<uint16_t>(rx_wr[s] - get16(SOCK_BASE[s] + SR_RX_RD));
    uint16_t space = BUF_SIZE - used;
    if (space == 0) return;

    uint8_t mode = regs[SOCK_BASE[s] + SR_MR] & 0x0F;

    if (mode == SMODE_UDP) {
        // Leave room for the 8-byte header we will prepend
        if (space <= 8) return;
        uint16_t payload_space = space - 8;

        uint8_t tmp[2048];
        struct sockaddr_in src{};
        socklen_t srclen = sizeof(src);
        ssize_t n = recvfrom(sock_fd[s],
                             reinterpret_cast<char*>(tmp),
                             payload_space < sizeof(tmp) ? payload_space : sizeof(tmp),
                             0,
                             reinterpret_cast<struct sockaddr*>(&src), &srclen);
        if (n <= 0) return;

        // Write 8-byte W5100S UDP header
        uint32_t src_ip = ntohl(src.sin_addr.s_addr);
        uint16_t src_port = ntohs(src.sin_port);

        auto write_rx = [&](uint8_t b) {
            regs[RX_BASE[s] + (rx_wr[s] & BUF_MASK)] = b;
            rx_wr[s]++;
        };
        write_rx(static_cast<uint8_t>(src_ip >> 24));
        write_rx(static_cast<uint8_t>(src_ip >> 16));
        write_rx(static_cast<uint8_t>(src_ip >> 8));
        write_rx(static_cast<uint8_t>(src_ip));
        write_rx(static_cast<uint8_t>(src_port >> 8));
        write_rx(static_cast<uint8_t>(src_port));
        write_rx(static_cast<uint8_t>(n >> 8));
        write_rx(static_cast<uint8_t>(n));

        for (ssize_t i = 0; i < n; i++) write_rx(tmp[i]);

    } else {
        uint8_t tmp[2048];
        ssize_t n = recv(sock_fd[s],
                         reinterpret_cast<char*>(tmp),
                         space < sizeof(tmp) ? space : sizeof(tmp),
                         0);
        if (n < 0) return;
        if (n == 0) {
            regs[SOCK_BASE[s] + SR_SR] = SSTAT_CLOSE_WAIT;
            close_sock(s);
            return;
        }
        for (ssize_t i = 0; i < n; i++) {
            regs[RX_BASE[s] + (rx_wr[s] & BUF_MASK)] = tmp[i];
            rx_wr[s]++;
        }
    }

    update_rx_rsr(s);
}

// Poll a non-blocking connect() for completion.
static void poll_connect(int s) {
    if (sock_fd[s] == INVALID_SOCK) return;
    if (regs[SOCK_BASE[s] + SR_SR] != SSTAT_SYNSENT) return;

    fd_set wfds, efds;
    FD_ZERO(&wfds); FD_SET(sock_fd[s], &wfds);
    FD_ZERO(&efds); FD_SET(sock_fd[s], &efds);
    struct timeval tv = {0, 0};

    if (select(static_cast<int>(sock_fd[s]) + 1, nullptr, &wfds, &efds, &tv) > 0) {
        int err = 0;
        socklen_t len = sizeof(err);
        getsockopt(sock_fd[s], SOL_SOCKET, SO_ERROR,
                   reinterpret_cast<char*>(&err), &len);
        if (err == 0) {
            regs[SOCK_BASE[s] + SR_SR] = SSTAT_ESTABLISHED;
            set16(SOCK_BASE[s] + SR_TX_FSR, BUF_SIZE);
        } else {
            regs[SOCK_BASE[s] + SR_SR] = SSTAT_CLOSED;
            close_sock(s);
        }
    }
}

// ---------------------------------------------------------------------------
// Socket command dispatch
// ---------------------------------------------------------------------------

static void handle_command(int s, uint8_t cmd) {
    uint8_t mode = regs[SOCK_BASE[s] + SR_MR] & 0x0F;

    switch (cmd) {

    case SCMD_OPEN:
        close_sock(s);
        sock_fd[s] = socket(AF_INET,
                            mode == SMODE_UDP ? SOCK_DGRAM : SOCK_STREAM,
                            0);
        if (sock_fd[s] != INVALID_SOCK) {
            set_nonblocking(sock_fd[s]);
            // Bind to the source IP the CPC configured in SIPR (0x000F–0x0012).
            // If the host has that IP on any interface, traffic will originate
            // from it.  Silently fall back to the host IP if bind fails.
            uint32_t sipr = (static_cast<uint32_t>(regs[0x000F]) << 24) |
                            (static_cast<uint32_t>(regs[0x0010]) << 16) |
                            (static_cast<uint32_t>(regs[0x0011]) <<  8) |
                             static_cast<uint32_t>(regs[0x0012]);
            if (sipr != 0) {
                struct sockaddr_in src{};
                src.sin_family      = AF_INET;
                src.sin_port        = 0;
                src.sin_addr.s_addr = htonl(sipr);
                bind(sock_fd[s], reinterpret_cast<struct sockaddr*>(&src), sizeof(src));
            }
            regs[SOCK_BASE[s] + SR_SR] = (mode == SMODE_TCP) ? SSTAT_INIT : SSTAT_UDP;
            set16(SOCK_BASE[s] + SR_TX_FSR, BUF_SIZE);
            set16(SOCK_BASE[s] + SR_TX_WR, 0);
            set16(SOCK_BASE[s] + SR_TX_RD, 0);
            set16(SOCK_BASE[s] + SR_RX_RSR, 0);
            set16(SOCK_BASE[s] + SR_RX_RD, 0);
            rx_wr[s] = 0;
        }
        break;

    case SCMD_CONNECT: {
        uint8_t  ip0   = regs[SOCK_BASE[s] + SR_DIPR];
        uint8_t  ip1   = regs[SOCK_BASE[s] + SR_DIPR + 1];
        uint8_t  ip2   = regs[SOCK_BASE[s] + SR_DIPR + 2];
        uint8_t  ip3   = regs[SOCK_BASE[s] + SR_DIPR + 3];
        uint16_t dport = get16(SOCK_BASE[s] + SR_DPORT);

        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(dport);
        addr.sin_addr.s_addr = htonl(
            (static_cast<uint32_t>(ip0) << 24) |
            (static_cast<uint32_t>(ip1) << 16) |
            (static_cast<uint32_t>(ip2) <<  8) |
             static_cast<uint32_t>(ip3));

        int r = connect(sock_fd[s],
                        reinterpret_cast<struct sockaddr*>(&addr),
                        sizeof(addr));
        if (r == 0 || connect_pending()) {
            regs[SOCK_BASE[s] + SR_SR] = SSTAT_SYNSENT;
        } else {
            regs[SOCK_BASE[s] + SR_SR] = SSTAT_CLOSED;
            close_sock(s);
        }
        break;
    }

    case SCMD_SEND: {
        uint16_t tx_rd = get16(SOCK_BASE[s] + SR_TX_RD);
        uint16_t tx_wr = get16(SOCK_BASE[s] + SR_TX_WR);
        uint16_t len   = static_cast<uint16_t>(tx_wr - tx_rd);

        if (len > 0 && sock_fd[s] != INVALID_SOCK) {
            uint8_t buf[2048];
            for (uint16_t i = 0; i < len; i++)
                buf[i] = regs[TX_BASE[s] + ((tx_rd + i) & BUF_MASK)];

            if (mode == SMODE_UDP) {
                uint8_t  ip0   = regs[SOCK_BASE[s] + SR_DIPR];
                uint8_t  ip1   = regs[SOCK_BASE[s] + SR_DIPR + 1];
                uint8_t  ip2   = regs[SOCK_BASE[s] + SR_DIPR + 2];
                uint8_t  ip3   = regs[SOCK_BASE[s] + SR_DIPR + 3];
                uint16_t dport = get16(SOCK_BASE[s] + SR_DPORT);

                struct sockaddr_in dst{};
                dst.sin_family = AF_INET;
                dst.sin_port   = htons(dport);
                dst.sin_addr.s_addr = htonl(
                    (static_cast<uint32_t>(ip0) << 24) |
                    (static_cast<uint32_t>(ip1) << 16) |
                    (static_cast<uint32_t>(ip2) <<  8) |
                     static_cast<uint32_t>(ip3));
                sendto(sock_fd[s], reinterpret_cast<const char*>(buf), len, 0,
                       reinterpret_cast<struct sockaddr*>(&dst), sizeof(dst));
            } else {
                send(sock_fd[s], reinterpret_cast<const char*>(buf), len, 0);
            }
        }

        set16(SOCK_BASE[s] + SR_TX_RD, tx_wr);
        update_tx_fsr(s);
        break;
    }

    case SCMD_RECV:
        update_rx_rsr(s);
        poll_rx(s);
        break;

    case SCMD_DISCON:
    case SCMD_CLOSE:
        close_sock(s);
        regs[SOCK_BASE[s] + SR_SR] = SSTAT_CLOSED;
        set16(SOCK_BASE[s] + SR_RX_RSR, 0);
        set16(SOCK_BASE[s] + SR_TX_FSR, 0);
        rx_wr[s] = 0;
        break;

    default:
        break;
    }

    regs[SOCK_BASE[s] + SR_CR] = 0x00; // command register self-clears
}

// ---------------------------------------------------------------------------
// Register read/write with side-effects
// ---------------------------------------------------------------------------

static uint8_t reg_read(uint16_t addr) {
    for (int s = 0; s < 4; s++) {
        if (addr == SOCK_BASE[s] + SR_SR) {
            poll_connect(s);
            return regs[addr];
        }
        // Reading either byte of RX_RSR triggers a receive poll
        if (addr == SOCK_BASE[s] + SR_RX_RSR ||
            addr == SOCK_BASE[s] + SR_RX_RSR + 1) {
            poll_rx(s);
            return regs[addr];
        }
    }
    return regs[addr];
}

static void reg_write(uint16_t addr, uint8_t val) {
    regs[addr] = val;
    for (int s = 0; s < 4; s++) {
        if (addr == SOCK_BASE[s] + SR_CR && val != 0) {
            handle_command(s, val);
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void net4cpc_reset() {
#ifdef _WIN32
    static bool wsa_init = false;
    if (!wsa_init) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        wsa_init = true;
    }
#endif
    memset(regs, 0, sizeof(regs));
    idm_ar = 0;
    for (int s = 0; s < 4; s++) {
        close_sock(s);
        rx_wr[s] = 0;
    }
    // MR = 0x03: auto-increment enabled, indirect bus mode active.
    // The n4c-nettools code reads this port to confirm the chip is present.
    regs[0x0000] = 0x03;
}

byte net4cpc_in(byte reg_sel) {
    switch (reg_sel & 0x03) {
    case 0: // MR – direct shortcut to register 0x0000
        return regs[0x0000];
    case 1: // IDM_ARH
        return static_cast<byte>(idm_ar >> 8);
    case 2: // IDM_ARL
        return static_cast<byte>(idm_ar & 0xFF);
    case 3: { // IDM_DR
        byte val = reg_read(idm_ar);
        if (regs[0x0000] & 0x02) idm_ar++; // auto-increment when AI bit set
        return val;
    }
    default:
        return 0xFF;
    }
}

void net4cpc_out(byte reg_sel, byte val) {
    switch (reg_sel & 0x03) {
    case 0: // MR
        regs[0x0000] = val;
        break;
    case 1: // IDM_ARH
        idm_ar = static_cast<uint16_t>((idm_ar & 0x00FFu) | (static_cast<uint16_t>(val) << 8));
        break;
    case 2: // IDM_ARL
        idm_ar = static_cast<uint16_t>((idm_ar & 0xFF00u) | val);
        break;
    case 3: // IDM_DR
        reg_write(idm_ar, val);
        if (regs[0x0000] & 0x02) idm_ar++; // auto-increment when AI bit set
        break;
    }
}
