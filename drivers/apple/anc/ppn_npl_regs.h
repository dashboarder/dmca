
//
// Copyright (C) 2010 Apple Inc. All rights reserved.
//
// This document is the property of Apple Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form
// in whole or in part, without the express written permission of
// Apple Inc.
//
#ifndef __PPNNPL_H__
#define __PPNNPL_H__          

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_PPNNPL_BASE                                      0x00000000
#define A_PPNNPL_REG(REG)                                  (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_PPNNPL_VERSION                                   0x00000000   
#define RRV_PPNNPL_VERSION                                 0x00000000  
#define RRM_PPNNPL_VERSION                                 0x00000000            
#define R_PPNNPL_CONFIG                                    0x00000004   
#define RRV_PPNNPL_CONFIG                                  0x00000000  
#define RRM_PPNNPL_CONFIG                                  0x00000000            
#define R_PPNNPL_DQS_TIMING                                0x00000008   
#define RRV_PPNNPL_DQS_TIMING                              0x003f003f  
#define RRM_PPNNPL_DQS_TIMING                              0x00000000            
#define R_PPNNPL_DLL_CODES                                 0x0000000c   
#define RRV_PPNNPL_DLL_CODES                               0x00000000  
#define RRM_PPNNPL_DLL_CODES                               0x00000000            
#define R_PPNNPL_DQS_ADJUST                                0x00000010   
#define RRV_PPNNPL_DQS_ADJUST                              0x00400040  
#define RRM_PPNNPL_DQS_ADJUST                              0x00000000            
#define R_PPNNPL_AMPMCDLL_CONTROL                          0x00000014   
#define RRV_PPNNPL_AMPMCDLL_CONTROL                        0x00000000  
#define RRM_PPNNPL_AMPMCDLL_CONTROL                        0x00000000            
#define R_PPNNPL_AMPSLV_CONTROL                            0x00000018   
#define RRV_PPNNPL_AMPSLV_CONTROL                          0x00000000  
#define RRM_PPNNPL_AMPSLV_CONTROL                          0x00000000            
#define R_PPNNPL_AMPDLL_STATUS                             0x0000001c   
#define RRV_PPNNPL_AMPDLL_STATUS                           0x00000000  
#define RRM_PPNNPL_AMPDLL_STATUS                           0x00000000            
#define R_PPNNPL_ANC0_AMPSLV0                              0x00000020   
#define RRV_PPNNPL_ANC0_AMPSLV0                            0x00000000  
#define RRM_PPNNPL_ANC0_AMPSLV0                            0x00000000            
#define R_PPNNPL_ANC0_AMPSLV1                              0x00000024   
#define RRV_PPNNPL_ANC0_AMPSLV1                            0x00000000  
#define RRM_PPNNPL_ANC0_AMPSLV1                            0x00000000            
#define R_PPNNPL_ANC1_AMPSLV0                              0x00000028   
#define RRV_PPNNPL_ANC1_AMPSLV0                            0x00000000  
#define RRM_PPNNPL_ANC1_AMPSLV0                            0x00000000            
#define R_PPNNPL_ANC1_AMPSLV1                              0x0000002c   
#define RRV_PPNNPL_ANC1_AMPSLV1                            0x00000000  
#define RRM_PPNNPL_ANC1_AMPSLV1                            0x00000000            
#define R_PPNNPL_TRAIN_CONTROL                             0x00000030   
#define RRV_PPNNPL_TRAIN_CONTROL                           0x00000000  
#define RRM_PPNNPL_TRAIN_CONTROL                           0x00000000            
#define R_PPNNPL_PERIODIC_TRAINING                         0x00000034   
#define RRV_PPNNPL_PERIODIC_TRAINING                       0x00000000  
#define RRM_PPNNPL_PERIODIC_TRAINING                       0x00000000            
#define R_PPNNPL_DEBUG_DLL_CONTROL                         0x00000038   
#define RRV_PPNNPL_DEBUG_DLL_CONTROL                       0x4c000034  
#define RRM_PPNNPL_DEBUG_DLL_CONTROL                       0x00000000            
#define R_PPNNPL_DEBUG_DATA_CAPTURE                        0x0000003c   
#define RRV_PPNNPL_DEBUG_DATA_CAPTURE                      0x00000000  
#define RRM_PPNNPL_DEBUG_DATA_CAPTURE                      0x00000000            
#define R_PPNNPL_PPN0_CLE                                  0x00000040   
#define RRV_PPNNPL_PPN0_CLE                                0x00000000  
#define RRM_PPNNPL_PPN0_CLE                                0x00000000            
#define R_PPNNPL_PPN0_ALE                                  0x00000044   
#define RRV_PPNNPL_PPN0_ALE                                0x00000000  
#define RRM_PPNNPL_PPN0_ALE                                0x00000000            
#define R_PPNNPL_PPN0_REN                                  0x00000048   
#define RRV_PPNNPL_PPN0_REN                                0x00000000  
#define RRM_PPNNPL_PPN0_REN                                0x00000000            
#define R_PPNNPL_PPN0_WEN                                  0x0000004c   
#define RRV_PPNNPL_PPN0_WEN                                0x00000000  
#define RRM_PPNNPL_PPN0_WEN                                0x00000000            
#define R_PPNNPL_PPN0_CEN                                  0x00000050   
#define RRV_PPNNPL_PPN0_CEN                                0x00000000  
#define RRM_PPNNPL_PPN0_CEN                                0x00000000            
#define R_PPNNPL_PPN0_DQS                                  0x00000054   
#define RRV_PPNNPL_PPN0_DQS                                0x00000000  
#define RRM_PPNNPL_PPN0_DQS                                0x00000000            
#define R_PPNNPL_PPN0_IO                                   0x00000058   
#define RRV_PPNNPL_PPN0_IO                                 0x00000000  
#define RRM_PPNNPL_PPN0_IO                                 0x00000000            
#define R_PPNNPL_PPN0_ZQ                                   0x0000005c   
#define RRV_PPNNPL_PPN0_ZQ                                 0x00000000  
#define RRM_PPNNPL_PPN0_ZQ                                 0x00000000
#define R_PPNNPL_PPN0_DS                                   0x00000060
#define RRV_PPNNPL_PPN0_DS                                 0x00000000
#define RRM_PPNNPL_PPN0_DS                                 0x00000000            
#define R_PPNNPL_PPN0_INPUT_SELECT                         0x00000064   
#define RRV_PPNNPL_PPN0_INPUT_SELECT                       0x00000000  
#define RRM_PPNNPL_PPN0_INPUT_SELECT                       0x00000000            
#define R_PPNNPL_PPN1_CLE                                  0x00000068   
#define RRV_PPNNPL_PPN1_CLE                                0x00000000  
#define RRM_PPNNPL_PPN1_CLE                                0x00000000            
#define R_PPNNPL_PPN1_ALE                                  0x0000006c   
#define RRV_PPNNPL_PPN1_ALE                                0x00000000  
#define RRM_PPNNPL_PPN1_ALE                                0x00000000            
#define R_PPNNPL_PPN1_REN                                  0x00000070   
#define RRV_PPNNPL_PPN1_REN                                0x00000000  
#define RRM_PPNNPL_PPN1_REN                                0x00000000            
#define R_PPNNPL_PPN1_WEN                                  0x00000074   
#define RRV_PPNNPL_PPN1_WEN                                0x00000000  
#define RRM_PPNNPL_PPN1_WEN                                0x00000000            
#define R_PPNNPL_PPN1_CEN                                  0x00000078   
#define RRV_PPNNPL_PPN1_CEN                                0x00000000  
#define RRM_PPNNPL_PPN1_CEN                                0x00000000            
#define R_PPNNPL_PPN1_DQS                                  0x0000007c   
#define RRV_PPNNPL_PPN1_DQS                                0x00000000  
#define RRM_PPNNPL_PPN1_DQS                                0x00000000            
#define R_PPNNPL_PPN1_IO                                   0x00000080   
#define RRV_PPNNPL_PPN1_IO                                 0x00000000  
#define RRM_PPNNPL_PPN1_IO                                 0x00000000            
#define R_PPNNPL_PPN1_ZQ                                   0x00000084   
#define RRV_PPNNPL_PPN1_ZQ                                 0x00000000  
#define RRM_PPNNPL_PPN1_ZQ                                 0x00000000            
#define R_PPNNPL_PPN1_DS                                   0x00000088   
#define RRV_PPNNPL_PPN1_DS                                 0x00000000  
#define RRM_PPNNPL_PPN1_DS                                 0x00000000            
#define R_PPNNPL_PPN1_INPUT_SELECT                         0x0000008c   
#define RRV_PPNNPL_PPN1_INPUT_SELECT                       0x00000000  
#define RRM_PPNNPL_PPN1_INPUT_SELECT                       0x00000000   
#define R_PPNNPL_LAST                                      0x0000008c 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rPPNNPL_VERSION                                             
#define rPPNNPL_CONFIG                                              
#define rPPNNPL_DQS_TIMING                                          
#define rPPNNPL_DLL_CODES                                           
#define rPPNNPL_DQS_ADJUST                                          
#define rPPNNPL_AMPMCDLL_CONTROL                                    
#define rPPNNPL_AMPSLV_CONTROL                                      
#define rPPNNPL_AMPDLL_STATUS                                       
#define rPPNNPL_ANC0_AMPSLV0                                        
#define rPPNNPL_ANC0_AMPSLV1                                        
#define rPPNNPL_ANC1_AMPSLV0                                        
#define rPPNNPL_ANC1_AMPSLV1                                        
#define rPPNNPL_TRAIN_CONTROL                                       
#define rPPNNPL_PERIODIC_TRAINING                                   
#define rPPNNPL_DEBUG_DLL_CONTROL                                   
#define rPPNNPL_DEBUG_DATA_CAPTURE                                  
#define rPPNNPL_PPN0_CLE                                            
#define rPPNNPL_PPN0_ALE                                            
#define rPPNNPL_PPN0_REN                                            
#define rPPNNPL_PPN0_WEN                                            
#define rPPNNPL_PPN0_CEN                                            
#define rPPNNPL_PPN0_DQS                                            
#define rPPNNPL_PPN0_IO                                             
#define rPPNNPL_PPN0_ZQ                                             
#define rPPNNPL_PPN0_ZQ_IN                                          
#define rPPNNPL_PPN0_ZCPD                                           
#define rPPNNPL_PPN0_ZCPU                                           
#define rPPNNPL_PPN0_DS                                             
#define rPPNNPL_PPN0_INPUT_SELECT                                   
#define rPPNNPL_PPN1_CLE                                            
#define rPPNNPL_PPN1_ALE                                            
#define rPPNNPL_PPN1_REN                                            
#define rPPNNPL_PPN1_WEN                                            
#define rPPNNPL_PPN1_CEN                                            
#define rPPNNPL_PPN1_DQS                                            
#define rPPNNPL_PPN1_IO                                             
#define rPPNNPL_PPN1_ZQ                                             
#define rPPNNPL_PPN1_ZQ_IN                                          
#define rPPNNPL_PPN1_ZCPD                                           
#define rPPNNPL_PPN1_ZCPU                                           
#define rPPNNPL_PPN1_DS                                             
#define rPPNNPL_PPN1_INPUT_SELECT                               
#define rPPNNPL_LAST                                         
#endif 

/******************************************************************************/
/* Register Fields defines */
/******************************************************************************/          
#define S_PPNNPL_VERSION_REVISION                                                 0  //Access: read-only
#define M_PPNNPL_VERSION_REVISION                                                 _MM_MAKEMASK(16,S_PPNNPL_VERSION_REVISION)
#define V_PPNNPL_VERSION_REVISION(V)                                              _MM_MAKEVALUE((V),S_PPNNPL_VERSION_REVISION)
#define G_PPNNPL_VERSION_REVISION(V)                                              _MM_GETVALUE((V),S_PPNNPL_VERSION_REVISION,M_PPNNPL_VERSION_REVISION)
     
#define S_PPNNPL_VERSION_VERSION                                                  16  //Access: read-only
#define M_PPNNPL_VERSION_VERSION                                                  _MM_MAKEMASK(16,S_PPNNPL_VERSION_VERSION)
#define V_PPNNPL_VERSION_VERSION(V)                                               _MM_MAKEVALUE((V),S_PPNNPL_VERSION_VERSION)
#define G_PPNNPL_VERSION_VERSION(V)                                               _MM_GETVALUE((V),S_PPNNPL_VERSION_VERSION,M_PPNNPL_VERSION_VERSION)

      
#define S_PPNNPL_CONFIG_AUTO_DISABLE_DQS_PULLDOWN                                 0  //Access: read-write
#define M_PPNNPL_CONFIG_AUTO_DISABLE_DQS_PULLDOWN                                 _MM_MAKEMASK(1,S_PPNNPL_CONFIG_AUTO_DISABLE_DQS_PULLDOWN)
#define V_PPNNPL_CONFIG_AUTO_DISABLE_DQS_PULLDOWN(V)                                   _MM_MAKEVALUE((V),S_PPNNPL_CONFIG_AUTO_DISABLE_DQS_PULLDOWN)
#define G_PPNNPL_CONFIG_AUTO_DISABLE_DQS_PULLDOWN(V)                                   _MM_GETVALUE((V),S_PPNNPL_CONFIG_AUTO_DISABLE_DQS_PULLDOWN,M_PPNNPL_CONFIG_AUTO_DISABLE_DQS_PULLDOWN)

#define S_PPNNPL_CONFIG_AUTO_DISABLE_IO_PULLDOWN                                 1  //Access: read-write
#define M_PPNNPL_CONFIG_AUTO_DISABLE_IO_PULLDOWN                                 _MM_MAKEMASK(1,S_PPNNPL_CONFIG_AUTO_DISABLE_IO_PULLDOWN)
#define V_PPNNPL_CONFIG_AUTO_DISABLE_IO_PULLDOWN(V)                                   _MM_MAKEVALUE((V),S_PPNNPL_CONFIG_AUTO_DISABLE_IO_PULLDOWN)
#define G_PPNNPL_CONFIG_AUTO_DISABLE_IO_PULLDOWN(V)                                   _MM_GETVALUE((V),S_PPNNPL_CONFIG_AUTO_DISABLE_IO_PULLDOWN,M_PPNNPL_CONFIG_AUTO_DISABLE_IO_PULLDOWN)
      
#define S_PPNNPL_DQS_TIMING_ANC0_DEFAULT_DELAY_CODE                               0  //Access: read-write
#define M_PPNNPL_DQS_TIMING_ANC0_DEFAULT_DELAY_CODE                               _MM_MAKEMASK(9,S_PPNNPL_DQS_TIMING_ANC0_DEFAULT_DELAY_CODE)
#define V_PPNNPL_DQS_TIMING_ANC0_DEFAULT_DELAY_CODE(V)                            _MM_MAKEVALUE((V),S_PPNNPL_DQS_TIMING_ANC0_DEFAULT_DELAY_CODE)
#define G_PPNNPL_DQS_TIMING_ANC0_DEFAULT_DELAY_CODE(V)                            _MM_GETVALUE((V),S_PPNNPL_DQS_TIMING_ANC0_DEFAULT_DELAY_CODE,M_PPNNPL_DQS_TIMING_ANC0_DEFAULT_DELAY_CODE)
     
#define S_PPNNPL_DQS_TIMING_ANC0_USE_DEFAULT_DELAY_CODE                           9  //Access: read-write
#define M_PPNNPL_DQS_TIMING_ANC0_USE_DEFAULT_DELAY_CODE                           _MM_MAKEMASK(1,S_PPNNPL_DQS_TIMING_ANC0_USE_DEFAULT_DELAY_CODE)
#define V_PPNNPL_DQS_TIMING_ANC0_USE_DEFAULT_DELAY_CODE(V)                        _MM_MAKEVALUE((V),S_PPNNPL_DQS_TIMING_ANC0_USE_DEFAULT_DELAY_CODE)
#define G_PPNNPL_DQS_TIMING_ANC0_USE_DEFAULT_DELAY_CODE(V)                        _MM_GETVALUE((V),S_PPNNPL_DQS_TIMING_ANC0_USE_DEFAULT_DELAY_CODE,M_PPNNPL_DQS_TIMING_ANC0_USE_DEFAULT_DELAY_CODE)
     
#define S_PPNNPL_DQS_TIMING_ANC0_DLL_SELECT                                       10  //Access: read-write
#define M_PPNNPL_DQS_TIMING_ANC0_DLL_SELECT                                       _MM_MAKEMASK(1,S_PPNNPL_DQS_TIMING_ANC0_DLL_SELECT)
#define V_PPNNPL_DQS_TIMING_ANC0_DLL_SELECT(V)                                    _MM_MAKEVALUE((V),S_PPNNPL_DQS_TIMING_ANC0_DLL_SELECT)
#define G_PPNNPL_DQS_TIMING_ANC0_DLL_SELECT(V)                                    _MM_GETVALUE((V),S_PPNNPL_DQS_TIMING_ANC0_DLL_SELECT,M_PPNNPL_DQS_TIMING_ANC0_DLL_SELECT)
     
#define S_PPNNPL_DQS_TIMING_RSVD0                                                 11  //Access: read-as-zero
#define M_PPNNPL_DQS_TIMING_RSVD0                                                 _MM_MAKEMASK(5,S_PPNNPL_DQS_TIMING_RSVD0)
#define V_PPNNPL_DQS_TIMING_RSVD0(V)                                              _MM_MAKEVALUE((V),S_PPNNPL_DQS_TIMING_RSVD0)
#define G_PPNNPL_DQS_TIMING_RSVD0(V)                                              _MM_GETVALUE((V),S_PPNNPL_DQS_TIMING_RSVD0,M_PPNNPL_DQS_TIMING_RSVD0)
     
#define S_PPNNPL_DQS_TIMING_ANC1_DEFAULT_DELAY_CODE                               16  //Access: read-write
#define M_PPNNPL_DQS_TIMING_ANC1_DEFAULT_DELAY_CODE                               _MM_MAKEMASK(9,S_PPNNPL_DQS_TIMING_ANC1_DEFAULT_DELAY_CODE)
#define V_PPNNPL_DQS_TIMING_ANC1_DEFAULT_DELAY_CODE(V)                            _MM_MAKEVALUE((V),S_PPNNPL_DQS_TIMING_ANC1_DEFAULT_DELAY_CODE)
#define G_PPNNPL_DQS_TIMING_ANC1_DEFAULT_DELAY_CODE(V)                            _MM_GETVALUE((V),S_PPNNPL_DQS_TIMING_ANC1_DEFAULT_DELAY_CODE,M_PPNNPL_DQS_TIMING_ANC1_DEFAULT_DELAY_CODE)
     
#define S_PPNNPL_DQS_TIMING_ANC1_USE_DEFAULT_DELAY_CODE                           25  //Access: read-write
#define M_PPNNPL_DQS_TIMING_ANC1_USE_DEFAULT_DELAY_CODE                           _MM_MAKEMASK(1,S_PPNNPL_DQS_TIMING_ANC1_USE_DEFAULT_DELAY_CODE)
#define V_PPNNPL_DQS_TIMING_ANC1_USE_DEFAULT_DELAY_CODE(V)                        _MM_MAKEVALUE((V),S_PPNNPL_DQS_TIMING_ANC1_USE_DEFAULT_DELAY_CODE)
#define G_PPNNPL_DQS_TIMING_ANC1_USE_DEFAULT_DELAY_CODE(V)                        _MM_GETVALUE((V),S_PPNNPL_DQS_TIMING_ANC1_USE_DEFAULT_DELAY_CODE,M_PPNNPL_DQS_TIMING_ANC1_USE_DEFAULT_DELAY_CODE)
     
#define S_PPNNPL_DQS_TIMING_ANC1_DLL_SELECT                                       26  //Access: read-write
#define M_PPNNPL_DQS_TIMING_ANC1_DLL_SELECT                                       _MM_MAKEMASK(1,S_PPNNPL_DQS_TIMING_ANC1_DLL_SELECT)
#define V_PPNNPL_DQS_TIMING_ANC1_DLL_SELECT(V)                                    _MM_MAKEVALUE((V),S_PPNNPL_DQS_TIMING_ANC1_DLL_SELECT)
#define G_PPNNPL_DQS_TIMING_ANC1_DLL_SELECT(V)                                    _MM_GETVALUE((V),S_PPNNPL_DQS_TIMING_ANC1_DLL_SELECT,M_PPNNPL_DQS_TIMING_ANC1_DLL_SELECT)
     
#define S_PPNNPL_DQS_TIMING_RSVD1                                                 27  //Access: read-as-zero
#define M_PPNNPL_DQS_TIMING_RSVD1                                                 _MM_MAKEMASK(5,S_PPNNPL_DQS_TIMING_RSVD1)
#define V_PPNNPL_DQS_TIMING_RSVD1(V)                                              _MM_MAKEVALUE((V),S_PPNNPL_DQS_TIMING_RSVD1)
#define G_PPNNPL_DQS_TIMING_RSVD1(V)                                              _MM_GETVALUE((V),S_PPNNPL_DQS_TIMING_RSVD1,M_PPNNPL_DQS_TIMING_RSVD1)

      
#define S_PPNNPL_DLL_CODES_MASTER_DELAY_OUT                                       0  //Access: read-only
#define M_PPNNPL_DLL_CODES_MASTER_DELAY_OUT                                       _MM_MAKEMASK(7,S_PPNNPL_DLL_CODES_MASTER_DELAY_OUT)
#define V_PPNNPL_DLL_CODES_MASTER_DELAY_OUT(V)                                    _MM_MAKEVALUE((V),S_PPNNPL_DLL_CODES_MASTER_DELAY_OUT)
#define G_PPNNPL_DLL_CODES_MASTER_DELAY_OUT(V)                                    _MM_GETVALUE((V),S_PPNNPL_DLL_CODES_MASTER_DELAY_OUT,M_PPNNPL_DLL_CODES_MASTER_DELAY_OUT)
     
#define S_PPNNPL_DLL_CODES_SLAVE0_DELAY_CODE_IN                                   7  //Access: read-only
#define M_PPNNPL_DLL_CODES_SLAVE0_DELAY_CODE_IN                                   _MM_MAKEMASK(9,S_PPNNPL_DLL_CODES_SLAVE0_DELAY_CODE_IN)
#define V_PPNNPL_DLL_CODES_SLAVE0_DELAY_CODE_IN(V)                                _MM_MAKEVALUE((V),S_PPNNPL_DLL_CODES_SLAVE0_DELAY_CODE_IN)
#define G_PPNNPL_DLL_CODES_SLAVE0_DELAY_CODE_IN(V)                                _MM_GETVALUE((V),S_PPNNPL_DLL_CODES_SLAVE0_DELAY_CODE_IN,M_PPNNPL_DLL_CODES_SLAVE0_DELAY_CODE_IN)
     
#define S_PPNNPL_DLL_CODES_SLAVE1_DELAY_CODE_IN                                   16  //Access: read-only
#define M_PPNNPL_DLL_CODES_SLAVE1_DELAY_CODE_IN                                   _MM_MAKEMASK(9,S_PPNNPL_DLL_CODES_SLAVE1_DELAY_CODE_IN)
#define V_PPNNPL_DLL_CODES_SLAVE1_DELAY_CODE_IN(V)                                _MM_MAKEVALUE((V),S_PPNNPL_DLL_CODES_SLAVE1_DELAY_CODE_IN)
#define G_PPNNPL_DLL_CODES_SLAVE1_DELAY_CODE_IN(V)                                _MM_GETVALUE((V),S_PPNNPL_DLL_CODES_SLAVE1_DELAY_CODE_IN,M_PPNNPL_DLL_CODES_SLAVE1_DELAY_CODE_IN)
     
#define S_PPNNPL_DLL_CODES_RSVD0                                                  25  //Access: read-as-zero
#define M_PPNNPL_DLL_CODES_RSVD0                                                  _MM_MAKEMASK(7,S_PPNNPL_DLL_CODES_RSVD0)
#define V_PPNNPL_DLL_CODES_RSVD0(V)                                               _MM_MAKEVALUE((V),S_PPNNPL_DLL_CODES_RSVD0)
#define G_PPNNPL_DLL_CODES_RSVD0(V)                                               _MM_GETVALUE((V),S_PPNNPL_DLL_CODES_RSVD0,M_PPNNPL_DLL_CODES_RSVD0)

      
#define S_PPNNPL_DQS_ADJUST_ANC0_FREQUENCY_RATIO                                  0  //Access: read-write
#define M_PPNNPL_DQS_ADJUST_ANC0_FREQUENCY_RATIO                                  _MM_MAKEMASK(8,S_PPNNPL_DQS_ADJUST_ANC0_FREQUENCY_RATIO)
#define V_PPNNPL_DQS_ADJUST_ANC0_FREQUENCY_RATIO(V)                               _MM_MAKEVALUE((V),S_PPNNPL_DQS_ADJUST_ANC0_FREQUENCY_RATIO)
#define G_PPNNPL_DQS_ADJUST_ANC0_FREQUENCY_RATIO(V)                               _MM_GETVALUE((V),S_PPNNPL_DQS_ADJUST_ANC0_FREQUENCY_RATIO,M_PPNNPL_DQS_ADJUST_ANC0_FREQUENCY_RATIO)
     
#define S_PPNNPL_DQS_ADJUST_ANC0_OFFSET                                           8  //Access: read-write
#define M_PPNNPL_DQS_ADJUST_ANC0_OFFSET                                           _MM_MAKEMASK(7,S_PPNNPL_DQS_ADJUST_ANC0_OFFSET)
#define V_PPNNPL_DQS_ADJUST_ANC0_OFFSET(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_DQS_ADJUST_ANC0_OFFSET)
#define G_PPNNPL_DQS_ADJUST_ANC0_OFFSET(V)                                        _MM_GETVALUE((V),S_PPNNPL_DQS_ADJUST_ANC0_OFFSET,M_PPNNPL_DQS_ADJUST_ANC0_OFFSET)
     
#define S_PPNNPL_DQS_ADJUST_RSVD0                                                 15  //Access: read-as-zero
#define M_PPNNPL_DQS_ADJUST_RSVD0                                                 _MM_MAKEMASK(1,S_PPNNPL_DQS_ADJUST_RSVD0)
#define V_PPNNPL_DQS_ADJUST_RSVD0(V)                                              _MM_MAKEVALUE((V),S_PPNNPL_DQS_ADJUST_RSVD0)
#define G_PPNNPL_DQS_ADJUST_RSVD0(V)                                              _MM_GETVALUE((V),S_PPNNPL_DQS_ADJUST_RSVD0,M_PPNNPL_DQS_ADJUST_RSVD0)
     
#define S_PPNNPL_DQS_ADJUST_ANC1_FREQUENCY_RATIO                                  16  //Access: read-write
#define M_PPNNPL_DQS_ADJUST_ANC1_FREQUENCY_RATIO                                  _MM_MAKEMASK(8,S_PPNNPL_DQS_ADJUST_ANC1_FREQUENCY_RATIO)
#define V_PPNNPL_DQS_ADJUST_ANC1_FREQUENCY_RATIO(V)                               _MM_MAKEVALUE((V),S_PPNNPL_DQS_ADJUST_ANC1_FREQUENCY_RATIO)
#define G_PPNNPL_DQS_ADJUST_ANC1_FREQUENCY_RATIO(V)                               _MM_GETVALUE((V),S_PPNNPL_DQS_ADJUST_ANC1_FREQUENCY_RATIO,M_PPNNPL_DQS_ADJUST_ANC1_FREQUENCY_RATIO)
     
#define S_PPNNPL_DQS_ADJUST_ANC1_OFFSET                                           24  //Access: read-write
#define M_PPNNPL_DQS_ADJUST_ANC1_OFFSET                                           _MM_MAKEMASK(7,S_PPNNPL_DQS_ADJUST_ANC1_OFFSET)
#define V_PPNNPL_DQS_ADJUST_ANC1_OFFSET(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_DQS_ADJUST_ANC1_OFFSET)
#define G_PPNNPL_DQS_ADJUST_ANC1_OFFSET(V)                                        _MM_GETVALUE((V),S_PPNNPL_DQS_ADJUST_ANC1_OFFSET,M_PPNNPL_DQS_ADJUST_ANC1_OFFSET)
     
#define S_PPNNPL_DQS_ADJUST_RSVD1                                                 31  //Access: read-as-zero
#define M_PPNNPL_DQS_ADJUST_RSVD1                                                 _MM_MAKEMASK(1,S_PPNNPL_DQS_ADJUST_RSVD1)
#define V_PPNNPL_DQS_ADJUST_RSVD1(V)                                              _MM_MAKEVALUE((V),S_PPNNPL_DQS_ADJUST_RSVD1)
#define G_PPNNPL_DQS_ADJUST_RSVD1(V)                                              _MM_GETVALUE((V),S_PPNNPL_DQS_ADJUST_RSVD1,M_PPNNPL_DQS_ADJUST_RSVD1)

      
#define S_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_IN                                     0  //Access: read-write
#define M_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_IN                                     _MM_MAKEMASK(1,S_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_IN)
#define V_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_IN(V)                                  _MM_MAKEVALUE((V),S_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_IN)
#define G_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_IN(V)                                  _MM_GETVALUE((V),S_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_IN,M_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_IN)
     
#define S_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_SEL                                    1  //Access: read-write
#define M_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_SEL                                    _MM_MAKEMASK(1,S_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_SEL)
#define V_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_SEL(V)                                 _MM_MAKEVALUE((V),S_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_SEL)
#define G_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_SEL(V)                                 _MM_GETVALUE((V),S_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_SEL,M_PPNNPL_AMPMCDLL_CONTROL_STEPSIZE_SEL)
     
#define S_PPNNPL_AMPMCDLL_CONTROL_RSVD0                                           2  //Access: read-as-zero
#define M_PPNNPL_AMPMCDLL_CONTROL_RSVD0                                           _MM_MAKEMASK(30,S_PPNNPL_AMPMCDLL_CONTROL_RSVD0)
#define V_PPNNPL_AMPMCDLL_CONTROL_RSVD0(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_AMPMCDLL_CONTROL_RSVD0)
#define G_PPNNPL_AMPMCDLL_CONTROL_RSVD0(V)                                        _MM_GETVALUE((V),S_PPNNPL_AMPMCDLL_CONTROL_RSVD0,M_PPNNPL_AMPMCDLL_CONTROL_RSVD0)

      
#define S_PPNNPL_AMPSLV_CONTROL_ANC0_BYPASS_MODE                                  0  //Access: read-write
#define M_PPNNPL_AMPSLV_CONTROL_ANC0_BYPASS_MODE                                  _MM_MAKEMASK(2,S_PPNNPL_AMPSLV_CONTROL_ANC0_BYPASS_MODE)
#define V_PPNNPL_AMPSLV_CONTROL_ANC0_BYPASS_MODE(V)                               _MM_MAKEVALUE((V),S_PPNNPL_AMPSLV_CONTROL_ANC0_BYPASS_MODE)
#define G_PPNNPL_AMPSLV_CONTROL_ANC0_BYPASS_MODE(V)                               _MM_GETVALUE((V),S_PPNNPL_AMPSLV_CONTROL_ANC0_BYPASS_MODE,M_PPNNPL_AMPSLV_CONTROL_ANC0_BYPASS_MODE)
     
#define S_PPNNPL_AMPSLV_CONTROL_ANC1_BYPASS_MODE                                  2  //Access: read-write
#define M_PPNNPL_AMPSLV_CONTROL_ANC1_BYPASS_MODE                                  _MM_MAKEMASK(2,S_PPNNPL_AMPSLV_CONTROL_ANC1_BYPASS_MODE)
#define V_PPNNPL_AMPSLV_CONTROL_ANC1_BYPASS_MODE(V)                               _MM_MAKEVALUE((V),S_PPNNPL_AMPSLV_CONTROL_ANC1_BYPASS_MODE)
#define G_PPNNPL_AMPSLV_CONTROL_ANC1_BYPASS_MODE(V)                               _MM_GETVALUE((V),S_PPNNPL_AMPSLV_CONTROL_ANC1_BYPASS_MODE,M_PPNNPL_AMPSLV_CONTROL_ANC1_BYPASS_MODE)
     
#define S_PPNNPL_AMPSLV_CONTROL_RSVD0                                             4  //Access: read-as-zero
#define M_PPNNPL_AMPSLV_CONTROL_RSVD0                                             _MM_MAKEMASK(28,S_PPNNPL_AMPSLV_CONTROL_RSVD0)
#define V_PPNNPL_AMPSLV_CONTROL_RSVD0(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_AMPSLV_CONTROL_RSVD0)
#define G_PPNNPL_AMPSLV_CONTROL_RSVD0(V)                                          _MM_GETVALUE((V),S_PPNNPL_AMPSLV_CONTROL_RSVD0,M_PPNNPL_AMPSLV_CONTROL_RSVD0)

      
#define S_PPNNPL_AMPDLL_STATUS_DLL_LOCKED                                         0  //Access: read-only
#define M_PPNNPL_AMPDLL_STATUS_DLL_LOCKED                                         _MM_MAKEMASK(1,S_PPNNPL_AMPDLL_STATUS_DLL_LOCKED)
#define V_PPNNPL_AMPDLL_STATUS_DLL_LOCKED(V)                                      _MM_MAKEVALUE((V),S_PPNNPL_AMPDLL_STATUS_DLL_LOCKED)
#define G_PPNNPL_AMPDLL_STATUS_DLL_LOCKED(V)                                      _MM_GETVALUE((V),S_PPNNPL_AMPDLL_STATUS_DLL_LOCKED,M_PPNNPL_AMPDLL_STATUS_DLL_LOCKED)
     
#define S_PPNNPL_AMPDLL_STATUS_RSVD0                                              1  //Access: read-as-zero
#define M_PPNNPL_AMPDLL_STATUS_RSVD0                                              _MM_MAKEMASK(31,S_PPNNPL_AMPDLL_STATUS_RSVD0)
#define V_PPNNPL_AMPDLL_STATUS_RSVD0(V)                                           _MM_MAKEVALUE((V),S_PPNNPL_AMPDLL_STATUS_RSVD0)
#define G_PPNNPL_AMPDLL_STATUS_RSVD0(V)                                           _MM_GETVALUE((V),S_PPNNPL_AMPDLL_STATUS_RSVD0,M_PPNNPL_AMPDLL_STATUS_RSVD0)

      
#define S_PPNNPL_ANC0_AMPSLV0_SLV_SEL                                             0  //Access: read-only
#define M_PPNNPL_ANC0_AMPSLV0_SLV_SEL                                             _MM_MAKEMASK(12,S_PPNNPL_ANC0_AMPSLV0_SLV_SEL)
#define V_PPNNPL_ANC0_AMPSLV0_SLV_SEL(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_ANC0_AMPSLV0_SLV_SEL)
#define G_PPNNPL_ANC0_AMPSLV0_SLV_SEL(V)                                          _MM_GETVALUE((V),S_PPNNPL_ANC0_AMPSLV0_SLV_SEL,M_PPNNPL_ANC0_AMPSLV0_SLV_SEL)
     
#define S_PPNNPL_ANC0_AMPSLV0_RSVD0                                               12  //Access: read-as-zero
#define M_PPNNPL_ANC0_AMPSLV0_RSVD0                                               _MM_MAKEMASK(20,S_PPNNPL_ANC0_AMPSLV0_RSVD0)
#define V_PPNNPL_ANC0_AMPSLV0_RSVD0(V)                                            _MM_MAKEVALUE((V),S_PPNNPL_ANC0_AMPSLV0_RSVD0)
#define G_PPNNPL_ANC0_AMPSLV0_RSVD0(V)                                            _MM_GETVALUE((V),S_PPNNPL_ANC0_AMPSLV0_RSVD0,M_PPNNPL_ANC0_AMPSLV0_RSVD0)

      
#define S_PPNNPL_ANC0_AMPSLV1_SLV_SEL                                             0  //Access: read-only
#define M_PPNNPL_ANC0_AMPSLV1_SLV_SEL                                             _MM_MAKEMASK(12,S_PPNNPL_ANC0_AMPSLV1_SLV_SEL)
#define V_PPNNPL_ANC0_AMPSLV1_SLV_SEL(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_ANC0_AMPSLV1_SLV_SEL)
#define G_PPNNPL_ANC0_AMPSLV1_SLV_SEL(V)                                          _MM_GETVALUE((V),S_PPNNPL_ANC0_AMPSLV1_SLV_SEL,M_PPNNPL_ANC0_AMPSLV1_SLV_SEL)
     
#define S_PPNNPL_ANC0_AMPSLV1_RSVD0                                               12  //Access: read-as-zero
#define M_PPNNPL_ANC0_AMPSLV1_RSVD0                                               _MM_MAKEMASK(20,S_PPNNPL_ANC0_AMPSLV1_RSVD0)
#define V_PPNNPL_ANC0_AMPSLV1_RSVD0(V)                                            _MM_MAKEVALUE((V),S_PPNNPL_ANC0_AMPSLV1_RSVD0)
#define G_PPNNPL_ANC0_AMPSLV1_RSVD0(V)                                            _MM_GETVALUE((V),S_PPNNPL_ANC0_AMPSLV1_RSVD0,M_PPNNPL_ANC0_AMPSLV1_RSVD0)

      
#define S_PPNNPL_ANC1_AMPSLV0_SLV_SEL                                             0  //Access: read-only
#define M_PPNNPL_ANC1_AMPSLV0_SLV_SEL                                             _MM_MAKEMASK(12,S_PPNNPL_ANC1_AMPSLV0_SLV_SEL)
#define V_PPNNPL_ANC1_AMPSLV0_SLV_SEL(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_ANC1_AMPSLV0_SLV_SEL)
#define G_PPNNPL_ANC1_AMPSLV0_SLV_SEL(V)                                          _MM_GETVALUE((V),S_PPNNPL_ANC1_AMPSLV0_SLV_SEL,M_PPNNPL_ANC1_AMPSLV0_SLV_SEL)
     
#define S_PPNNPL_ANC1_AMPSLV0_RSVD0                                               12  //Access: read-as-zero
#define M_PPNNPL_ANC1_AMPSLV0_RSVD0                                               _MM_MAKEMASK(20,S_PPNNPL_ANC1_AMPSLV0_RSVD0)
#define V_PPNNPL_ANC1_AMPSLV0_RSVD0(V)                                            _MM_MAKEVALUE((V),S_PPNNPL_ANC1_AMPSLV0_RSVD0)
#define G_PPNNPL_ANC1_AMPSLV0_RSVD0(V)                                            _MM_GETVALUE((V),S_PPNNPL_ANC1_AMPSLV0_RSVD0,M_PPNNPL_ANC1_AMPSLV0_RSVD0)

      
#define S_PPNNPL_ANC1_AMPSLV1_SLV_SEL                                             0  //Access: read-only
#define M_PPNNPL_ANC1_AMPSLV1_SLV_SEL                                             _MM_MAKEMASK(12,S_PPNNPL_ANC1_AMPSLV1_SLV_SEL)
#define V_PPNNPL_ANC1_AMPSLV1_SLV_SEL(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_ANC1_AMPSLV1_SLV_SEL)
#define G_PPNNPL_ANC1_AMPSLV1_SLV_SEL(V)                                          _MM_GETVALUE((V),S_PPNNPL_ANC1_AMPSLV1_SLV_SEL,M_PPNNPL_ANC1_AMPSLV1_SLV_SEL)
     
#define S_PPNNPL_ANC1_AMPSLV1_RSVD0                                               12  //Access: read-as-zero
#define M_PPNNPL_ANC1_AMPSLV1_RSVD0                                               _MM_MAKEMASK(20,S_PPNNPL_ANC1_AMPSLV1_RSVD0)
#define V_PPNNPL_ANC1_AMPSLV1_RSVD0(V)                                            _MM_MAKEVALUE((V),S_PPNNPL_ANC1_AMPSLV1_RSVD0)
#define G_PPNNPL_ANC1_AMPSLV1_RSVD0(V)                                            _MM_GETVALUE((V),S_PPNNPL_ANC1_AMPSLV1_RSVD0,M_PPNNPL_ANC1_AMPSLV1_RSVD0)

      
#define S_PPNNPL_TRAIN_CONTROL_DISABLE_CONTINUOUS_TRAINING                        0  //Access: read-write
#define M_PPNNPL_TRAIN_CONTROL_DISABLE_CONTINUOUS_TRAINING                        _MM_MAKEMASK(1,S_PPNNPL_TRAIN_CONTROL_DISABLE_CONTINUOUS_TRAINING)
#define V_PPNNPL_TRAIN_CONTROL_DISABLE_CONTINUOUS_TRAINING(V)                     _MM_MAKEVALUE((V),S_PPNNPL_TRAIN_CONTROL_DISABLE_CONTINUOUS_TRAINING)
#define G_PPNNPL_TRAIN_CONTROL_DISABLE_CONTINUOUS_TRAINING(V)                     _MM_GETVALUE((V),S_PPNNPL_TRAIN_CONTROL_DISABLE_CONTINUOUS_TRAINING,M_PPNNPL_TRAIN_CONTROL_DISABLE_CONTINUOUS_TRAINING)
     
#define S_PPNNPL_TRAIN_CONTROL_TRAIN_PERIODICALLY                                 1  //Access: read-write
#define M_PPNNPL_TRAIN_CONTROL_TRAIN_PERIODICALLY                                 _MM_MAKEMASK(1,S_PPNNPL_TRAIN_CONTROL_TRAIN_PERIODICALLY)
#define V_PPNNPL_TRAIN_CONTROL_TRAIN_PERIODICALLY(V)                              _MM_MAKEVALUE((V),S_PPNNPL_TRAIN_CONTROL_TRAIN_PERIODICALLY)
#define G_PPNNPL_TRAIN_CONTROL_TRAIN_PERIODICALLY(V)                              _MM_GETVALUE((V),S_PPNNPL_TRAIN_CONTROL_TRAIN_PERIODICALLY,M_PPNNPL_TRAIN_CONTROL_TRAIN_PERIODICALLY)
     
#define S_PPNNPL_TRAIN_CONTROL_TRAIN_ONCE                                         2  //Access: write-auto-clear
#define M_PPNNPL_TRAIN_CONTROL_TRAIN_ONCE                                         _MM_MAKEMASK(1,S_PPNNPL_TRAIN_CONTROL_TRAIN_ONCE)
#define V_PPNNPL_TRAIN_CONTROL_TRAIN_ONCE(V)                                      _MM_MAKEVALUE((V),S_PPNNPL_TRAIN_CONTROL_TRAIN_ONCE)
#define G_PPNNPL_TRAIN_CONTROL_TRAIN_ONCE(V)                                      _MM_GETVALUE((V),S_PPNNPL_TRAIN_CONTROL_TRAIN_ONCE,M_PPNNPL_TRAIN_CONTROL_TRAIN_ONCE)
     
#define S_PPNNPL_TRAIN_CONTROL_RSVD0                                              3  //Access: read-as-zero
#define M_PPNNPL_TRAIN_CONTROL_RSVD0                                              _MM_MAKEMASK(29,S_PPNNPL_TRAIN_CONTROL_RSVD0)
#define V_PPNNPL_TRAIN_CONTROL_RSVD0(V)                                           _MM_MAKEVALUE((V),S_PPNNPL_TRAIN_CONTROL_RSVD0)
#define G_PPNNPL_TRAIN_CONTROL_RSVD0(V)                                           _MM_GETVALUE((V),S_PPNNPL_TRAIN_CONTROL_RSVD0,M_PPNNPL_TRAIN_CONTROL_RSVD0)

      
#define S_PPNNPL_PERIODIC_TRAINING_DELAY                                          0  //Access: read-write
#define M_PPNNPL_PERIODIC_TRAINING_DELAY                                          _MM_MAKEMASK(32,S_PPNNPL_PERIODIC_TRAINING_DELAY)
#define V_PPNNPL_PERIODIC_TRAINING_DELAY(V)                                       _MM_MAKEVALUE((V),S_PPNNPL_PERIODIC_TRAINING_DELAY)
#define G_PPNNPL_PERIODIC_TRAINING_DELAY(V)                                       _MM_GETVALUE((V),S_PPNNPL_PERIODIC_TRAINING_DELAY,M_PPNNPL_PERIODIC_TRAINING_DELAY)

      
#define S_PPNNPL_DEBUG_DLL_CONTROL_START_DELAY                                    0  //Access: read-write
#define M_PPNNPL_DEBUG_DLL_CONTROL_START_DELAY                                    _MM_MAKEMASK(3,S_PPNNPL_DEBUG_DLL_CONTROL_START_DELAY)
#define V_PPNNPL_DEBUG_DLL_CONTROL_START_DELAY(V)                                 _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_START_DELAY)
#define G_PPNNPL_DEBUG_DLL_CONTROL_START_DELAY(V)                                 _MM_GETVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_START_DELAY,M_PPNNPL_DEBUG_DLL_CONTROL_START_DELAY)
     
#define S_PPNNPL_DEBUG_DLL_CONTROL_RSVD0                                          3  //Access: read-as-zero
#define M_PPNNPL_DEBUG_DLL_CONTROL_RSVD0                                          _MM_MAKEMASK(1,S_PPNNPL_DEBUG_DLL_CONTROL_RSVD0)
#define V_PPNNPL_DEBUG_DLL_CONTROL_RSVD0(V)                                       _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_RSVD0)
#define G_PPNNPL_DEBUG_DLL_CONTROL_RSVD0(V)                                       _MM_GETVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_RSVD0,M_PPNNPL_DEBUG_DLL_CONTROL_RSVD0)
     
#define S_PPNNPL_DEBUG_DLL_CONTROL_STEP_SIZE                                      4  //Access: read-write
#define M_PPNNPL_DEBUG_DLL_CONTROL_STEP_SIZE                                      _MM_MAKEMASK(3,S_PPNNPL_DEBUG_DLL_CONTROL_STEP_SIZE)
#define V_PPNNPL_DEBUG_DLL_CONTROL_STEP_SIZE(V)                                   _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_STEP_SIZE)
#define G_PPNNPL_DEBUG_DLL_CONTROL_STEP_SIZE(V)                                   _MM_GETVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_STEP_SIZE,M_PPNNPL_DEBUG_DLL_CONTROL_STEP_SIZE)
     
#define S_PPNNPL_DEBUG_DLL_CONTROL_RSVD1                                          7  //Access: read-as-zero
#define M_PPNNPL_DEBUG_DLL_CONTROL_RSVD1                                          _MM_MAKEMASK(9,S_PPNNPL_DEBUG_DLL_CONTROL_RSVD1)
#define V_PPNNPL_DEBUG_DLL_CONTROL_RSVD1(V)                                       _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_RSVD1)
#define G_PPNNPL_DEBUG_DLL_CONTROL_RSVD1(V)                                       _MM_GETVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_RSVD1,M_PPNNPL_DEBUG_DLL_CONTROL_RSVD1)
     
#define S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN                                          16  //Access: read-write
#define M_PPNNPL_DEBUG_DLL_CONTROL_TRAIN                                          _MM_MAKEMASK(1,S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN)
#define V_PPNNPL_DEBUG_DLL_CONTROL_TRAIN(V)                                       _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN)
#define G_PPNNPL_DEBUG_DLL_CONTROL_TRAIN(V)                                       _MM_GETVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN,M_PPNNPL_DEBUG_DLL_CONTROL_TRAIN)
     
#define S_PPNNPL_DEBUG_DLL_CONTROL_RSVD2                                          17  //Access: read-as-zero
#define M_PPNNPL_DEBUG_DLL_CONTROL_RSVD2                                          _MM_MAKEMASK(7,S_PPNNPL_DEBUG_DLL_CONTROL_RSVD2)
#define V_PPNNPL_DEBUG_DLL_CONTROL_RSVD2(V)                                       _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_RSVD2)
#define G_PPNNPL_DEBUG_DLL_CONTROL_RSVD2(V)                                       _MM_GETVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_RSVD2,M_PPNNPL_DEBUG_DLL_CONTROL_RSVD2)
     
#define S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PREPARE_DELAY                            24  //Access: read-write
#define M_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PREPARE_DELAY                            _MM_MAKEMASK(4,S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PREPARE_DELAY)
#define V_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PREPARE_DELAY(V)                         _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PREPARE_DELAY)
#define G_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PREPARE_DELAY(V)                         _MM_GETVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PREPARE_DELAY,M_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PREPARE_DELAY)
     
#define S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PAUSE_DELAY                              28  //Access: read-write
#define M_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PAUSE_DELAY                              _MM_MAKEMASK(4,S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PAUSE_DELAY)
#define V_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PAUSE_DELAY(V)                           _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PAUSE_DELAY)
#define G_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PAUSE_DELAY(V)                           _MM_GETVALUE((V),S_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PAUSE_DELAY,M_PPNNPL_DEBUG_DLL_CONTROL_TRAIN_PAUSE_DELAY)

      
#define S_PPNNPL_DEBUG_DATA_CAPTURE_ANC0_POSEDGE_DATA                             0  //Access: read-only
#define M_PPNNPL_DEBUG_DATA_CAPTURE_ANC0_POSEDGE_DATA                             _MM_MAKEMASK(8,S_PPNNPL_DEBUG_DATA_CAPTURE_ANC0_POSEDGE_DATA)
#define V_PPNNPL_DEBUG_DATA_CAPTURE_ANC0_POSEDGE_DATA(V)                          _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DATA_CAPTURE_ANC0_POSEDGE_DATA)
#define G_PPNNPL_DEBUG_DATA_CAPTURE_ANC0_POSEDGE_DATA(V)                          _MM_GETVALUE((V),S_PPNNPL_DEBUG_DATA_CAPTURE_ANC0_POSEDGE_DATA,M_PPNNPL_DEBUG_DATA_CAPTURE_ANC0_POSEDGE_DATA)
     
#define S_PPNNPL_DEBUG_DATA_CAPTURE_ANC1_POSEDGE_DATA                             8  //Access: read-only
#define M_PPNNPL_DEBUG_DATA_CAPTURE_ANC1_POSEDGE_DATA                             _MM_MAKEMASK(8,S_PPNNPL_DEBUG_DATA_CAPTURE_ANC1_POSEDGE_DATA)
#define V_PPNNPL_DEBUG_DATA_CAPTURE_ANC1_POSEDGE_DATA(V)                          _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DATA_CAPTURE_ANC1_POSEDGE_DATA)
#define G_PPNNPL_DEBUG_DATA_CAPTURE_ANC1_POSEDGE_DATA(V)                          _MM_GETVALUE((V),S_PPNNPL_DEBUG_DATA_CAPTURE_ANC1_POSEDGE_DATA,M_PPNNPL_DEBUG_DATA_CAPTURE_ANC1_POSEDGE_DATA)
     
#define S_PPNNPL_DEBUG_DATA_CAPTURE_RSVD0                                         16  //Access: read-as-zero
#define M_PPNNPL_DEBUG_DATA_CAPTURE_RSVD0                                         _MM_MAKEMASK(16,S_PPNNPL_DEBUG_DATA_CAPTURE_RSVD0)
#define V_PPNNPL_DEBUG_DATA_CAPTURE_RSVD0(V)                                      _MM_MAKEVALUE((V),S_PPNNPL_DEBUG_DATA_CAPTURE_RSVD0)
#define G_PPNNPL_DEBUG_DATA_CAPTURE_RSVD0(V)                                      _MM_GETVALUE((V),S_PPNNPL_DEBUG_DATA_CAPTURE_RSVD0,M_PPNNPL_DEBUG_DATA_CAPTURE_RSVD0)

      
#define S_PPNNPL_PPN0_CLE_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN0_CLE_INPUT_ENABLE                                            _MM_MAKEMASK(1,S_PPNNPL_PPN0_CLE_INPUT_ENABLE)
#define V_PPNNPL_PPN0_CLE_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN0_CLE_INPUT_ENABLE)
#define G_PPNNPL_PPN0_CLE_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN0_CLE_INPUT_ENABLE,M_PPNNPL_PPN0_CLE_INPUT_ENABLE)
     
#define S_PPNNPL_PPN0_CLE_OUTPUT_ENABLE                                           1  //Access: read-write
#define M_PPNNPL_PPN0_CLE_OUTPUT_ENABLE                                           _MM_MAKEMASK(1,S_PPNNPL_PPN0_CLE_OUTPUT_ENABLE)
#define V_PPNNPL_PPN0_CLE_OUTPUT_ENABLE(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN0_CLE_OUTPUT_ENABLE)
#define G_PPNNPL_PPN0_CLE_OUTPUT_ENABLE(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN0_CLE_OUTPUT_ENABLE,M_PPNNPL_PPN0_CLE_OUTPUT_ENABLE)
     
#define S_PPNNPL_PPN0_CLE_PULL_ENABLE                                             2  //Access: read-write
#define M_PPNNPL_PPN0_CLE_PULL_ENABLE                                             _MM_MAKEMASK(1,S_PPNNPL_PPN0_CLE_PULL_ENABLE)
#define V_PPNNPL_PPN0_CLE_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_CLE_PULL_ENABLE)
#define G_PPNNPL_PPN0_CLE_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_CLE_PULL_ENABLE,M_PPNNPL_PPN0_CLE_PULL_ENABLE)
     
#define S_PPNNPL_PPN0_CLE_PULL_SELECT                                             3  //Access: read-write
#define M_PPNNPL_PPN0_CLE_PULL_SELECT                                             _MM_MAKEMASK(1,S_PPNNPL_PPN0_CLE_PULL_SELECT)
#define V_PPNNPL_PPN0_CLE_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_CLE_PULL_SELECT)
#define G_PPNNPL_PPN0_CLE_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_CLE_PULL_SELECT,M_PPNNPL_PPN0_CLE_PULL_SELECT)
     
#define S_PPNNPL_PPN0_CLE_RSVD0                                                   4  //Access: read-as-zero
#define M_PPNNPL_PPN0_CLE_RSVD0                                                   _MM_MAKEMASK(28,S_PPNNPL_PPN0_CLE_RSVD0)
#define V_PPNNPL_PPN0_CLE_RSVD0(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_CLE_RSVD0)
#define G_PPNNPL_PPN0_CLE_RSVD0(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN0_CLE_RSVD0,M_PPNNPL_PPN0_CLE_RSVD0)

      
#define S_PPNNPL_PPN0_ALE_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN0_ALE_INPUT_ENABLE                                            _MM_MAKEMASK(1,S_PPNNPL_PPN0_ALE_INPUT_ENABLE)
#define V_PPNNPL_PPN0_ALE_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ALE_INPUT_ENABLE)
#define G_PPNNPL_PPN0_ALE_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN0_ALE_INPUT_ENABLE,M_PPNNPL_PPN0_ALE_INPUT_ENABLE)
     
#define S_PPNNPL_PPN0_ALE_OUTPUT_ENABLE                                           1  //Access: read-write
#define M_PPNNPL_PPN0_ALE_OUTPUT_ENABLE                                           _MM_MAKEMASK(1,S_PPNNPL_PPN0_ALE_OUTPUT_ENABLE)
#define V_PPNNPL_PPN0_ALE_OUTPUT_ENABLE(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ALE_OUTPUT_ENABLE)
#define G_PPNNPL_PPN0_ALE_OUTPUT_ENABLE(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN0_ALE_OUTPUT_ENABLE,M_PPNNPL_PPN0_ALE_OUTPUT_ENABLE)
     
#define S_PPNNPL_PPN0_ALE_PULL_ENABLE                                             2  //Access: read-write
#define M_PPNNPL_PPN0_ALE_PULL_ENABLE                                             _MM_MAKEMASK(1,S_PPNNPL_PPN0_ALE_PULL_ENABLE)
#define V_PPNNPL_PPN0_ALE_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ALE_PULL_ENABLE)
#define G_PPNNPL_PPN0_ALE_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_ALE_PULL_ENABLE,M_PPNNPL_PPN0_ALE_PULL_ENABLE)
     
#define S_PPNNPL_PPN0_ALE_PULL_SELECT                                             3  //Access: read-write
#define M_PPNNPL_PPN0_ALE_PULL_SELECT                                             _MM_MAKEMASK(1,S_PPNNPL_PPN0_ALE_PULL_SELECT)
#define V_PPNNPL_PPN0_ALE_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ALE_PULL_SELECT)
#define G_PPNNPL_PPN0_ALE_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_ALE_PULL_SELECT,M_PPNNPL_PPN0_ALE_PULL_SELECT)
     
#define S_PPNNPL_PPN0_ALE_RSVD0                                                   4  //Access: read-as-zero
#define M_PPNNPL_PPN0_ALE_RSVD0                                                   _MM_MAKEMASK(28,S_PPNNPL_PPN0_ALE_RSVD0)
#define V_PPNNPL_PPN0_ALE_RSVD0(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ALE_RSVD0)
#define G_PPNNPL_PPN0_ALE_RSVD0(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN0_ALE_RSVD0,M_PPNNPL_PPN0_ALE_RSVD0)

      
#define S_PPNNPL_PPN0_REN_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN0_REN_INPUT_ENABLE                                            _MM_MAKEMASK(1,S_PPNNPL_PPN0_REN_INPUT_ENABLE)
#define V_PPNNPL_PPN0_REN_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN0_REN_INPUT_ENABLE)
#define G_PPNNPL_PPN0_REN_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN0_REN_INPUT_ENABLE,M_PPNNPL_PPN0_REN_INPUT_ENABLE)
     
#define S_PPNNPL_PPN0_REN_OUTPUT_ENABLE                                           1  //Access: read-write
#define M_PPNNPL_PPN0_REN_OUTPUT_ENABLE                                           _MM_MAKEMASK(1,S_PPNNPL_PPN0_REN_OUTPUT_ENABLE)
#define V_PPNNPL_PPN0_REN_OUTPUT_ENABLE(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN0_REN_OUTPUT_ENABLE)
#define G_PPNNPL_PPN0_REN_OUTPUT_ENABLE(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN0_REN_OUTPUT_ENABLE,M_PPNNPL_PPN0_REN_OUTPUT_ENABLE)
     
#define S_PPNNPL_PPN0_REN_PULL_ENABLE                                             2  //Access: read-write
#define M_PPNNPL_PPN0_REN_PULL_ENABLE                                             _MM_MAKEMASK(1,S_PPNNPL_PPN0_REN_PULL_ENABLE)
#define V_PPNNPL_PPN0_REN_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_REN_PULL_ENABLE)
#define G_PPNNPL_PPN0_REN_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_REN_PULL_ENABLE,M_PPNNPL_PPN0_REN_PULL_ENABLE)
     
#define S_PPNNPL_PPN0_REN_PULL_SELECT                                             3  //Access: read-write
#define M_PPNNPL_PPN0_REN_PULL_SELECT                                             _MM_MAKEMASK(1,S_PPNNPL_PPN0_REN_PULL_SELECT)
#define V_PPNNPL_PPN0_REN_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_REN_PULL_SELECT)
#define G_PPNNPL_PPN0_REN_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_REN_PULL_SELECT,M_PPNNPL_PPN0_REN_PULL_SELECT)
     
#define S_PPNNPL_PPN0_REN_RSVD0                                                   4  //Access: read-as-zero
#define M_PPNNPL_PPN0_REN_RSVD0                                                   _MM_MAKEMASK(28,S_PPNNPL_PPN0_REN_RSVD0)
#define V_PPNNPL_PPN0_REN_RSVD0(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_REN_RSVD0)
#define G_PPNNPL_PPN0_REN_RSVD0(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN0_REN_RSVD0,M_PPNNPL_PPN0_REN_RSVD0)

      
#define S_PPNNPL_PPN0_WEN_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN0_WEN_INPUT_ENABLE                                            _MM_MAKEMASK(1,S_PPNNPL_PPN0_WEN_INPUT_ENABLE)
#define V_PPNNPL_PPN0_WEN_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN0_WEN_INPUT_ENABLE)
#define G_PPNNPL_PPN0_WEN_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN0_WEN_INPUT_ENABLE,M_PPNNPL_PPN0_WEN_INPUT_ENABLE)
     
#define S_PPNNPL_PPN0_WEN_OUTPUT_ENABLE                                           1  //Access: read-write
#define M_PPNNPL_PPN0_WEN_OUTPUT_ENABLE                                           _MM_MAKEMASK(1,S_PPNNPL_PPN0_WEN_OUTPUT_ENABLE)
#define V_PPNNPL_PPN0_WEN_OUTPUT_ENABLE(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN0_WEN_OUTPUT_ENABLE)
#define G_PPNNPL_PPN0_WEN_OUTPUT_ENABLE(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN0_WEN_OUTPUT_ENABLE,M_PPNNPL_PPN0_WEN_OUTPUT_ENABLE)
     
#define S_PPNNPL_PPN0_WEN_PULL_ENABLE                                             2  //Access: read-write
#define M_PPNNPL_PPN0_WEN_PULL_ENABLE                                             _MM_MAKEMASK(1,S_PPNNPL_PPN0_WEN_PULL_ENABLE)
#define V_PPNNPL_PPN0_WEN_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_WEN_PULL_ENABLE)
#define G_PPNNPL_PPN0_WEN_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_WEN_PULL_ENABLE,M_PPNNPL_PPN0_WEN_PULL_ENABLE)
     
#define S_PPNNPL_PPN0_WEN_PULL_SELECT                                             3  //Access: read-write
#define M_PPNNPL_PPN0_WEN_PULL_SELECT                                             _MM_MAKEMASK(1,S_PPNNPL_PPN0_WEN_PULL_SELECT)
#define V_PPNNPL_PPN0_WEN_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_WEN_PULL_SELECT)
#define G_PPNNPL_PPN0_WEN_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_WEN_PULL_SELECT,M_PPNNPL_PPN0_WEN_PULL_SELECT)
     
#define S_PPNNPL_PPN0_WEN_RSVD0                                                   4  //Access: read-as-zero
#define M_PPNNPL_PPN0_WEN_RSVD0                                                   _MM_MAKEMASK(28,S_PPNNPL_PPN0_WEN_RSVD0)
#define V_PPNNPL_PPN0_WEN_RSVD0(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_WEN_RSVD0)
#define G_PPNNPL_PPN0_WEN_RSVD0(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN0_WEN_RSVD0,M_PPNNPL_PPN0_WEN_RSVD0)

      
#define S_PPNNPL_PPN0_CEN_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN0_CEN_INPUT_ENABLE                                            _MM_MAKEMASK(8,S_PPNNPL_PPN0_CEN_INPUT_ENABLE)
#define V_PPNNPL_PPN0_CEN_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN0_CEN_INPUT_ENABLE)
#define G_PPNNPL_PPN0_CEN_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN0_CEN_INPUT_ENABLE,M_PPNNPL_PPN0_CEN_INPUT_ENABLE)
     
#define S_PPNNPL_PPN0_CEN_OUTPUT_ENABLE                                           8  //Access: read-write
#define M_PPNNPL_PPN0_CEN_OUTPUT_ENABLE                                           _MM_MAKEMASK(8,S_PPNNPL_PPN0_CEN_OUTPUT_ENABLE)
#define V_PPNNPL_PPN0_CEN_OUTPUT_ENABLE(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN0_CEN_OUTPUT_ENABLE)
#define G_PPNNPL_PPN0_CEN_OUTPUT_ENABLE(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN0_CEN_OUTPUT_ENABLE,M_PPNNPL_PPN0_CEN_OUTPUT_ENABLE)
     
#define S_PPNNPL_PPN0_CEN_PULL_ENABLE                                             16  //Access: read-write
#define M_PPNNPL_PPN0_CEN_PULL_ENABLE                                             _MM_MAKEMASK(8,S_PPNNPL_PPN0_CEN_PULL_ENABLE)
#define V_PPNNPL_PPN0_CEN_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_CEN_PULL_ENABLE)
#define G_PPNNPL_PPN0_CEN_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_CEN_PULL_ENABLE,M_PPNNPL_PPN0_CEN_PULL_ENABLE)
     
#define S_PPNNPL_PPN0_CEN_PULL_SELECT                                             24  //Access: read-write
#define M_PPNNPL_PPN0_CEN_PULL_SELECT                                             _MM_MAKEMASK(8,S_PPNNPL_PPN0_CEN_PULL_SELECT)
#define V_PPNNPL_PPN0_CEN_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_CEN_PULL_SELECT)
#define G_PPNNPL_PPN0_CEN_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_CEN_PULL_SELECT,M_PPNNPL_PPN0_CEN_PULL_SELECT)

      
#define S_PPNNPL_PPN0_DQS_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN0_DQS_INPUT_ENABLE                                            _MM_MAKEMASK(1,S_PPNNPL_PPN0_DQS_INPUT_ENABLE)
#define V_PPNNPL_PPN0_DQS_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN0_DQS_INPUT_ENABLE)
#define G_PPNNPL_PPN0_DQS_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN0_DQS_INPUT_ENABLE,M_PPNNPL_PPN0_DQS_INPUT_ENABLE)
     
#define S_PPNNPL_PPN0_DQS_RSVD0                                                   1  //Access: read-as-zero
#define M_PPNNPL_PPN0_DQS_RSVD0                                                   _MM_MAKEMASK(1,S_PPNNPL_PPN0_DQS_RSVD0)
#define V_PPNNPL_PPN0_DQS_RSVD0(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_DQS_RSVD0)
#define G_PPNNPL_PPN0_DQS_RSVD0(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN0_DQS_RSVD0,M_PPNNPL_PPN0_DQS_RSVD0)
     
#define S_PPNNPL_PPN0_DQS_PULL_ENABLE                                             2  //Access: read-write
#define M_PPNNPL_PPN0_DQS_PULL_ENABLE                                             _MM_MAKEMASK(1,S_PPNNPL_PPN0_DQS_PULL_ENABLE)
#define V_PPNNPL_PPN0_DQS_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_DQS_PULL_ENABLE)
#define G_PPNNPL_PPN0_DQS_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_DQS_PULL_ENABLE,M_PPNNPL_PPN0_DQS_PULL_ENABLE)
     
#define S_PPNNPL_PPN0_DQS_PULL_SELECT                                             3  //Access: read-write
#define M_PPNNPL_PPN0_DQS_PULL_SELECT                                             _MM_MAKEMASK(1,S_PPNNPL_PPN0_DQS_PULL_SELECT)
#define V_PPNNPL_PPN0_DQS_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_DQS_PULL_SELECT)
#define G_PPNNPL_PPN0_DQS_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_DQS_PULL_SELECT,M_PPNNPL_PPN0_DQS_PULL_SELECT)
     
#define S_PPNNPL_PPN0_DQS_RSVD1                                                   4  //Access: read-as-zero
#define M_PPNNPL_PPN0_DQS_RSVD1                                                   _MM_MAKEMASK(28,S_PPNNPL_PPN0_DQS_RSVD1)
#define V_PPNNPL_PPN0_DQS_RSVD1(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_DQS_RSVD1)
#define G_PPNNPL_PPN0_DQS_RSVD1(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN0_DQS_RSVD1,M_PPNNPL_PPN0_DQS_RSVD1)

      
#define S_PPNNPL_PPN0_IO_INPUT_ENABLE                                             0  //Access: read-write
#define M_PPNNPL_PPN0_IO_INPUT_ENABLE                                             _MM_MAKEMASK(8,S_PPNNPL_PPN0_IO_INPUT_ENABLE)
#define V_PPNNPL_PPN0_IO_INPUT_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN0_IO_INPUT_ENABLE)
#define G_PPNNPL_PPN0_IO_INPUT_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN0_IO_INPUT_ENABLE,M_PPNNPL_PPN0_IO_INPUT_ENABLE)
     
#define S_PPNNPL_PPN0_IO_RSVD0                                                    8  //Access: read-as-zero
#define M_PPNNPL_PPN0_IO_RSVD0                                                    _MM_MAKEMASK(8,S_PPNNPL_PPN0_IO_RSVD0)
#define V_PPNNPL_PPN0_IO_RSVD0(V)                                                 _MM_MAKEVALUE((V),S_PPNNPL_PPN0_IO_RSVD0)
#define G_PPNNPL_PPN0_IO_RSVD0(V)                                                 _MM_GETVALUE((V),S_PPNNPL_PPN0_IO_RSVD0,M_PPNNPL_PPN0_IO_RSVD0)
     
#define S_PPNNPL_PPN0_IO_PULL_ENABLE                                              16  //Access: read-write
#define M_PPNNPL_PPN0_IO_PULL_ENABLE                                              _MM_MAKEMASK(8,S_PPNNPL_PPN0_IO_PULL_ENABLE)
#define V_PPNNPL_PPN0_IO_PULL_ENABLE(V)                                           _MM_MAKEVALUE((V),S_PPNNPL_PPN0_IO_PULL_ENABLE)
#define G_PPNNPL_PPN0_IO_PULL_ENABLE(V)                                           _MM_GETVALUE((V),S_PPNNPL_PPN0_IO_PULL_ENABLE,M_PPNNPL_PPN0_IO_PULL_ENABLE)
     
#define S_PPNNPL_PPN0_IO_PULL_SELECT                                              24  //Access: read-write
#define M_PPNNPL_PPN0_IO_PULL_SELECT                                              _MM_MAKEMASK(8,S_PPNNPL_PPN0_IO_PULL_SELECT)
#define V_PPNNPL_PPN0_IO_PULL_SELECT(V)                                           _MM_MAKEVALUE((V),S_PPNNPL_PPN0_IO_PULL_SELECT)
#define G_PPNNPL_PPN0_IO_PULL_SELECT(V)                                           _MM_GETVALUE((V),S_PPNNPL_PPN0_IO_PULL_SELECT,M_PPNNPL_PPN0_IO_PULL_SELECT)

      
#define S_PPNNPL_PPN0_ZQ_OEPD                                                     0  //Access: read-write
#define M_PPNNPL_PPN0_ZQ_OEPD                                                     _MM_MAKEMASK(1,S_PPNNPL_PPN0_ZQ_OEPD)
#define V_PPNNPL_PPN0_ZQ_OEPD(V)                                                  _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZQ_OEPD)
#define G_PPNNPL_PPN0_ZQ_OEPD(V)                                                  _MM_GETVALUE((V),S_PPNNPL_PPN0_ZQ_OEPD,M_PPNNPL_PPN0_ZQ_OEPD)
     
#define S_PPNNPL_PPN0_ZQ_OEPU                                                     1  //Access: read-write
#define M_PPNNPL_PPN0_ZQ_OEPU                                                     _MM_MAKEMASK(1,S_PPNNPL_PPN0_ZQ_OEPU)
#define V_PPNNPL_PPN0_ZQ_OEPU(V)                                                  _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZQ_OEPU)
#define G_PPNNPL_PPN0_ZQ_OEPU(V)                                                  _MM_GETVALUE((V),S_PPNNPL_PPN0_ZQ_OEPU,M_PPNNPL_PPN0_ZQ_OEPU)
     
#define S_PPNNPL_PPN0_ZQ_ZCPD                                                     2  //Access: read-write
#define M_PPNNPL_PPN0_ZQ_ZCPD                                                     _MM_MAKEMASK(4,S_PPNNPL_PPN0_ZQ_ZCPD)
#define V_PPNNPL_PPN0_ZQ_ZCPD(V)                                                  _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZQ_ZCPD)
#define G_PPNNPL_PPN0_ZQ_ZCPD(V)                                                  _MM_GETVALUE((V),S_PPNNPL_PPN0_ZQ_ZCPD,M_PPNNPL_PPN0_ZQ_ZCPD)
     
#define S_PPNNPL_PPN0_ZQ_ZCPU                                                     6  //Access: read-write
#define M_PPNNPL_PPN0_ZQ_ZCPU                                                     _MM_MAKEMASK(4,S_PPNNPL_PPN0_ZQ_ZCPU)
#define V_PPNNPL_PPN0_ZQ_ZCPU(V)                                                  _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZQ_ZCPU)
#define G_PPNNPL_PPN0_ZQ_ZCPU(V)                                                  _MM_GETVALUE((V),S_PPNNPL_PPN0_ZQ_ZCPU,M_PPNNPL_PPN0_ZQ_ZCPU)
     
#define S_PPNNPL_PPN0_ZQ_RSVD0                                                    10  //Access: read-as-zero
#define M_PPNNPL_PPN0_ZQ_RSVD0                                                    _MM_MAKEMASK(22,S_PPNNPL_PPN0_ZQ_RSVD0)
#define V_PPNNPL_PPN0_ZQ_RSVD0(V)                                                 _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZQ_RSVD0)
#define G_PPNNPL_PPN0_ZQ_RSVD0(V)                                                 _MM_GETVALUE((V),S_PPNNPL_PPN0_ZQ_RSVD0,M_PPNNPL_PPN0_ZQ_RSVD0)

      
#define S_PPNNPL_PPN0_ZQ_IN_YPD                                                   0  //Access: read-only
#define M_PPNNPL_PPN0_ZQ_IN_YPD                                                   _MM_MAKEMASK(1,S_PPNNPL_PPN0_ZQ_IN_YPD)
#define V_PPNNPL_PPN0_ZQ_IN_YPD(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZQ_IN_YPD)
#define G_PPNNPL_PPN0_ZQ_IN_YPD(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN0_ZQ_IN_YPD,M_PPNNPL_PPN0_ZQ_IN_YPD)
     
#define S_PPNNPL_PPN0_ZQ_IN_YPU                                                   1  //Access: read-only
#define M_PPNNPL_PPN0_ZQ_IN_YPU                                                   _MM_MAKEMASK(1,S_PPNNPL_PPN0_ZQ_IN_YPU)
#define V_PPNNPL_PPN0_ZQ_IN_YPU(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZQ_IN_YPU)
#define G_PPNNPL_PPN0_ZQ_IN_YPU(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN0_ZQ_IN_YPU,M_PPNNPL_PPN0_ZQ_IN_YPU)
     
#define S_PPNNPL_PPN0_ZQ_IN_RSVD0                                                 2  //Access: read-as-zero
#define M_PPNNPL_PPN0_ZQ_IN_RSVD0                                                 _MM_MAKEMASK(30,S_PPNNPL_PPN0_ZQ_IN_RSVD0)
#define V_PPNNPL_PPN0_ZQ_IN_RSVD0(V)                                              _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZQ_IN_RSVD0)
#define G_PPNNPL_PPN0_ZQ_IN_RSVD0(V)                                              _MM_GETVALUE((V),S_PPNNPL_PPN0_ZQ_IN_RSVD0,M_PPNNPL_PPN0_ZQ_IN_RSVD0)

      
#define S_PPNNPL_PPN0_ZCPD_ZCPD                                                   0  //Access: read-write
#define M_PPNNPL_PPN0_ZCPD_ZCPD                                                   _MM_MAKEMASK(4,S_PPNNPL_PPN0_ZCPD_ZCPD)
#define V_PPNNPL_PPN0_ZCPD_ZCPD(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZCPD_ZCPD)
#define G_PPNNPL_PPN0_ZCPD_ZCPD(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN0_ZCPD_ZCPD,M_PPNNPL_PPN0_ZCPD_ZCPD)
     
#define S_PPNNPL_PPN0_ZCPD_RSVD0                                                  4  //Access: read-as-zero
#define M_PPNNPL_PPN0_ZCPD_RSVD0                                                  _MM_MAKEMASK(28,S_PPNNPL_PPN0_ZCPD_RSVD0)
#define V_PPNNPL_PPN0_ZCPD_RSVD0(V)                                               _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZCPD_RSVD0)
#define G_PPNNPL_PPN0_ZCPD_RSVD0(V)                                               _MM_GETVALUE((V),S_PPNNPL_PPN0_ZCPD_RSVD0,M_PPNNPL_PPN0_ZCPD_RSVD0)

      
#define S_PPNNPL_PPN0_ZCPU_ZCPU                                                   0  //Access: read-write
#define M_PPNNPL_PPN0_ZCPU_ZCPU                                                   _MM_MAKEMASK(4,S_PPNNPL_PPN0_ZCPU_ZCPU)
#define V_PPNNPL_PPN0_ZCPU_ZCPU(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZCPU_ZCPU)
#define G_PPNNPL_PPN0_ZCPU_ZCPU(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN0_ZCPU_ZCPU,M_PPNNPL_PPN0_ZCPU_ZCPU)
     
#define S_PPNNPL_PPN0_ZCPU_RSVD0                                                  4  //Access: read-as-zero
#define M_PPNNPL_PPN0_ZCPU_RSVD0                                                  _MM_MAKEMASK(28,S_PPNNPL_PPN0_ZCPU_RSVD0)
#define V_PPNNPL_PPN0_ZCPU_RSVD0(V)                                               _MM_MAKEVALUE((V),S_PPNNPL_PPN0_ZCPU_RSVD0)
#define G_PPNNPL_PPN0_ZCPU_RSVD0(V)                                               _MM_GETVALUE((V),S_PPNNPL_PPN0_ZCPU_RSVD0,M_PPNNPL_PPN0_ZCPU_RSVD0)

      
#define S_PPNNPL_PPN0_DS_DRIVE_STRENGTH                                           0  //Access: read-write
#define M_PPNNPL_PPN0_DS_DRIVE_STRENGTH                                           _MM_MAKEMASK(4,S_PPNNPL_PPN0_DS_DRIVE_STRENGTH)
#define V_PPNNPL_PPN0_DS_DRIVE_STRENGTH(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN0_DS_DRIVE_STRENGTH)
#define G_PPNNPL_PPN0_DS_DRIVE_STRENGTH(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN0_DS_DRIVE_STRENGTH,M_PPNNPL_PPN0_DS_DRIVE_STRENGTH)
     
#define S_PPNNPL_PPN0_DS_RSVD0                                                    4  //Access: read-as-zero
#define M_PPNNPL_PPN0_DS_RSVD0                                                    _MM_MAKEMASK(28,S_PPNNPL_PPN0_DS_RSVD0)
#define V_PPNNPL_PPN0_DS_RSVD0(V)                                                 _MM_MAKEVALUE((V),S_PPNNPL_PPN0_DS_RSVD0)
#define G_PPNNPL_PPN0_DS_RSVD0(V)                                                 _MM_GETVALUE((V),S_PPNNPL_PPN0_DS_RSVD0,M_PPNNPL_PPN0_DS_RSVD0)

      
#define S_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT                                   0  //Access: read-write
#define M_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT                                   _MM_MAKEMASK(1,S_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT)
#define V_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT(V)                                _MM_MAKEVALUE((V),S_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT)
#define G_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT(V)                                _MM_GETVALUE((V),S_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT,M_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT)
     
#define S_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT_SCHMITT                           1  //Access: read-write
#define M_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT_SCHMITT                           _MM_MAKEMASK(1,S_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT_SCHMITT)
#define V_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT_SCHMITT(V)                        _MM_MAKEVALUE((V),S_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT_SCHMITT)
#define G_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT_SCHMITT(V)                        _MM_GETVALUE((V),S_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT_SCHMITT,M_PPNNPL_PPN0_INPUT_SELECT_INPUT_SELECT_SCHMITT)
     
#define S_PPNNPL_PPN0_INPUT_SELECT_RSVD0                                          2  //Access: read-as-zero
#define M_PPNNPL_PPN0_INPUT_SELECT_RSVD0                                          _MM_MAKEMASK(30,S_PPNNPL_PPN0_INPUT_SELECT_RSVD0)
#define V_PPNNPL_PPN0_INPUT_SELECT_RSVD0(V)                                       _MM_MAKEVALUE((V),S_PPNNPL_PPN0_INPUT_SELECT_RSVD0)
#define G_PPNNPL_PPN0_INPUT_SELECT_RSVD0(V)                                       _MM_GETVALUE((V),S_PPNNPL_PPN0_INPUT_SELECT_RSVD0,M_PPNNPL_PPN0_INPUT_SELECT_RSVD0)

      
#define S_PPNNPL_PPN1_CLE_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN1_CLE_INPUT_ENABLE                                            _MM_MAKEMASK(1,S_PPNNPL_PPN1_CLE_INPUT_ENABLE)
#define V_PPNNPL_PPN1_CLE_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN1_CLE_INPUT_ENABLE)
#define G_PPNNPL_PPN1_CLE_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN1_CLE_INPUT_ENABLE,M_PPNNPL_PPN1_CLE_INPUT_ENABLE)
     
#define S_PPNNPL_PPN1_CLE_OUTPUT_ENABLE                                           1  //Access: read-write
#define M_PPNNPL_PPN1_CLE_OUTPUT_ENABLE                                           _MM_MAKEMASK(1,S_PPNNPL_PPN1_CLE_OUTPUT_ENABLE)
#define V_PPNNPL_PPN1_CLE_OUTPUT_ENABLE(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN1_CLE_OUTPUT_ENABLE)
#define G_PPNNPL_PPN1_CLE_OUTPUT_ENABLE(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN1_CLE_OUTPUT_ENABLE,M_PPNNPL_PPN1_CLE_OUTPUT_ENABLE)
     
#define S_PPNNPL_PPN1_CLE_PULL_ENABLE                                             2  //Access: read-write
#define M_PPNNPL_PPN1_CLE_PULL_ENABLE                                             _MM_MAKEMASK(1,S_PPNNPL_PPN1_CLE_PULL_ENABLE)
#define V_PPNNPL_PPN1_CLE_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_CLE_PULL_ENABLE)
#define G_PPNNPL_PPN1_CLE_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_CLE_PULL_ENABLE,M_PPNNPL_PPN1_CLE_PULL_ENABLE)
     
#define S_PPNNPL_PPN1_CLE_PULL_SELECT                                             3  //Access: read-write
#define M_PPNNPL_PPN1_CLE_PULL_SELECT                                             _MM_MAKEMASK(1,S_PPNNPL_PPN1_CLE_PULL_SELECT)
#define V_PPNNPL_PPN1_CLE_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_CLE_PULL_SELECT)
#define G_PPNNPL_PPN1_CLE_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_CLE_PULL_SELECT,M_PPNNPL_PPN1_CLE_PULL_SELECT)
     
#define S_PPNNPL_PPN1_CLE_RSVD0                                                   4  //Access: read-as-zero
#define M_PPNNPL_PPN1_CLE_RSVD0                                                   _MM_MAKEMASK(28,S_PPNNPL_PPN1_CLE_RSVD0)
#define V_PPNNPL_PPN1_CLE_RSVD0(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_CLE_RSVD0)
#define G_PPNNPL_PPN1_CLE_RSVD0(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN1_CLE_RSVD0,M_PPNNPL_PPN1_CLE_RSVD0)

      
#define S_PPNNPL_PPN1_ALE_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN1_ALE_INPUT_ENABLE                                            _MM_MAKEMASK(1,S_PPNNPL_PPN1_ALE_INPUT_ENABLE)
#define V_PPNNPL_PPN1_ALE_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ALE_INPUT_ENABLE)
#define G_PPNNPL_PPN1_ALE_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN1_ALE_INPUT_ENABLE,M_PPNNPL_PPN1_ALE_INPUT_ENABLE)
     
#define S_PPNNPL_PPN1_ALE_OUTPUT_ENABLE                                           1  //Access: read-write
#define M_PPNNPL_PPN1_ALE_OUTPUT_ENABLE                                           _MM_MAKEMASK(1,S_PPNNPL_PPN1_ALE_OUTPUT_ENABLE)
#define V_PPNNPL_PPN1_ALE_OUTPUT_ENABLE(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ALE_OUTPUT_ENABLE)
#define G_PPNNPL_PPN1_ALE_OUTPUT_ENABLE(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN1_ALE_OUTPUT_ENABLE,M_PPNNPL_PPN1_ALE_OUTPUT_ENABLE)
     
#define S_PPNNPL_PPN1_ALE_PULL_ENABLE                                             2  //Access: read-write
#define M_PPNNPL_PPN1_ALE_PULL_ENABLE                                             _MM_MAKEMASK(1,S_PPNNPL_PPN1_ALE_PULL_ENABLE)
#define V_PPNNPL_PPN1_ALE_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ALE_PULL_ENABLE)
#define G_PPNNPL_PPN1_ALE_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_ALE_PULL_ENABLE,M_PPNNPL_PPN1_ALE_PULL_ENABLE)
     
#define S_PPNNPL_PPN1_ALE_PULL_SELECT                                             3  //Access: read-write
#define M_PPNNPL_PPN1_ALE_PULL_SELECT                                             _MM_MAKEMASK(1,S_PPNNPL_PPN1_ALE_PULL_SELECT)
#define V_PPNNPL_PPN1_ALE_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ALE_PULL_SELECT)
#define G_PPNNPL_PPN1_ALE_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_ALE_PULL_SELECT,M_PPNNPL_PPN1_ALE_PULL_SELECT)
     
#define S_PPNNPL_PPN1_ALE_RSVD0                                                   4  //Access: read-as-zero
#define M_PPNNPL_PPN1_ALE_RSVD0                                                   _MM_MAKEMASK(28,S_PPNNPL_PPN1_ALE_RSVD0)
#define V_PPNNPL_PPN1_ALE_RSVD0(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ALE_RSVD0)
#define G_PPNNPL_PPN1_ALE_RSVD0(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN1_ALE_RSVD0,M_PPNNPL_PPN1_ALE_RSVD0)

      
#define S_PPNNPL_PPN1_REN_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN1_REN_INPUT_ENABLE                                            _MM_MAKEMASK(1,S_PPNNPL_PPN1_REN_INPUT_ENABLE)
#define V_PPNNPL_PPN1_REN_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN1_REN_INPUT_ENABLE)
#define G_PPNNPL_PPN1_REN_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN1_REN_INPUT_ENABLE,M_PPNNPL_PPN1_REN_INPUT_ENABLE)
     
#define S_PPNNPL_PPN1_REN_OUTPUT_ENABLE                                           1  //Access: read-write
#define M_PPNNPL_PPN1_REN_OUTPUT_ENABLE                                           _MM_MAKEMASK(1,S_PPNNPL_PPN1_REN_OUTPUT_ENABLE)
#define V_PPNNPL_PPN1_REN_OUTPUT_ENABLE(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN1_REN_OUTPUT_ENABLE)
#define G_PPNNPL_PPN1_REN_OUTPUT_ENABLE(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN1_REN_OUTPUT_ENABLE,M_PPNNPL_PPN1_REN_OUTPUT_ENABLE)
     
#define S_PPNNPL_PPN1_REN_PULL_ENABLE                                             2  //Access: read-write
#define M_PPNNPL_PPN1_REN_PULL_ENABLE                                             _MM_MAKEMASK(1,S_PPNNPL_PPN1_REN_PULL_ENABLE)
#define V_PPNNPL_PPN1_REN_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_REN_PULL_ENABLE)
#define G_PPNNPL_PPN1_REN_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_REN_PULL_ENABLE,M_PPNNPL_PPN1_REN_PULL_ENABLE)
     
#define S_PPNNPL_PPN1_REN_PULL_SELECT                                             3  //Access: read-write
#define M_PPNNPL_PPN1_REN_PULL_SELECT                                             _MM_MAKEMASK(1,S_PPNNPL_PPN1_REN_PULL_SELECT)
#define V_PPNNPL_PPN1_REN_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_REN_PULL_SELECT)
#define G_PPNNPL_PPN1_REN_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_REN_PULL_SELECT,M_PPNNPL_PPN1_REN_PULL_SELECT)
     
#define S_PPNNPL_PPN1_REN_RSVD0                                                   4  //Access: read-as-zero
#define M_PPNNPL_PPN1_REN_RSVD0                                                   _MM_MAKEMASK(28,S_PPNNPL_PPN1_REN_RSVD0)
#define V_PPNNPL_PPN1_REN_RSVD0(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_REN_RSVD0)
#define G_PPNNPL_PPN1_REN_RSVD0(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN1_REN_RSVD0,M_PPNNPL_PPN1_REN_RSVD0)

      
#define S_PPNNPL_PPN1_WEN_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN1_WEN_INPUT_ENABLE                                            _MM_MAKEMASK(1,S_PPNNPL_PPN1_WEN_INPUT_ENABLE)
#define V_PPNNPL_PPN1_WEN_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN1_WEN_INPUT_ENABLE)
#define G_PPNNPL_PPN1_WEN_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN1_WEN_INPUT_ENABLE,M_PPNNPL_PPN1_WEN_INPUT_ENABLE)
     
#define S_PPNNPL_PPN1_WEN_OUTPUT_ENABLE                                           1  //Access: read-write
#define M_PPNNPL_PPN1_WEN_OUTPUT_ENABLE                                           _MM_MAKEMASK(1,S_PPNNPL_PPN1_WEN_OUTPUT_ENABLE)
#define V_PPNNPL_PPN1_WEN_OUTPUT_ENABLE(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN1_WEN_OUTPUT_ENABLE)
#define G_PPNNPL_PPN1_WEN_OUTPUT_ENABLE(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN1_WEN_OUTPUT_ENABLE,M_PPNNPL_PPN1_WEN_OUTPUT_ENABLE)
     
#define S_PPNNPL_PPN1_WEN_PULL_ENABLE                                             2  //Access: read-write
#define M_PPNNPL_PPN1_WEN_PULL_ENABLE                                             _MM_MAKEMASK(1,S_PPNNPL_PPN1_WEN_PULL_ENABLE)
#define V_PPNNPL_PPN1_WEN_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_WEN_PULL_ENABLE)
#define G_PPNNPL_PPN1_WEN_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_WEN_PULL_ENABLE,M_PPNNPL_PPN1_WEN_PULL_ENABLE)
     
#define S_PPNNPL_PPN1_WEN_PULL_SELECT                                             3  //Access: read-write
#define M_PPNNPL_PPN1_WEN_PULL_SELECT                                             _MM_MAKEMASK(1,S_PPNNPL_PPN1_WEN_PULL_SELECT)
#define V_PPNNPL_PPN1_WEN_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_WEN_PULL_SELECT)
#define G_PPNNPL_PPN1_WEN_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_WEN_PULL_SELECT,M_PPNNPL_PPN1_WEN_PULL_SELECT)
     
#define S_PPNNPL_PPN1_WEN_RSVD0                                                   4  //Access: read-as-zero
#define M_PPNNPL_PPN1_WEN_RSVD0                                                   _MM_MAKEMASK(28,S_PPNNPL_PPN1_WEN_RSVD0)
#define V_PPNNPL_PPN1_WEN_RSVD0(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_WEN_RSVD0)
#define G_PPNNPL_PPN1_WEN_RSVD0(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN1_WEN_RSVD0,M_PPNNPL_PPN1_WEN_RSVD0)

      
#define S_PPNNPL_PPN1_CEN_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN1_CEN_INPUT_ENABLE                                            _MM_MAKEMASK(8,S_PPNNPL_PPN1_CEN_INPUT_ENABLE)
#define V_PPNNPL_PPN1_CEN_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN1_CEN_INPUT_ENABLE)
#define G_PPNNPL_PPN1_CEN_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN1_CEN_INPUT_ENABLE,M_PPNNPL_PPN1_CEN_INPUT_ENABLE)
     
#define S_PPNNPL_PPN1_CEN_OUTPUT_ENABLE                                           8  //Access: read-write
#define M_PPNNPL_PPN1_CEN_OUTPUT_ENABLE                                           _MM_MAKEMASK(8,S_PPNNPL_PPN1_CEN_OUTPUT_ENABLE)
#define V_PPNNPL_PPN1_CEN_OUTPUT_ENABLE(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN1_CEN_OUTPUT_ENABLE)
#define G_PPNNPL_PPN1_CEN_OUTPUT_ENABLE(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN1_CEN_OUTPUT_ENABLE,M_PPNNPL_PPN1_CEN_OUTPUT_ENABLE)
     
#define S_PPNNPL_PPN1_CEN_PULL_ENABLE                                             16  //Access: read-write
#define M_PPNNPL_PPN1_CEN_PULL_ENABLE                                             _MM_MAKEMASK(8,S_PPNNPL_PPN1_CEN_PULL_ENABLE)
#define V_PPNNPL_PPN1_CEN_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_CEN_PULL_ENABLE)
#define G_PPNNPL_PPN1_CEN_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_CEN_PULL_ENABLE,M_PPNNPL_PPN1_CEN_PULL_ENABLE)
     
#define S_PPNNPL_PPN1_CEN_PULL_SELECT                                             24  //Access: read-write
#define M_PPNNPL_PPN1_CEN_PULL_SELECT                                             _MM_MAKEMASK(8,S_PPNNPL_PPN1_CEN_PULL_SELECT)
#define V_PPNNPL_PPN1_CEN_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_CEN_PULL_SELECT)
#define G_PPNNPL_PPN1_CEN_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_CEN_PULL_SELECT,M_PPNNPL_PPN1_CEN_PULL_SELECT)

      
#define S_PPNNPL_PPN1_DQS_INPUT_ENABLE                                            0  //Access: read-write
#define M_PPNNPL_PPN1_DQS_INPUT_ENABLE                                            _MM_MAKEMASK(1,S_PPNNPL_PPN1_DQS_INPUT_ENABLE)
#define V_PPNNPL_PPN1_DQS_INPUT_ENABLE(V)                                         _MM_MAKEVALUE((V),S_PPNNPL_PPN1_DQS_INPUT_ENABLE)
#define G_PPNNPL_PPN1_DQS_INPUT_ENABLE(V)                                         _MM_GETVALUE((V),S_PPNNPL_PPN1_DQS_INPUT_ENABLE,M_PPNNPL_PPN1_DQS_INPUT_ENABLE)
     
#define S_PPNNPL_PPN1_DQS_RSVD0                                                   1  //Access: read-as-zero
#define M_PPNNPL_PPN1_DQS_RSVD0                                                   _MM_MAKEMASK(1,S_PPNNPL_PPN1_DQS_RSVD0)
#define V_PPNNPL_PPN1_DQS_RSVD0(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_DQS_RSVD0)
#define G_PPNNPL_PPN1_DQS_RSVD0(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN1_DQS_RSVD0,M_PPNNPL_PPN1_DQS_RSVD0)
     
#define S_PPNNPL_PPN1_DQS_PULL_ENABLE                                             2  //Access: read-write
#define M_PPNNPL_PPN1_DQS_PULL_ENABLE                                             _MM_MAKEMASK(1,S_PPNNPL_PPN1_DQS_PULL_ENABLE)
#define V_PPNNPL_PPN1_DQS_PULL_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_DQS_PULL_ENABLE)
#define G_PPNNPL_PPN1_DQS_PULL_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_DQS_PULL_ENABLE,M_PPNNPL_PPN1_DQS_PULL_ENABLE)
     
#define S_PPNNPL_PPN1_DQS_PULL_SELECT                                             3  //Access: read-write
#define M_PPNNPL_PPN1_DQS_PULL_SELECT                                             _MM_MAKEMASK(1,S_PPNNPL_PPN1_DQS_PULL_SELECT)
#define V_PPNNPL_PPN1_DQS_PULL_SELECT(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_DQS_PULL_SELECT)
#define G_PPNNPL_PPN1_DQS_PULL_SELECT(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_DQS_PULL_SELECT,M_PPNNPL_PPN1_DQS_PULL_SELECT)
     
#define S_PPNNPL_PPN1_DQS_RSVD1                                                   4  //Access: read-as-zero
#define M_PPNNPL_PPN1_DQS_RSVD1                                                   _MM_MAKEMASK(28,S_PPNNPL_PPN1_DQS_RSVD1)
#define V_PPNNPL_PPN1_DQS_RSVD1(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_DQS_RSVD1)
#define G_PPNNPL_PPN1_DQS_RSVD1(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN1_DQS_RSVD1,M_PPNNPL_PPN1_DQS_RSVD1)

      
#define S_PPNNPL_PPN1_IO_INPUT_ENABLE                                             0  //Access: read-write
#define M_PPNNPL_PPN1_IO_INPUT_ENABLE                                             _MM_MAKEMASK(8,S_PPNNPL_PPN1_IO_INPUT_ENABLE)
#define V_PPNNPL_PPN1_IO_INPUT_ENABLE(V)                                          _MM_MAKEVALUE((V),S_PPNNPL_PPN1_IO_INPUT_ENABLE)
#define G_PPNNPL_PPN1_IO_INPUT_ENABLE(V)                                          _MM_GETVALUE((V),S_PPNNPL_PPN1_IO_INPUT_ENABLE,M_PPNNPL_PPN1_IO_INPUT_ENABLE)
     
#define S_PPNNPL_PPN1_IO_RSVD0                                                    8  //Access: read-as-zero
#define M_PPNNPL_PPN1_IO_RSVD0                                                    _MM_MAKEMASK(8,S_PPNNPL_PPN1_IO_RSVD0)
#define V_PPNNPL_PPN1_IO_RSVD0(V)                                                 _MM_MAKEVALUE((V),S_PPNNPL_PPN1_IO_RSVD0)
#define G_PPNNPL_PPN1_IO_RSVD0(V)                                                 _MM_GETVALUE((V),S_PPNNPL_PPN1_IO_RSVD0,M_PPNNPL_PPN1_IO_RSVD0)
     
#define S_PPNNPL_PPN1_IO_PULL_ENABLE                                              16  //Access: read-write
#define M_PPNNPL_PPN1_IO_PULL_ENABLE                                              _MM_MAKEMASK(8,S_PPNNPL_PPN1_IO_PULL_ENABLE)
#define V_PPNNPL_PPN1_IO_PULL_ENABLE(V)                                           _MM_MAKEVALUE((V),S_PPNNPL_PPN1_IO_PULL_ENABLE)
#define G_PPNNPL_PPN1_IO_PULL_ENABLE(V)                                           _MM_GETVALUE((V),S_PPNNPL_PPN1_IO_PULL_ENABLE,M_PPNNPL_PPN1_IO_PULL_ENABLE)
     
#define S_PPNNPL_PPN1_IO_PULL_SELECT                                              24  //Access: read-write
#define M_PPNNPL_PPN1_IO_PULL_SELECT                                              _MM_MAKEMASK(8,S_PPNNPL_PPN1_IO_PULL_SELECT)
#define V_PPNNPL_PPN1_IO_PULL_SELECT(V)                                           _MM_MAKEVALUE((V),S_PPNNPL_PPN1_IO_PULL_SELECT)
#define G_PPNNPL_PPN1_IO_PULL_SELECT(V)                                           _MM_GETVALUE((V),S_PPNNPL_PPN1_IO_PULL_SELECT,M_PPNNPL_PPN1_IO_PULL_SELECT)

      
#define S_PPNNPL_PPN1_ZQ_OEPD                                                     0  //Access: read-write
#define M_PPNNPL_PPN1_ZQ_OEPD                                                     _MM_MAKEMASK(1,S_PPNNPL_PPN1_ZQ_OEPD)
#define V_PPNNPL_PPN1_ZQ_OEPD(V)                                                  _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZQ_OEPD)
#define G_PPNNPL_PPN1_ZQ_OEPD(V)                                                  _MM_GETVALUE((V),S_PPNNPL_PPN1_ZQ_OEPD,M_PPNNPL_PPN1_ZQ_OEPD)
     
#define S_PPNNPL_PPN1_ZQ_OEPU                                                     1  //Access: read-write
#define M_PPNNPL_PPN1_ZQ_OEPU                                                     _MM_MAKEMASK(1,S_PPNNPL_PPN1_ZQ_OEPU)
#define V_PPNNPL_PPN1_ZQ_OEPU(V)                                                  _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZQ_OEPU)
#define G_PPNNPL_PPN1_ZQ_OEPU(V)                                                  _MM_GETVALUE((V),S_PPNNPL_PPN1_ZQ_OEPU,M_PPNNPL_PPN1_ZQ_OEPU)
     
#define S_PPNNPL_PPN1_ZQ_ZCPD                                                     2  //Access: read-write
#define M_PPNNPL_PPN1_ZQ_ZCPD                                                     _MM_MAKEMASK(4,S_PPNNPL_PPN1_ZQ_ZCPD)
#define V_PPNNPL_PPN1_ZQ_ZCPD(V)                                                  _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZQ_ZCPD)
#define G_PPNNPL_PPN1_ZQ_ZCPD(V)                                                  _MM_GETVALUE((V),S_PPNNPL_PPN1_ZQ_ZCPD,M_PPNNPL_PPN1_ZQ_ZCPD)
     
#define S_PPNNPL_PPN1_ZQ_ZCPU                                                     6  //Access: read-write
#define M_PPNNPL_PPN1_ZQ_ZCPU                                                     _MM_MAKEMASK(4,S_PPNNPL_PPN1_ZQ_ZCPU)
#define V_PPNNPL_PPN1_ZQ_ZCPU(V)                                                  _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZQ_ZCPU)
#define G_PPNNPL_PPN1_ZQ_ZCPU(V)                                                  _MM_GETVALUE((V),S_PPNNPL_PPN1_ZQ_ZCPU,M_PPNNPL_PPN1_ZQ_ZCPU)
     
#define S_PPNNPL_PPN1_ZQ_RSVD0                                                    10  //Access: read-as-zero
#define M_PPNNPL_PPN1_ZQ_RSVD0                                                    _MM_MAKEMASK(22,S_PPNNPL_PPN1_ZQ_RSVD0)
#define V_PPNNPL_PPN1_ZQ_RSVD0(V)                                                 _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZQ_RSVD0)
#define G_PPNNPL_PPN1_ZQ_RSVD0(V)                                                 _MM_GETVALUE((V),S_PPNNPL_PPN1_ZQ_RSVD0,M_PPNNPL_PPN1_ZQ_RSVD0)

      
#define S_PPNNPL_PPN1_ZQ_IN_YPD                                                   0  //Access: read-only
#define M_PPNNPL_PPN1_ZQ_IN_YPD                                                   _MM_MAKEMASK(1,S_PPNNPL_PPN1_ZQ_IN_YPD)
#define V_PPNNPL_PPN1_ZQ_IN_YPD(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZQ_IN_YPD)
#define G_PPNNPL_PPN1_ZQ_IN_YPD(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN1_ZQ_IN_YPD,M_PPNNPL_PPN1_ZQ_IN_YPD)
     
#define S_PPNNPL_PPN1_ZQ_IN_YPU                                                   1  //Access: read-only
#define M_PPNNPL_PPN1_ZQ_IN_YPU                                                   _MM_MAKEMASK(1,S_PPNNPL_PPN1_ZQ_IN_YPU)
#define V_PPNNPL_PPN1_ZQ_IN_YPU(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZQ_IN_YPU)
#define G_PPNNPL_PPN1_ZQ_IN_YPU(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN1_ZQ_IN_YPU,M_PPNNPL_PPN1_ZQ_IN_YPU)
     
#define S_PPNNPL_PPN1_ZQ_IN_RSVD0                                                 2  //Access: read-as-zero
#define M_PPNNPL_PPN1_ZQ_IN_RSVD0                                                 _MM_MAKEMASK(30,S_PPNNPL_PPN1_ZQ_IN_RSVD0)
#define V_PPNNPL_PPN1_ZQ_IN_RSVD0(V)                                              _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZQ_IN_RSVD0)
#define G_PPNNPL_PPN1_ZQ_IN_RSVD0(V)                                              _MM_GETVALUE((V),S_PPNNPL_PPN1_ZQ_IN_RSVD0,M_PPNNPL_PPN1_ZQ_IN_RSVD0)

      
#define S_PPNNPL_PPN1_ZCPD_ZCPD                                                   0  //Access: read-write
#define M_PPNNPL_PPN1_ZCPD_ZCPD                                                   _MM_MAKEMASK(4,S_PPNNPL_PPN1_ZCPD_ZCPD)
#define V_PPNNPL_PPN1_ZCPD_ZCPD(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZCPD_ZCPD)
#define G_PPNNPL_PPN1_ZCPD_ZCPD(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN1_ZCPD_ZCPD,M_PPNNPL_PPN1_ZCPD_ZCPD)
     
#define S_PPNNPL_PPN1_ZCPD_RSVD0                                                  4  //Access: read-as-zero
#define M_PPNNPL_PPN1_ZCPD_RSVD0                                                  _MM_MAKEMASK(28,S_PPNNPL_PPN1_ZCPD_RSVD0)
#define V_PPNNPL_PPN1_ZCPD_RSVD0(V)                                               _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZCPD_RSVD0)
#define G_PPNNPL_PPN1_ZCPD_RSVD0(V)                                               _MM_GETVALUE((V),S_PPNNPL_PPN1_ZCPD_RSVD0,M_PPNNPL_PPN1_ZCPD_RSVD0)

      
#define S_PPNNPL_PPN1_ZCPU_ZCPU                                                   0  //Access: read-write
#define M_PPNNPL_PPN1_ZCPU_ZCPU                                                   _MM_MAKEMASK(4,S_PPNNPL_PPN1_ZCPU_ZCPU)
#define V_PPNNPL_PPN1_ZCPU_ZCPU(V)                                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZCPU_ZCPU)
#define G_PPNNPL_PPN1_ZCPU_ZCPU(V)                                                _MM_GETVALUE((V),S_PPNNPL_PPN1_ZCPU_ZCPU,M_PPNNPL_PPN1_ZCPU_ZCPU)
     
#define S_PPNNPL_PPN1_ZCPU_RSVD0                                                  4  //Access: read-as-zero
#define M_PPNNPL_PPN1_ZCPU_RSVD0                                                  _MM_MAKEMASK(28,S_PPNNPL_PPN1_ZCPU_RSVD0)
#define V_PPNNPL_PPN1_ZCPU_RSVD0(V)                                               _MM_MAKEVALUE((V),S_PPNNPL_PPN1_ZCPU_RSVD0)
#define G_PPNNPL_PPN1_ZCPU_RSVD0(V)                                               _MM_GETVALUE((V),S_PPNNPL_PPN1_ZCPU_RSVD0,M_PPNNPL_PPN1_ZCPU_RSVD0)

      
#define S_PPNNPL_PPN1_DS_DRIVE_STRENGTH                                           0  //Access: read-write
#define M_PPNNPL_PPN1_DS_DRIVE_STRENGTH                                           _MM_MAKEMASK(4,S_PPNNPL_PPN1_DS_DRIVE_STRENGTH)
#define V_PPNNPL_PPN1_DS_DRIVE_STRENGTH(V)                                        _MM_MAKEVALUE((V),S_PPNNPL_PPN1_DS_DRIVE_STRENGTH)
#define G_PPNNPL_PPN1_DS_DRIVE_STRENGTH(V)                                        _MM_GETVALUE((V),S_PPNNPL_PPN1_DS_DRIVE_STRENGTH,M_PPNNPL_PPN1_DS_DRIVE_STRENGTH)
     
#define S_PPNNPL_PPN1_DS_RSVD0                                                    4  //Access: read-as-zero
#define M_PPNNPL_PPN1_DS_RSVD0                                                    _MM_MAKEMASK(28,S_PPNNPL_PPN1_DS_RSVD0)
#define V_PPNNPL_PPN1_DS_RSVD0(V)                                                 _MM_MAKEVALUE((V),S_PPNNPL_PPN1_DS_RSVD0)
#define G_PPNNPL_PPN1_DS_RSVD0(V)                                                 _MM_GETVALUE((V),S_PPNNPL_PPN1_DS_RSVD0,M_PPNNPL_PPN1_DS_RSVD0)

      
#define S_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT                                   0  //Access: read-write
#define M_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT                                   _MM_MAKEMASK(1,S_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT)
#define V_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT(V)                                _MM_MAKEVALUE((V),S_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT)
#define G_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT(V)                                _MM_GETVALUE((V),S_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT,M_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT)
     
#define S_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT_SCHMITT                           1  //Access: read-write
#define M_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT_SCHMITT                           _MM_MAKEMASK(1,S_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT_SCHMITT)
#define V_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT_SCHMITT(V)                        _MM_MAKEVALUE((V),S_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT_SCHMITT)
#define G_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT_SCHMITT(V)                        _MM_GETVALUE((V),S_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT_SCHMITT,M_PPNNPL_PPN1_INPUT_SELECT_INPUT_SELECT_SCHMITT)
     
#define S_PPNNPL_PPN1_INPUT_SELECT_RSVD0                                          2  //Access: read-as-zero
#define M_PPNNPL_PPN1_INPUT_SELECT_RSVD0                                          _MM_MAKEMASK(30,S_PPNNPL_PPN1_INPUT_SELECT_RSVD0)
#define V_PPNNPL_PPN1_INPUT_SELECT_RSVD0(V)                                       _MM_MAKEVALUE((V),S_PPNNPL_PPN1_INPUT_SELECT_RSVD0)
#define G_PPNNPL_PPN1_INPUT_SELECT_RSVD0(V)                                       _MM_GETVALUE((V),S_PPNNPL_PPN1_INPUT_SELECT_RSVD0,M_PPNNPL_PPN1_INPUT_SELECT_RSVD0)

                                                                                                                                                                                                                                                                                                                                                                                                             

/******************************************************************************/
/* Register Fields typedef structs */
/******************************************************************************/
#ifndef __ASSEMBLY__     
    
typedef union ppnnpl_version_t {                                      //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t revision:16;                                         //Access: read-only     
        uint32_t version:16;                                          //Access: read-only  
        }f;
} ppnnpl_version_t;
    
typedef union ppnnpl_config_t {                                       //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t auto_pulldone_enable:1;                              //Access: read-write     
        uint32_t rsvd0:31;                                            //Access: read-as-zero  
        }f;
} ppnnpl_config_t;
    
typedef union ppnnpl_dqs_timing_t {                                   //RRV:0x003f003f 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t anc0_default_delay_code:9;                           //Access: read-write     
        uint32_t anc0_use_default_delay_code:1;                       //Access: read-write     
        uint32_t anc0_dll_select:1;                                   //Access: read-write     
        uint32_t rsvd0:5;                                             //Access: read-as-zero     
        uint32_t anc1_default_delay_code:9;                           //Access: read-write     
        uint32_t anc1_use_default_delay_code:1;                       //Access: read-write     
        uint32_t anc1_dll_select:1;                                   //Access: read-write     
        uint32_t rsvd1:5;                                             //Access: read-as-zero  
        }f;
} ppnnpl_dqs_timing_t;
    
typedef union ppnnpl_dll_codes_t {                                    //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t master_delay_out:7;                                  //Access: read-only     
        uint32_t slave0_delay_code_in:9;                              //Access: read-only     
        uint32_t slave1_delay_code_in:9;                              //Access: read-only     
        uint32_t rsvd0:7;                                             //Access: read-as-zero  
        }f;
} ppnnpl_dll_codes_t;
    
typedef union ppnnpl_dqs_adjust_t {                                   //RRV:0x00400040 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t anc0_frequency_ratio:8;                              //Access: read-write     
        uint32_t anc0_offset:7;                                       //Access: read-write     
        uint32_t rsvd0:1;                                             //Access: read-as-zero     
        uint32_t anc1_frequency_ratio:8;                              //Access: read-write     
        uint32_t anc1_offset:7;                                       //Access: read-write     
        uint32_t rsvd1:1;                                             //Access: read-as-zero  
        }f;
} ppnnpl_dqs_adjust_t;
    
typedef union ppnnpl_ampmcdll_control_t {                             //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t stepsize_in:1;                                       //Access: read-write     
        uint32_t stepsize_sel:1;                                      //Access: read-write     
        uint32_t rsvd0:30;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ampmcdll_control_t;
    
typedef union ppnnpl_ampslv_control_t {                               //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t anc0_bypass_mode:2;                                  //Access: read-write     
        uint32_t anc1_bypass_mode:2;                                  //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ampslv_control_t;
    
typedef union ppnnpl_ampdll_status_t {                                //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t dll_locked:1;                                        //Access: read-only     
        uint32_t rsvd0:31;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ampdll_status_t;
    
typedef union ppnnpl_anc0_ampslv0_t {                                 //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t slv_sel:12;                                          //Access: read-only     
        uint32_t rsvd0:20;                                            //Access: read-as-zero  
        }f;
} ppnnpl_anc0_ampslv0_t;
    
typedef union ppnnpl_anc0_ampslv1_t {                                 //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t slv_sel:12;                                          //Access: read-only     
        uint32_t rsvd0:20;                                            //Access: read-as-zero  
        }f;
} ppnnpl_anc0_ampslv1_t;
    
typedef union ppnnpl_anc1_ampslv0_t {                                 //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t slv_sel:12;                                          //Access: read-only     
        uint32_t rsvd0:20;                                            //Access: read-as-zero  
        }f;
} ppnnpl_anc1_ampslv0_t;
    
typedef union ppnnpl_anc1_ampslv1_t {                                 //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t slv_sel:12;                                          //Access: read-only     
        uint32_t rsvd0:20;                                            //Access: read-as-zero  
        }f;
} ppnnpl_anc1_ampslv1_t;
    
typedef union ppnnpl_train_control_t {                                //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t disable_continuous_training:1;                       //Access: read-write     
        uint32_t train_periodically:1;                                //Access: read-write     
        uint32_t train_once:1;                                        //Access: write-auto-clear     
        uint32_t rsvd0:29;                                            //Access: read-as-zero  
        }f;
} ppnnpl_train_control_t;
    
typedef union ppnnpl_periodic_training_t {                            //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t delay:32;                                            //Access: read-write  
        }f;
} ppnnpl_periodic_training_t;
    
typedef union ppnnpl_debug_dll_control_t {                            //RRV:0x4c000034 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t start_delay:3;                                       //Access: read-write     
        uint32_t rsvd0:1;                                             //Access: read-as-zero     
        uint32_t step_size:3;                                         //Access: read-write     
        uint32_t rsvd1:9;                                             //Access: read-as-zero     
        uint32_t train:1;                                             //Access: read-write     
        uint32_t rsvd2:7;                                             //Access: read-as-zero     
        uint32_t train_prepare_delay:4;                               //Access: read-write     
        uint32_t train_pause_delay:4;                                 //Access: read-write  
        }f;
} ppnnpl_debug_dll_control_t;
    
typedef union ppnnpl_debug_data_capture_t {                           //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t anc0_posedge_data:8;                                 //Access: read-only     
        uint32_t anc1_posedge_data:8;                                 //Access: read-only     
        uint32_t rsvd0:16;                                            //Access: read-as-zero  
        }f;
} ppnnpl_debug_data_capture_t;
    
typedef union ppnnpl_ppn0_cle_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:1;                                      //Access: read-write     
        uint32_t output_enable:1;                                     //Access: read-write     
        uint32_t pull_enable:1;                                       //Access: read-write     
        uint32_t pull_select:1;                                       //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_cle_t;
    
typedef union ppnnpl_ppn0_ale_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:1;                                      //Access: read-write     
        uint32_t output_enable:1;                                     //Access: read-write     
        uint32_t pull_enable:1;                                       //Access: read-write     
        uint32_t pull_select:1;                                       //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_ale_t;
    
typedef union ppnnpl_ppn0_ren_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:1;                                      //Access: read-write     
        uint32_t output_enable:1;                                     //Access: read-write     
        uint32_t pull_enable:1;                                       //Access: read-write     
        uint32_t pull_select:1;                                       //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_ren_t;
    
typedef union ppnnpl_ppn0_wen_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:1;                                      //Access: read-write     
        uint32_t output_enable:1;                                     //Access: read-write     
        uint32_t pull_enable:1;                                       //Access: read-write     
        uint32_t pull_select:1;                                       //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_wen_t;
    
typedef union ppnnpl_ppn0_cen_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:8;                                      //Access: read-write     
        uint32_t output_enable:8;                                     //Access: read-write     
        uint32_t pull_enable:8;                                       //Access: read-write     
        uint32_t pull_select:8;                                       //Access: read-write  
        }f;
} ppnnpl_ppn0_cen_t;
    
typedef union ppnnpl_ppn0_dqs_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:1;                                      //Access: read-write     
        uint32_t rsvd0:1;                                             //Access: read-as-zero     
        uint32_t pull_enable:1;                                       //Access: read-write     
        uint32_t pull_select:1;                                       //Access: read-write     
        uint32_t rsvd1:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_dqs_t;
    
typedef union ppnnpl_ppn0_io_t {                                      //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:8;                                      //Access: read-write     
        uint32_t rsvd0:8;                                             //Access: read-as-zero     
        uint32_t pull_enable:8;                                       //Access: read-write     
        uint32_t pull_select:8;                                       //Access: read-write  
        }f;
} ppnnpl_ppn0_io_t;
    
typedef union ppnnpl_ppn0_zq_t {                                      //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t oepd:1;                                              //Access: read-write     
        uint32_t oepu:1;                                              //Access: read-write     
        uint32_t zcpd:4;                                              //Access: read-write     
        uint32_t zcpu:4;                                              //Access: read-write     
        uint32_t rsvd0:22;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_zq_t;
    
typedef union ppnnpl_ppn0_zq_in_t {                                   //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t ypd:1;                                               //Access: read-only     
        uint32_t ypu:1;                                               //Access: read-only     
        uint32_t rsvd0:30;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_zq_in_t;
    
typedef union ppnnpl_ppn0_zcpd_t {                                    //RRV:0x00000008 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t zcpd:4;                                              //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_zcpd_t;
    
typedef union ppnnpl_ppn0_zcpu_t {                                    //RRV:0x00000008 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t zcpu:4;                                              //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_zcpu_t;
    
typedef union ppnnpl_ppn0_ds_t {                                      //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t drive_strength:4;                                    //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_ds_t;
    
typedef union ppnnpl_ppn0_input_select_t {                            //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_select:1;                                      //Access: read-write     
        uint32_t input_select_schmitt:1;                              //Access: read-write     
        uint32_t rsvd0:30;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn0_input_select_t;
    
typedef union ppnnpl_ppn1_cle_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:1;                                      //Access: read-write     
        uint32_t output_enable:1;                                     //Access: read-write     
        uint32_t pull_enable:1;                                       //Access: read-write     
        uint32_t pull_select:1;                                       //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_cle_t;
    
typedef union ppnnpl_ppn1_ale_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:1;                                      //Access: read-write     
        uint32_t output_enable:1;                                     //Access: read-write     
        uint32_t pull_enable:1;                                       //Access: read-write     
        uint32_t pull_select:1;                                       //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_ale_t;
    
typedef union ppnnpl_ppn1_ren_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:1;                                      //Access: read-write     
        uint32_t output_enable:1;                                     //Access: read-write     
        uint32_t pull_enable:1;                                       //Access: read-write     
        uint32_t pull_select:1;                                       //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_ren_t;
    
typedef union ppnnpl_ppn1_wen_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:1;                                      //Access: read-write     
        uint32_t output_enable:1;                                     //Access: read-write     
        uint32_t pull_enable:1;                                       //Access: read-write     
        uint32_t pull_select:1;                                       //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_wen_t;
    
typedef union ppnnpl_ppn1_cen_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:8;                                      //Access: read-write     
        uint32_t output_enable:8;                                     //Access: read-write     
        uint32_t pull_enable:8;                                       //Access: read-write     
        uint32_t pull_select:8;                                       //Access: read-write  
        }f;
} ppnnpl_ppn1_cen_t;
    
typedef union ppnnpl_ppn1_dqs_t {                                     //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:1;                                      //Access: read-write     
        uint32_t rsvd0:1;                                             //Access: read-as-zero     
        uint32_t pull_enable:1;                                       //Access: read-write     
        uint32_t pull_select:1;                                       //Access: read-write     
        uint32_t rsvd1:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_dqs_t;
    
typedef union ppnnpl_ppn1_io_t {                                      //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_enable:8;                                      //Access: read-write     
        uint32_t rsvd0:8;                                             //Access: read-as-zero     
        uint32_t pull_enable:8;                                       //Access: read-write     
        uint32_t pull_select:8;                                       //Access: read-write  
        }f;
} ppnnpl_ppn1_io_t;
    
typedef union ppnnpl_ppn1_zq_t {                                      //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t oepd:1;                                              //Access: read-write     
        uint32_t oepu:1;                                              //Access: read-write     
        uint32_t zcpd:4;                                              //Access: read-write     
        uint32_t zcpu:4;                                              //Access: read-write     
        uint32_t rsvd0:22;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_zq_t;
    
typedef union ppnnpl_ppn1_zq_in_t {                                   //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t ypd:1;                                               //Access: read-only     
        uint32_t ypu:1;                                               //Access: read-only     
        uint32_t rsvd0:30;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_zq_in_t;
    
typedef union ppnnpl_ppn1_zcpd_t {                                    //RRV:0x00000008 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t zcpd:4;                                              //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_zcpd_t;
    
typedef union ppnnpl_ppn1_zcpu_t {                                    //RRV:0x00000008 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t zcpu:4;                                              //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_zcpu_t;
    
typedef union ppnnpl_ppn1_ds_t {                                      //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t drive_strength:4;                                    //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_ds_t;
    
typedef union ppnnpl_ppn1_input_select_t {                            //RRV:0x00000000 
    uint32_t all;                                                     //RRM:0x00000000
    struct {                                                              
        uint32_t input_select:1;                                      //Access: read-write     
        uint32_t input_select_schmitt:1;                              //Access: read-write     
        uint32_t rsvd0:30;                                            //Access: read-as-zero  
        }f;
} ppnnpl_ppn1_input_select_t;
  

/******************************************************************************/
/* Registers typedef structs */
/******************************************************************************/
/* errors in perl template and spds required these to be commented out
     
  
typedef union ppnnpl_t {
        uint32_t all[1024];
        struct {      
           ppnnpl_version_t                                  VERSION                        ; // 0x00000000       
           ppnnpl_config_t                                   CONFIG                         ; // 0x00000004       
           ppnnpl_dqs_timing_t                               DQS_TIMING                     ; // 0x00000008       
           ppnnpl_dll_codes_t                                DLL_CODES                      ; // 0x0000000c       
           ppnnpl_dqs_adjust_t                               DQS_ADJUST                     ; // 0x00000010       
           ppnnpl_ampmcdll_control_t                         AMPMCDLL_CONTROL               ; // 0x00000014       
           ppnnpl_ampslv_control_t                           AMPSLV_CONTROL                 ; // 0x00000018       
           ppnnpl_ampdll_status_t                            AMPDLL_STATUS                  ; // 0x0000001c       
           ppnnpl_anc0_ampslv0_t                             ANC0_AMPSLV0                   ; // 0x00000020       
           ppnnpl_anc0_ampslv1_t                             ANC0_AMPSLV1                   ; // 0x00000024       
           ppnnpl_anc1_ampslv0_t                             ANC1_AMPSLV0                   ; // 0x00000028       
           ppnnpl_anc1_ampslv1_t                             ANC1_AMPSLV1                   ; // 0x0000002c       
           ppnnpl_train_control_t                            TRAIN_CONTROL                  ; // 0x00000030       
           ppnnpl_periodic_training_t                        PERIODIC_TRAINING              ; // 0x00000034       
           ppnnpl_debug_dll_control_t                        DEBUG_DLL_CONTROL              ; // 0x00000038       
           ppnnpl_debug_data_capture_t                       DEBUG_DATA_CAPTURE             ; // 0x0000003c       
           ppnnpl_ppn0_cle_t                                 PPN0_CLE                       ; // 0x00000040       
           ppnnpl_ppn0_ale_t                                 PPN0_ALE                       ; // 0x00000044       
           ppnnpl_ppn0_ren_t                                 PPN0_REN                       ; // 0x00000048       
           ppnnpl_ppn0_wen_t                                 PPN0_WEN                       ; // 0x0000004c       
           ppnnpl_ppn0_cen_t                                 PPN0_CEN                       ; // 0x00000050       
           ppnnpl_ppn0_dqs_t                                 PPN0_DQS                       ; // 0x00000054       
           ppnnpl_ppn0_io_t                                  PPN0_IO                        ; // 0x00000058       
           ppnnpl_ppn0_zq_t                                  PPN0_ZQ                        ; // 0x0000005c       
           ppnnpl_ppn0_zq_in_t                               PPN0_ZQ_IN                     ; // 0x00000060       
           ppnnpl_ppn0_zcpd_t                                PPN0_ZCPD                      ; // 0x00000064       
           ppnnpl_ppn0_zcpu_t                                PPN0_ZCPU                      ; // 0x00000068       
           ppnnpl_ppn0_ds_t                                  PPN0_DS                        ; // 0x0000006c       
           ppnnpl_ppn0_input_select_t                        PPN0_INPUT_SELECT              ; // 0x00000070       
           ppnnpl_ppn1_cle_t                                 PPN1_CLE                       ; // 0x00000074       
           ppnnpl_ppn1_ale_t                                 PPN1_ALE                       ; // 0x00000078       
           ppnnpl_ppn1_ren_t                                 PPN1_REN                       ; // 0x0000007c       
           ppnnpl_ppn1_wen_t                                 PPN1_WEN                       ; // 0x00000080       
           ppnnpl_ppn1_cen_t                                 PPN1_CEN                       ; // 0x00000084       
           ppnnpl_ppn1_dqs_t                                 PPN1_DQS                       ; // 0x00000088       
           ppnnpl_ppn1_io_t                                  PPN1_IO                        ; // 0x0000008c       
           ppnnpl_ppn1_zq_t                                  PPN1_ZQ                        ; // 0x00000090       
           ppnnpl_ppn1_zq_in_t                               PPN1_ZQ_IN                     ; // 0x00000094       
           ppnnpl_ppn1_zcpd_t                                PPN1_ZCPD                      ; // 0x00000098       
           ppnnpl_ppn1_zcpu_t                                PPN1_ZCPU                      ; // 0x0000009c       
           ppnnpl_ppn1_ds_t                                  PPN1_DS                        ; // 0x000000a0       
           ppnnpl_ppn1_input_select_t                        PPN1_INPUT_SELECT              ; // 0x000000a4  
        }r;
} ppnnpl_t;
  

typedef union ppnnpl_regs_s{
        struct {           
           ppnnpl_t                      ppnNpl;                       // 0x00000000   
        }ab;
        uint32_t all[1024];
    };
}ppnnpl_regs_t;
*/

/******************************************************************************/
/* Registers enum */
/******************************************************************************/
/* errors in perl template and spds required these to be commented out
    
    
typedef enum ppnnpl_regenum {     
           version = 0,       
           config = 1,       
           dqs_timing = 2,       
           dll_codes = 3,       
           dqs_adjust = 4,       
           ampmcdll_control = 5,       
           ampslv_control = 6,       
           ampdll_status = 7,       
           anc0_ampslv0 = 8,       
           anc0_ampslv1 = 9,       
           anc1_ampslv0 = 10,       
           anc1_ampslv1 = 11,       
           train_control = 12,       
           periodic_training = 13,       
           debug_dll_control = 14,       
           debug_data_capture = 15,       
           ppn0_cle = 16,       
           ppn0_ale = 17,       
           ppn0_ren = 18,       
           ppn0_wen = 19,       
           ppn0_cen = 20,       
           ppn0_dqs = 21,       
           ppn0_io = 22,       
           ppn0_zq = 23,       
           ppn0_zq_in = 24,       
           ppn0_zcpd = 25,       
           ppn0_zcpu = 26,       
           ppn0_ds = 27,       
           ppn0_input_select = 28,       
           ppn1_cle = 29,       
           ppn1_ale = 30,       
           ppn1_ren = 31,       
           ppn1_wen = 32,       
           ppn1_cen = 33,       
           ppn1_dqs = 34,       
           ppn1_io = 35,       
           ppn1_zq = 36,       
           ppn1_zq_in = 37,       
           ppn1_zcpd = 38,       
           ppn1_zcpu = 39,       
           ppn1_ds = 40,       
           ppn1_input_select = 41,   
}ppnnpl_regenum;
  

*/
#endif
#endif
