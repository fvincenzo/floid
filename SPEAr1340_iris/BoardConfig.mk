include device/stm/SPEAr1340/BoardConfig.mk

TARGET_BOARD_PLATFORM := iris
TARGET_DISPLAY := 7inch
BOARD_HAVE_BLUETOOTH := false
TARGET_BOARD_RAM := 1GB
BOARD_IRIS_USE_WIFI := true
#WIFI_MODULE_TYPE := 8192CU
WIFI_MODULE_TYPE := 8188EUS

ifeq ($(BOARD_IRIS_USE_WIFI),true)
        WPA_SUPPLICANT_VERSION := VER_0_8_X
        BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_wext
        BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_wext

ifeq ($(WIFI_MODULE_TYPE),8192CU)
        BOARD_WLAN_DEVICE := 8192cu
        WIFI_DRIVER_MODULE_PATH     :=  "/system/lib/modules/8192cu.ko"
        WIFI_DRIVER_MODULE_NAME     :=  "8192cu"
endif
ifeq ($(WIFI_MODULE_TYPE),8188EUS)
        BOARD_WLAN_DEVICE := 8188eu
        WIFI_DRIVER_MODULE_PATH     :=  "/system/lib/modules/8188eu.ko"
        WIFI_DRIVER_MODULE_NAME     :=  "8188eu"
endif
endif

BOARD_USE_WIFI := false
