#include "gev-device.h"

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h> // sigemptyset, sigaction
#include <string.h> // strndup

#include <stdio.h>
#include <gpiod.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <lib_log.h>
#include <lib_common.h>

static void print_help(const char *name) {
    printf("usage: %s <-h> <-i if1,if2> <-c config>\n"
           "\n"
           "    -m --manifest  A comma separated list of manifests\n"
           "    -i --if        A comma separated list of network interfaces.\n"
           "    -p --prod      Enable production mode, this adds a couple of functions\n"
           "    -h             Show this help text\n"
           "\n"
           "Note: The order of the comma separated lists equal the Gig-E-Vision order\n"
           "of their description. e.g. \"-i eth0,eth3\" equates to Gig-E-Vision\n"
           "network-interface 0=eth0 and 1=eth3\n",
           name);
}

int main(int argc, char **argv) {
    uint8_t mac_default[6] = { 0x00, 0x0a, 0x35, 0x00, 0x00, 0x00 };
    struct gev_dev_s gev_main;
    int c, optind;
    const char *ifnames = "eth0"; // eth0
    const char *manifests = "/etc/gige/gev_main_2.7.0.xml";

    memset(&gev_main, 0, sizeof(struct gev_dev_s));

    openlog("gige-vision-server", (LOG_PID | LOG_CONS | LOG_PERROR), LOG_LPR);
    for(;;) {
        static struct option long_options[] = {
            {"help",     no_argument,       0, 'h' },
            {"prod",     no_argument,       0, 'p' },
            {"if",       required_argument, 0, 'i' },
            {"manifest", required_argument, 0, 'm' },
            {}
        };

        c = getopt_long(argc, argv, "hpi:m:", long_options, &optind);

        if(c == -1)
            break;

        switch(c) {
        case 'm': manifests = optarg; break;
        case 'i': ifnames = optarg; break;
        case 'p': gev_main.production = 1; break;
        case 'h': print_help(argv[0]); exit(0);
        default: break;
        }
    }

    if(sig_register(SIGTERM, &sig_handler) != 0) {
        LIB_LOG_ERR("failed to register SIGTERM handler");
    }

    if(sig_register(SIGINT, &sig_handler) != 0) {
        LIB_LOG_ERR("failed to register SIGINT handler");
    }

    get_config(&gev_main);
    gev_main.gvcp = &gvcp;

    if(gev_main.version_hw < HW_VERSION_MAX) {
        strncpy(gvcp.reg.version_str,
                hw_versions[gev_main.version_hw],
                GEV_DEV_VERSION_LEN - 1);
    }
    else {
        strncpy(gvcp.reg.version_str,
                hw_versions[HW_VERSION_MAX],
                GEV_DEV_VERSION_LEN - 1);
    }

    memcpy(gvcp.reg.serial,
           gev_main.sensor_serial,
           sizeof(gev_main.sensor_serial));

    /** GigE-Vision device features */
    if(gvcp_open(&gvcp) < 0) {
        LIB_LOG_ERR("failed to open Gig-E-Vision device");
        return -1;
    }

    // configure netifs
    if(parse_ifnames(&gvcp, ifnames)) {
        LIB_LOG_WARNING("failed to add network interfaces to Gig-E-Vision device");
        goto err_close_device;
    }

    if(fw_get_ipaddr(&gvcp.netif[0].ip4_perma_adr) > 0)
        gvcp.netif[0].ip4_perma_adr = inet_addr("192.168.0.42");

    if(fw_get_netmask(&gvcp.netif[0].ip4_perma_sub) > 0)
        gvcp.netif[0].ip4_perma_sub = inet_addr("255.255.255.0");

    if(fw_get_gateway(&gvcp.netif[0].ip4_perma_gtw) > 0)
        gvcp.netif[0].ip4_perma_gtw = inet_addr("192.168.0.1");

    if(fw_get_ethmode(&gvcp.netif[0].cfg) != 0) {
        fw_set_ethmode(GEV_ETC_P);
        fw_get_ethmode(&gvcp.netif[0].cfg);
    }

    check_mac_address(&gev_main, mac_default);

    if(gev_main.perma_adr && gev_main.perma_sub) {
        lib_netif_adr_add(gvcp.netif[0].ifname, gev_main.perma_adr);
        lib_netif_sub_add(gvcp.netif[0].ifname, gev_main.perma_sub);
    }

    if(parse_manifests(&gvcp, manifests)) {
        LIB_LOG_WARNING("failed to add manifest to Gig-E-Vision device");
        goto err_close_device;
    }

    capture_cfg.aoi.left = gev_main.roi.x;
    capture_cfg.aoi.top = gev_main.roi.y;
    if(capture_probe(&gvcp, &capture_cfg)) {
        LIB_LOG_ERR("failed to probe capture device");
    }

    if(gpio_probe(&gvcp)) {
        LIB_LOG_ERR("failed to probe gpio");
        goto err_close_device;
    }
    if(gpio_add(gpio_cfg, ARRAY_SIZE(gpio_cfg))) {
        LIB_LOG_ERR("failed to add gpio");
        goto err_close_device;
    }

    if(drst_probe(&gvcp, GPIO_SD_SELF_RST_N)) {
        LIB_LOG_ERR("failed to add reset gpio");
        goto err_close_device;
    }

    if(text_probe(&gvcp, gev_main.version_fpga,
                  sizeof(gev_main.version_fpga), "r", "version_fpga"))
        LIB_LOG_ERR("failed to probe text \"version_fpga\"");

    if(text_probe(&gvcp, gev_main.version_boot,
                  sizeof(gev_main.version_boot), "r", "version_boot"))
        LIB_LOG_ERR("failed to probe text \"version_boot\"");

    if(text_probe(&gvcp, gev_main.version_linux,
                  sizeof(gev_main.version_linux), "r", "version_linux"))
        LIB_LOG_ERR("failed to probe text \"version_linux\"");

    // temperature handler
    temperature_probe(&gvcp);

    // Error status
    if(gvcp_add_custom_desc(&gvcp, GVCP_ADV_DESC_AUTO_ADDR, 0x4,
                            err_stat_rd, NULL, &gev_main) == NULL)
        LIB_LOG_ERR("failed to add err_stat");


err_close_device:
    gpio_trigger(GPIO_SRC_ERR, 1);
    gvcp_close(&gvcp);
    closelog();

    return 0;
}

