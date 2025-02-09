#include "my_wifi.h"
#include "pico/cyw43_arch.h"
#include "my_std_debug.h"

#define MY_WIFI_CONNECT_TIMEOUT_MS 30000UL

my_std_ret_t my_wifi_init(const char *wifi_ssid, const char *wifi_password)
{
    // inicializa modulo bluetooth/wifi
    if (cyw43_arch_init())
    {
        return MY_STD_RET_FAIL;
    }

    // Habilita wifi station
    cyw43_arch_enable_sta_mode();

    // Tenta conexao na rede wifi
    if (cyw43_arch_wifi_connect_timeout_ms(wifi_ssid, wifi_password, CYW43_AUTH_WPA2_AES_PSK, MY_WIFI_CONNECT_TIMEOUT_MS))
    {
        return MY_STD_RET_FAIL;
    }

    #if GLOBAL_DEBUG_ENABLED
    printf("Wifi Conectado com sucesso!.\n");
    uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
    printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    #endif

    return MY_STD_RET_OK;
}
