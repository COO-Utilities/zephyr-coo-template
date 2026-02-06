# COO Network Configuration Design

## Overview

Network configuration module with mode selector (DHCP vs Static) and NVS override capability.

## Design Decisions

1. **Mode selector**: String enum `CONFIG_COO_NET_MODE="dhcp"` or `"static"`
2. **NVS can override mode**: Runtime commands can switch modes
3. **Static config source**: Kconfig defaults + NVS override

---

## Kconfig Options

```kconfig
config COO_NET_CONFIG
    bool "COO network configuration"
    depends on COO_NETWORK
    depends on SETTINGS
    default n

if COO_NET_CONFIG

config COO_NET_MODE
    string "Network mode (dhcp or static)"
    default "dhcp"
    help
      Network configuration mode:
      - "dhcp": Use DHCP for IP/gateway/netmask
      - "static": Use static configuration from Kconfig/NVS
      Can be overridden at runtime via NVS settings.

# Static mode defaults
config COO_NET_STATIC_IP
    string "Static IP address"
    default "192.168.1.100"

config COO_NET_STATIC_GATEWAY
    string "Static gateway"
    default "192.168.1.1"

config COO_NET_STATIC_NETMASK
    string "Static netmask"
    default "255.255.255.0"

config COO_NET_STATIC_DNS
    string "Static DNS server"
    default "8.8.8.8"

config COO_NET_STATIC_NTP
    string "Static NTP server"
    default "pool.ntp.org"

endif # COO_NET_CONFIG
```

---

## NVS Settings Keys

| Key | Type | Description |
|-----|------|-------------|
| `coo/net/mode` | string | "dhcp" or "static" - overrides Kconfig |
| `coo/net/ip` | string | Static IP override |
| `coo/net/gateway` | string | Gateway override |
| `coo/net/netmask` | string | Netmask override |
| `coo/net/dns` | string | DNS server override |

---

## Configuration Resolution Logic

```
coo_net_config_apply():
1. Determine mode:
   - Read "coo/net/mode" from NVS
   - If not set, use CONFIG_COO_NET_MODE

2. If mode == "dhcp":
   - Start DHCP client
   - IP/gateway/netmask from DHCP
   - DNS from DHCP (or NVS override if set)

3. If mode == "static":
   - Stop DHCP if running
   - For each setting (ip, gateway, netmask, dns):
     a. Check NVS for override
     b. If not set, use Kconfig default
   - Apply to interface
```

---

## API

```c
/**
 * @brief Initialize and apply network configuration
 * @return 0 on success, negative errno on error
 */
int coo_net_config_apply(void);

/**
 * @brief Check if using DHCP mode
 * @return true if DHCP, false if static
 */
bool coo_net_config_is_dhcp(void);

/**
 * @brief Get current IP address
 * @param out Output for IP address
 * @return 0 on success, negative errno on error
 */
int coo_net_config_get_ip(struct in_addr *out);


/* Setters - persist to NVS, call apply() to take effect */
int coo_net_config_set_mode(const char *mode);  /* "dhcp" or "static" */
int coo_net_config_set_ip(const char *ip);
int coo_net_config_set_gateway(const char *gw);
int coo_net_config_set_netmask(const char *mask);
int coo_net_config_set_dns(const char *dns);
```

---

## Files to Create/Modify

### New Files
1. `include/coo_commons/net_config.h` - Public API
2. `lib/coo_commons/net_config.c` - Implementation

### Modified Files
3. `lib/coo_commons/Kconfig` - Add COO_NET_CONFIG block
4. `lib/coo_commons/CMakeLists.txt` - Add source

---

## Zephyr APIs Used

| API | Purpose |
|-----|---------|
| `net_dhcpv4_start()` / `net_dhcpv4_stop()` | DHCP control |
| `net_if_ipv4_addr_add()` | Set static IP |
| `net_if_ipv4_set_gw()` | Set gateway |
| `net_if_ipv4_set_netmask_by_addr()` | Set netmask |
| `settings_runtime_get/set()` | NVS access |

---

## Testing Plan

1. Build with `CONFIG_COO_NET_MODE="dhcp"` - verify DHCP works
2. Build with `CONFIG_COO_NET_MODE="static"` - verify static IP applied
3. Set `coo/net/mode` to "static" via NVS - verify mode switch works
4. Set `coo/net/ip` override - verify it takes precedence over Kconfig
