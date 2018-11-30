#define MANUFACTURER_ID 7337
#define DEVICE_ID 10
#define REVISION 0

#define COMOBJ_cmd_close 0
#define COMOBJ_cmd_open 1
#define COMOBJ_cmd_leave_open 2
#define COMOBJ_cmd_partly_open 3
#define COMOBJ_cmd_stop 4
#define COMOBJ_cmd_position 5
#define COMOBJ_stat_closed 6
#define COMOBJ_stat_opened 7
#define COMOBJ_stat_moving 8
#define COMOBJ_stat_moving_direction 9
#define COMOBJ_stat_position 10
#define COMOBJ_stat_barrier 11
#define COMOBJ_debug 12
#define PARAM_close_time 0
#define PARAM_close_time 1
#define PARAM_close_ext 2
        
KnxComObject KnxDevice::_comObjectsList[] = {
    /* Index 0 - cmd_close */ KnxComObject(KNX_DPT_1_001, 0x28),
    /* Index 1 - cmd_open */ KnxComObject(KNX_DPT_1_001, 0x28),
    /* Index 2 - cmd_leave_open */ KnxComObject(KNX_DPT_1_001, 0x28),
    /* Index 3 - cmd_partly_open */ KnxComObject(KNX_DPT_1_001, 0x28),
    /* Index 4 - cmd_stop */ KnxComObject(KNX_DPT_1_001, 0x28),
    /* Index 5 - cmd_position */ KnxComObject(KNX_DPT_5_001, 0x28),
    /* Index 6 - stat_closed */ KnxComObject(KNX_DPT_1_002, 0x34),
    /* Index 7 - stat_opened */ KnxComObject(KNX_DPT_1_002, 0x34),
    /* Index 8 - stat_moving */ KnxComObject(KNX_DPT_1_002, 0x34),
    /* Index 9 - stat_moving_direction */ KnxComObject(KNX_DPT_1_009, 0x34),
    /* Index 10 - stat_position */ KnxComObject(KNX_DPT_5_001, 0x34),
    /* Index 11 - stat_barrier */ KnxComObject(KNX_DPT_1_002, 0x34),
    /* Index 12 - debug */ KnxComObject(KNX_DPT_12_001, 0x34)
};
const byte KnxDevice::_numberOfComObjects = sizeof (_comObjectsList) / sizeof (KnxComObject); // do not change this code
       
byte KonnektingDevice::_paramSizeList[] = {
    /* Index 0 - close_time */ PARAM_UINT8,
    /* Index 1 - close_time */ PARAM_UINT8,
    /* Index 2 - close_ext */ PARAM_UINT8
};
const byte KonnektingDevice::_numberOfParams = sizeof (_paramSizeList); // do not change this code
