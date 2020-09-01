#include "unittest.h"
#include <uci.h>

//#define DEBUG(format,...) printf(format"\r\n", ##__VA_ARGS__)
#define DEBUG(fmt, ...) 	printf("[%s][%d] ",__FUNCTION__,__LINE__ );\
						printf(fmt, ##__VA_ARGS__);

bool get_wirelesss_cfg_value(char* name, char* value, int value_buf_len)
{
    if(nullptr == name || nullptr == value)
    {
        return false;
    }

    struct uci_context* uci_ctx = uci_alloc_context();
    const char* value_data = nullptr;
    struct uci_package* pkg = nullptr;
    struct uci_element *e;
    const char* NETWORK_UCI_CFG_NAME = "wireless";

    do{
        if(UCI_OK != uci_load(uci_ctx, NETWORK_UCI_CFG_NAME, &pkg))
            break;
        uci_foreach_element(&pkg->sections, e)
        {
            struct uci_section *s = uci_to_section(e);
            value_data = uci_lookup_option_string(uci_ctx, s, name);
            if(nullptr != value_data)
            {
                strncpy(value, value_data, value_buf_len);
                //printf("get wirelsss cfg ok, %s:%s\n", name, value);
                DEBUG("get wireless cfg ok => %s:%s\n", name, value);
            }
        }
        uci_unload(uci_ctx, pkg);
    }while(false);
    

    uci_free_context(uci_ctx);

    return true;
}

std::string get_wireless_info()
{
    char ssid[256]  = {0};
    char key[256]   = {0};
    char encryption[256] = {0};
    auto return_value = get_wirelesss_cfg_value("ssid", ssid, sizeof(ssid));
    if(!return_value){
        DEBUG("read ssid section failed\n");  
        return "";
    }
    return_value = get_wirelesss_cfg_value("key", key, sizeof(key));
    if(!return_value){
        DEBUG("read key section failed\n");
        return "";
    }
    return_value = get_wirelesss_cfg_value("encryption", encryption, sizeof(key));
    if(!return_value)
    {
        DEBUG("read encryption section failed\n");
        return "";
    }
    std::string wifi_info = "ssid=" + std::string(ssid) + ";";
    wifi_info += "key=" + std::string(key) + ";";
    wifi_info += "encryption=" + std::string(encryption) + ";";
    return wifi_info;
}

UNITTEST(uci_wirless_info)
{
    OSTTY::print(OSTTY::GREEN,"uci_wirless_info: %s\n",get_wireless_info().c_str());
}

