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
#define COMOBJ_error_code 12
#define COMOBJ_debug1 13
#define COMOBJ_debug2 14
#define COMOBJ_debug3 15
#define COMOBJ_debug4 16
#define COMOBJ_debug5 17
#define COMOBJ_test1 18
#define COMOBJ_test2 19
#define COMOBJ_test3 20
#define PARAM_close_time_contact 0
#define PARAM_close_time_nocontact 1
#define PARAM_close_ext 2
#define PARAM_enable_errormessage 3
#define PARAM_DRIVECURRENT_ZERO 4
#define PARAM_DRIVECURRENT_JIT 5
#define PARAM_DRIVECURRENT_NUM 6
        
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
    /* Index 12 - error_code */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 13 - debug1 */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 14 - debug2 */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 15 - debug3 */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 16 - debug4 */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 17 - debug5 */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 18 - test1 */ KnxComObject(KNX_DPT_1_001, 0x28),
    /* Index 19 - test2 */ KnxComObject(KNX_DPT_1_001, 0x28),
    /* Index 20 - test3 */ KnxComObject(KNX_DPT_1_001, 0x28)
};
const byte KnxDevice::_numberOfComObjects = sizeof (_comObjectsList) / sizeof (KnxComObject); // do not change this code
       
byte KonnektingDevice::_paramSizeList[] = {
    /* Index 0 - close_time_contact */ PARAM_UINT8,
    /* Index 1 - close_time_nocontact */ PARAM_UINT8,
    /* Index 2 - close_ext */ PARAM_UINT8,
    /* Index 3 - enable_errormessage */ PARAM_UINT8,
    /* Index 4 - DRIVECURRENT_ZERO */ PARAM_UINT16,
    /* Index 5 - DRIVECURRENT_JIT */ PARAM_UINT16,
    /* Index 6 - DRIVECURRENT_NUM */ PARAM_UINT16
};
const byte KonnektingDevice::_numberOfParams = sizeof (_paramSizeList); // do not change this code
