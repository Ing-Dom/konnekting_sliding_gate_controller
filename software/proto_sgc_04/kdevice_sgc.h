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
#define COMOBJ_error_drivecurrentzero 13
#define COMOBJ_debug1 14
#define COMOBJ_debug2 15
#define COMOBJ_debug3 16
#define COMOBJ_debug4 17
#define COMOBJ_debug5 18
#define COMOBJ_test1 19
#define COMOBJ_test2 20
#define COMOBJ_test3 21
#define PARAM_enable_errormessage 0
#define PARAM_send_status_cyclic 1
#define PARAM_close_time_contact 2
#define PARAM_close_time_nocontact 3
#define PARAM_close_ext 4
#define PARAM_drivecurrent_zero 5
#define PARAM_drivecurrent_jit 6
#define PARAM_drivecurrent_num 7
#define PARAM_drivecurrent_autozero 8
        
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
    /* Index 13 - error_code */ KnxComObject(KNX_DPT_7_001, 0x30),
    /* Index 14 - debug1 */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 15 - debug2 */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 16 - debug3 */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 17 - debug4 */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 18 - debug5 */ KnxComObject(KNX_DPT_12_001, 0x34),
    /* Index 19 - test1 */ KnxComObject(KNX_DPT_1_001, 0x28),
    /* Index 20 - test2 */ KnxComObject(KNX_DPT_1_001, 0x28),
    /* Index 21 - test3 */ KnxComObject(KNX_DPT_1_001, 0x28)
};
const byte KnxDevice::_numberOfComObjects = sizeof (_comObjectsList) / sizeof (KnxComObject); // do not change this code
       
byte KonnektingDevice::_paramSizeList[] = {
    /* Index 0 - enable_errormessage */ PARAM_UINT8,
    /* Index 1 - send_status_cyclic */ PARAM_UINT8,
    /* Index 2 - close_time_contact */ PARAM_UINT8,
    /* Index 3 - close_time_nocontact */ PARAM_UINT8,
    /* Index 4 - close_ext */ PARAM_UINT8,
    /* Index 5 - drivecurrent_zero */ PARAM_UINT16,
    /* Index 6 - drivecurrent_jit */ PARAM_UINT16,
    /* Index 7 - drivecurrent_num */ PARAM_UINT16,
    /* Index 8 - drivecurrent_autozero */ PARAM_UINT8
};
const byte KonnektingDevice::_numberOfParams = sizeof (_paramSizeList); // do not change this code