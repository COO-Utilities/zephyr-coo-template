# COO Time Sync Implementation Plan

## Overview

Implement a network time synchronization module for coo-commons that provides:
- Layered NTP server discovery (Settings/NVS → DHCP → DNS → Build-time default)
- SNTP-based clock synchronization with periodic resync
- Valid time gating for certain systems

## Goals

1. Provide a firmware-level wall clock (CLOCK_REALTIME) when network is available
2. Keep behavior deterministic across projects via coo-commons
3. Support multiple ways to discover NTP servers (config, DHCP, DNS), with clear precedence
4. Allow "time-valid" gating for telemetry/logging/TLS decisions

---

## Files to Create/Modify

### New Files
1. `include/coo_commons/time_sync.h` - Public API header
2. `lib/coo_commons/time_sync.c` - Implementation

### Modified Files
3. `lib/coo_commons/Kconfig` - Add COO_TIME_SYNC config block
4. `lib/coo_commons/CMakeLists.txt` - Add conditional source

---

## API Design

### Server Discovery API

```c
#define COO_TIME_MAX_SERVERS 4
#define COO_TIME_SERVER_STRLEN 128

enum coo_time_server_source {
    COO_TIME_SRC_SETTINGS,  /* NVS/settings */
    COO_TIME_SRC_DHCP,      /* DHCP option 42 */
    COO_TIME_SRC_DNS,       /* Well-known DNS name */
    COO_TIME_SRC_BUILD,     /* Build-time default (e.g. Kconfig) */
};

struct coo_time_server_candidate {
    char server[COO_TIME_SERVER_STRLEN];
    enum coo_time_server_source source;
};

/**
 * @brief Resolve NTP server candidates using layered discovery
 *
 * @param out      Output array for candidates
 * @param out_cap  Capacity of output array
 * @param out_len  Number of candidates found
 * @return 0 on success, negative errno on error
 */
int coo_time_server_resolve(struct coo_time_server_candidate *out,
                            size_t out_cap, size_t *out_len);
```

### Time Sync Service API

```c
/**
 * @brief Start the time sync service (spawns worker)
 * @return 0 on success, negative errno on error
 */
int coo_time_sync_start(void);

/**
 * @brief Stop the time sync service
 */
void coo_time_sync_stop(void);

/**
 * @brief Check if wall clock time is valid (synced at least once)
 * @return true if CLOCK_REALTIME is set and trusted
 */
bool coo_time_is_valid(void);

/**
 * @brief Get current wall clock time in milliseconds since epoch
 *
 * @param out  Output for timestamp
 * @return 0 on success, -ENODATA if time not yet valid
 */
int coo_time_now_ms(int64_t *out);

/**
 * @brief Callback type for time-valid events
 */
typedef void (*coo_time_valid_cb_t)(void *user);
```

---

## Implementation Details

### Server Resolution Flow (`coo_time_server_resolve()`)

```
1. Initialize output count to 0
2. If COO_TIME_SOURCE_SETTINGS enabled:
   - Read "coo/time/ntp_server" from settings
   - If found and non-empty, add to candidates with SRC_SETTINGS
3. If COO_TIME_SOURCE_DHCP enabled:
   - Get default interface via net_if_get_default()
   - Check iface->config.dhcpv4.ntp_addr
   - If not unspecified, convert to string, add with SRC_DHCP
4. If COO_TIME_SOURCE_DNS enabled:
   - Add CONFIG_COO_TIME_DNS hostname with SRC_DNS
5. If CONFIG_COO_TIME_SERVER_DEFAULT is non-empty:
   - Add with SRC_BUILD
6. Deduplicate (skip servers already in list)
7. Return count of candidates
```

### Sync Worker Flow

Uses system work queue with `k_work_delayable`:

```
sync_work_handler():
1. If !coo_network_is_ready(), reschedule work and return
2. Call coo_time_server_resolve() to get candidates
3. For each candidate in order:
   a. Call sntp_simple(server, timeout_ms, &sntp_time)
   b. If success:
      - Convert sntp_time.seconds → struct timespec
      - Call clock_settime(CLOCK_REALTIME, &ts)
      - Set time_valid = true
      - Fire registered callbacks
      - Break loop
   c. If failure, LOG_WRN and try next
4. If all failed, LOG_ERR but keep any existing time
5. Schedule next sync (CONFIG_COO_TIME_SYNC_PERIOD_SECONDS)
```

---

## Zephyr APIs Used

| API | Purpose |
|-----|---------|
| `sntp_simple(server, timeout, &ts)` | Convenience SNTP query function |
| `clock_settime(CLOCK_REALTIME, &ts)` | Set system wall clock |
| `clock_gettime(CLOCK_REALTIME, &ts)` | Get current wall clock |
| `settings_runtime_get()` | Read NVS/settings |
| `k_work_delayable` | Periodic sync scheduling |

---

## Dependencies

**Required Zephyr configs:**
- `CONFIG_SNTP=y` - Zephyr SNTP client
- `CONFIG_POSIX_TIMERS=y` - For clock_settime/clock_gettime
- `CONFIG_NETWORKING=y` - Network stack
- `CONFIG_NET_DHCPV4_OPTION_NTP_SERVER=y` - For DHCP NTP (if using that source)

---

## Expected Usage

```c
#include <coo_commons/time_sync.h>

void time_ready_callback(void *user)
{
    LOG_INF("Time is now valid, enabling TLS connections");
}

int main(void)
{
    /* Initialize networking first */
    coo_network_init(NULL);

    /* Start time sync service */
    coo_time_sync_start();

    /* ... application code ... */

    /* Gate time-sensitive operations */
    if (coo_time_is_valid()) {
        int64_t now_ms;
        coo_time_now_ms(&now_ms);
        /* Use wall clock timestamp */
    } else {
        /* Fall back to monotonic time */
        int64_t uptime = k_uptime_get();
    }
}
```

---

## Testing Plan

1. **Unit test on QEMU** - Configure with build-time default server, verify time sync
2. **DHCP test** - Use QEMU with DHCP server providing NTP option 42
3. **Validity gating test** - Verify `coo_time_is_valid()` returns false before sync, true after
4. **Periodic resync test** - Verify resync happens after configured interval

---

## Possible Future Enhancements

- **MQTT provisioning source**: Allow MQTT to update the settings/NVS override server
