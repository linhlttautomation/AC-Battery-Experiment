
/*
 * ID_CAN.h
 *
 *  Created on: 10 Oct 2017
 *      Author: dinhngock6
 */

#ifndef _ID_CAN_H
#define _ID_CAN_H

#ifndef TRUE
#define TRUE        1
#endif

#ifndef FALSE
#define FALSE       0
#endif

#ifndef ON
#define ON          1
#endif

#ifndef OFF
#define OFF         0
#endif

//=========================================================
// Ma ID MASTER --> DSP
//=========================================================

// CMD
#define MASTER_RUN_SLAVE_PV                  0x0101
#define MASTER_STOP_SLAVE_PV                 0x0102
#define MASTER_CONFIG_SLAVE_PV               0x0103
#define MASTER_CHECKING_SLAVE_PV             0x0104

#define MASTER_RUN_SLAVE_BAT                 0x0105
#define MASTER_STOP_SLAVE_BAT                0x0106
#define MASTER_CONFIG_SLAVE_BAT              0x0107
#define MASTER_CHECKING_SLAVE_BAT            0x0108

#define MASTER_RUN_SLAVE_DCAC                0x0109
#define MASTER_STOP_SLAVE_DCAC               0x010A
#define MASTER_CONFIG_SLAVE_DCAC             0x010B
#define MASTER_CHECKING_SLAVE_DCAC           0x010C

#define MASTER_RESET_ALL_SLAVE               0x010D

// CONF
#define MASTER_CONFIG_CONTROL_MODE           0x0110

#define MASTER_CONFIG_PARAMETER_POWER        0x0111
#define MASTER_CONFIG_PARAMETER_VOLTAGE_DC   0x0112
#define MASTER_CONFIG_PARAMETERVOLTAGE_AC    0x0113
#define MASTER_CONFIG_PARAMETER_COSPHI       0x0114
#define MASTER_CONFIG_PARAMETER_FREQUENCY    0x0115

#define MASTER_CONFIG_PARAMETER_BAT_COFF     0x0116
#define MASTER_CONFIG_PARAMETER_BAT_RE       0x0117
#define MASTER_CONFIG_PARAMETER_BAT_TCHAR    0x0118
#define MASTER_CONFIG_PARAMETER_BAT_VCHAR    0x0119

#define MASTER_CONFIG_LIMIT_VDC              0x011A
#define MASTER_CONFIG_LIMIT_IDC_MAX          0x011B
#define MASTER_CONFIG_LIMIT_VGRID            0x011C
#define MASTER_CONFIG_LIMIT_IGRID_MAX        0x011D
#define MASTER_CONFIG_LIMIT_VLOAD            0x011E
#define MASTER_CONFIG_LIMIT_ILOAD_MAX        0x011F
#define MASTER_CONFIG_LIMIT_FREQ             0x0120

#define MASTER_CONFIG_LIMIT_VPV              0x0121
#define MASTER_CONFIG_LIMIT_IPV_MAX          0x0122

#define MASTER_CONFIG_LIMIT_VDC_BAT          0x0123
#define MASTER_CONFIG_LIMIT_VBAT             0x0124
#define MASTER_CONFIG_LIMIT_IBAT_MAX         0x0125

#define MASTER_CONFIG_LIMIT_TEMP_MAX         0x0126

#define MASTER_CONFIG_CONTROLLER_CURRENT_AC  0x0127
#define MASTER_CONFIG_CONTROLLER_VOLTAGE_AC  0x0128
#define MASTER_CONFIG_CONTROLLER_VOLTAGE_DC  0x0129

#define MASTER_CONFIG_CONTROLLER_BAT         0x012A

#define MASTER_CONFIG_CONTROLLER_PV          0x012B

// SET
#define MASTER_SET_VDC_REF                   0x0130

#define MASTER_SET_VDC_BAT_REF               0x0131
#define MASTER_SET_VBAT_REF                  0x0132
#define MASTER_SET_IBAT_REF                  0x0133

#define MASTER_SET_VAC_REF                   0x0134

#define MASTER_REQUEST_VPV                   0x0140
#define MASTER_REQUEST_IPV                   0x0141
#define MASTER_REQUEST_POW_PV                0x0142

#define MASTER_REQUEST_VBAT                  0x0143
#define MASTER_REQUEST_IBAT                  0x0144
#define MASTER_REQUEST_POW_BAT               0x0145

#define MASTER_REQUEST_VDC                   0x0146
#define MASTER_REQUEST_IGRID                 0x0147
#define MASTER_REQUEST_VGRID                 0x0148
#define MASTER_REQUEST_FREQ_GRID             0x0149
#define MASTER_REQUEST_POW_GRID              0x014A
#define MASTER_REQUEST_ILOAD                 0x014B
#define MASTER_REQUEST_VLOAD                 0x014C
#define MASTER_REQUEST_POW_LOAD              0x014D

//=========================================================
// Ma ID PV --> MASTER
//=========================================================
// OK

#define SLAVE_PV_OK_RUN                      0x0200
#define SLAVE_PV_OK_STOP                     0x0201
#define SLAVE_PV_OK_CHECKING                 0x0202

// ERROR
#define SLAVE_PV_ERROR_RUN                   0x0203
#define SLAVE_PV_ERROR_CHECKING              0x0204
#define SLAVE_ERROR_IPV1_HIGH                0x0205
#define SLAVE_ERROR_VPV1_HIGH                0x0206
#define SLAVE_ERROR_VPV1_LOW                 0x0207
#define SLAVE_ERROR_IPV2_HIGH                0x0208
#define SLAVE_ERROR_VPV2_HIGH                0x0209
#define SLAVE_ERROR_VPV2_LOW                 0x020A
#define SLAVE_ERROR_OVER_CURRENT_PV          0x020B
#define SLAVE_ERROR_TEMP_PV1_HIGH            0x020C
#define SLAVE_ERROR_TEMP_PV2_HIGH            0x020D

// RESPOND
#define SLAVE_RESPOSE_VPV                    0x0210
#define SLAVE_RESPOSE_IPV                    0x0211
#define SLAVE_RESPOSE_POW_PV                 0x0212

//=========================================================
// Ma ID BAT --> MASTER
//=========================================================
// OK

#define SLAVE_BAT_OK_RUN                     0x0300
#define SLAVE_BAT_OK_STOP                    0x0301
#define SLAVE_BAT_OK_CHECKING                0x0302

// ERROR
#define SLAVE_BAT_ERROR_RUN                  0x0303
#define SLAVE_BAT_ERROR_CHECKING             0x0304
#define SLAVE_ERROR_IBAT_HIGH                0x0305
#define SLAVE_ERROR_VBAT_HIGH                0x0306
#define SLAVE_ERROR_VBAT_LOW                 0x0307
#define SLAVE_ERROR_VDC_BAT_HIGH             0x0308
#define SLAVE_ERROR_VDC_BAT_LOW              0x0309
#define SLAVE_ERROR_CONTROLLER_BAT           0x030A
#define SLAVE_ERROR_OVER_CURRENT_BAT         0x030B
#define SLAVE_ERROR_TEMP_BAT_HIGH            0x030C

// RESPOND
#define SLAVE_RESPOSE_VBAT                   0x030D
#define SLAVE_RESPOSE_IBAT                   0x030E
#define SLAVE_RESPOSE_POW_BAT                0x030F

//=========================================================
// Ma ID DCAC --> MASTER
//=========================================================
// OK

#define SLAVE_DCAC_OK_RUN                    0x0400
#define SLAVE_DCAC_OK_STOP                   0x0401
#define SLAVE_DCAC_OK_CHECKING               0x0402

// ERROR
#define SLAVE_DCAC_ERROR_RUN                 0x0403
#define SLAVE_DCAC_ERROR_CHECKING            0x0404
#define SLAVE_ERROR_VDC_HIGH                 0x0405
#define SLAVE_ERROR_VDC_LOW                  0x0406
#define SLAVE_ERROR_ILOAD_HIGH               0x0407
#define SLAVE_ERROR_VLOAD_HIGH               0x0408
#define SLAVE_ERROR_VLOAD_LOW                0x0409
#define SLAVE_ERROR_IGRID_HIGH               0x040A
#define SLAVE_ERROR_VGRID_HIGH               0x040B
#define SLAVE_ERROR_VGRID_LOW                0x040C
#define SLAVE_ERROR_TEMP_DCAC_HIGH           0x040D
#define SLAVE_ERROR_FAULT_IGBT               0x040E
#define SLAVE_ERROR_CONTROLLER_DCAC          0x040F
#define SLAVE_ERROR_OVER_CURRENT_DCAC        0x0410
#define SLAVE_ERROR_PEAK_CURRENT_DCAC        0x0411

// RESPOND
#define SLAVE_RESPOSE_VDC                    0x0412
#define SLAVE_RESPOSE_VGRID                  0x0413
#define SLAVE_RESPOSE_IGRID                  0x0414
#define SLAVE_RESPOSE_POW_GRID               0x0415
#define SLAVE_RESPOSE_VLOAD                  0x0416
#define SLAVE_RESPOSE_ILOAD                  0x0417
#define SLAVE_RESPOSE_POW_LOAD               0x0418
#define SLAVE_RESPOSE_FREQ                   0x0419

#endif
