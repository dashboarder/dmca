
//
// Copyright (C) 2013 Apple Inc. All rights reserved.
//
// This document is the property of Apple Inc.
// It is considered confidential and proprietary.
//
// This document may not be reproduced or transmitted in any form
// in whole or in part, without the express written permission of
// Apple Inc.
//
#ifndef __ANC_H__
#define __ANC_H__          

#define _MM_MAKEMASK(width, offset)                     (((1 << (width)) - 1) << (offset))
#define _MM_MAKEVALUE(value, offset)                    ((value) << (offset))
#define _MM_GETVALUE(registerContent, offset, mask)     (((registerContent) & (mask))>> (offset))

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_CHAN_BASE                                    0x00000000
#define A_ANC_CHAN_REG(REG)                                (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_CHAN_VERSION                                 0x00000000               
#define R_ANC_CHAN_CONFIG                                  0x00000004               
#define R_ANC_CHAN_INT_STATUS                              0x00000008               
#define R_ANC_CHAN_INT_ENABLE                              0x0000000c               
#define R_ANC_CHAN_DMA_WATERMARKS                          0x00000010               
#define R_ANC_CHAN_LINK_WATERMARKS                         0x00000014               
#define R_ANC_CHAN_STATUS_WATERMARKS                       0x00000018               
#define R_ANC_CHAN_DMA_CMDQ_FIFO_STATUS                    0x0000001c               
#define R_ANC_CHAN_LINK_CMDQ_FIFO_STATUS                   0x00000020               
#define R_ANC_CHAN_LINK_BYPASS_FIFO_STATUS                 0x00000024               
#define R_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS               0x00000028               
#define R_ANC_CHAN_DMA_DEBUG_FIFO_STATUS                   0x0000002c               
#define R_ANC_CHAN_LINK_DEBUG_FIFO_STATUS                  0x00000030               
#define R_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS             0x00000034               
#define R_ANC_CHAN_DMA_CMDQ_FIFO                           0x00000100               
#define R_ANC_CHAN_LINK_CMDQ_FIFO                          0x00000140               
#define R_ANC_CHAN_LINK_BYPASS_FIFO                        0x00000180               
#define R_ANC_CHAN_LINK_PIO_READ_FIFO                      0x000001c0               
#define R_ANC_CHAN_DMA_DEBUG_FIFO                          0x00000200               
#define R_ANC_CHAN_LINK_DEBUG_FIFO                         0x00000240               
#define R_ANC_CHAN_LINK_ECC_STATUS_FIFO                    0x00000280      
#define R_ANC_CHAN_LAST                                    0x00000280 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_CHAN_VERSION                                           
#define rANC_CHAN_CONFIG                                            
#define rANC_CHAN_INT_STATUS                                        
#define rANC_CHAN_INT_ENABLE                                        
#define rANC_CHAN_DMA_WATERMARKS                                    
#define rANC_CHAN_LINK_WATERMARKS                                   
#define rANC_CHAN_STATUS_WATERMARKS                                 
#define rANC_CHAN_DMA_CMDQ_FIFO_STATUS                              
#define rANC_CHAN_LINK_CMDQ_FIFO_STATUS                             
#define rANC_CHAN_LINK_BYPASS_FIFO_STATUS                           
#define rANC_CHAN_LINK_PIO_READ_FIFO_STATUS                         
#define rANC_CHAN_DMA_DEBUG_FIFO_STATUS                             
#define rANC_CHAN_LINK_DEBUG_FIFO_STATUS                            
#define rANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS                       
#define rANC_CHAN_DMA_CMDQ_FIFO                                     
#define rANC_CHAN_LINK_CMDQ_FIFO                                    
#define rANC_CHAN_LINK_BYPASS_FIFO                                  
#define rANC_CHAN_LINK_PIO_READ_FIFO                                
#define rANC_CHAN_DMA_DEBUG_FIFO                                    
#define rANC_CHAN_LINK_DEBUG_FIFO                                   
#define rANC_CHAN_LINK_ECC_STATUS_FIFO                          
#define rANC_CHAN_LAST                                       
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_DMA_BASE                                     0x00001000
#define A_ANC_DMA_REG(REG)                                 (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_DMA_CONFIG                                   0x00001000               
#define R_ANC_DMA_CONTROL                                  0x00001004               
#define R_ANC_DMA_STATUS                                   0x00001008               
#define R_ANC_DMA_AES_STATUS                               0x0000100c               
#define R_ANC_DMA_CMDQ_COUNT                               0x00001010               
#define R_ANC_DMA_CMD_TIMEOUT                              0x00001014               
#define R_ANC_DMA_CMDQ_INT_CODE                            0x00001018               
#define R_ANC_DMA_AXI                                      0x0000101c               
#define R_ANC_DMA_AXI_NEXT                                 0x00001020               
#define R_ANC_DMA_AXI_NEXT_ADDRESS                         0x00001024               
#define R_ANC_DMA_AXI_DMA_COUNT                            0x00001028               
#define R_ANC_DMA_AXI_COUNT                                0x0000102c               
#define R_ANC_DMA_DLFIFO_COUNT                             0x00001030               
#define R_ANC_DMA_DLFIFO_DMA_COUNT                         0x00001034               
#define R_ANC_DMA_AXI_DMA_LAST_COUNT                       0x00001038               
#define R_ANC_DMA_AXI_LAST_COUNT                           0x0000103c               
#define R_ANC_DMA_DLFIFO_LAST_COUNT                        0x00001040               
#define R_ANC_DMA_DLFIFO_DMA_LAST_COUNT                    0x00001044      
#define R_ANC_DMA_LAST                                     0x00001044 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_DMA_CONFIG                                             
#define rANC_DMA_CONTROL                                            
#define rANC_DMA_STATUS                                             
#define rANC_DMA_AES_STATUS                                         
#define rANC_DMA_CMDQ_COUNT                                         
#define rANC_DMA_CMD_TIMEOUT                                        
#define rANC_DMA_CMDQ_INT_CODE                                      
#define rANC_DMA_AXI                                                
#define rANC_DMA_AXI_NEXT                                           
#define rANC_DMA_AXI_NEXT_ADDRESS                                   
#define rANC_DMA_AXI_DMA_COUNT                                      
#define rANC_DMA_AXI_COUNT                                          
#define rANC_DMA_DLFIFO_COUNT                                       
#define rANC_DMA_DLFIFO_DMA_COUNT                                   
#define rANC_DMA_AXI_DMA_LAST_COUNT                                 
#define rANC_DMA_AXI_LAST_COUNT                                     
#define rANC_DMA_DLFIFO_LAST_COUNT                                  
#define rANC_DMA_DLFIFO_DMA_LAST_COUNT                          
#define rANC_DMA_LAST                                        
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_LINK_BASE                                    0x00002000
#define A_ANC_LINK_REG(REG)                                (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_LINK_CONFIG                                  0x00002000               
#define R_ANC_LINK_CONTROL                                 0x00002004               
#define R_ANC_LINK_STATUS                                  0x00002008               
#define R_ANC_LINK_CHIP_ENABLE                             0x0000200c               
#define R_ANC_LINK_READ_STATUS_CONFIG                      0x00002010               
#define R_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING            0x00002014               
#define R_ANC_LINK_SDR_TIMING                              0x00002018               
#define R_ANC_LINK_SDR_DATA_TIMING                         0x0000201c               
#define R_ANC_LINK_DDR_DATA_TIMING                         0x00002020               
#define R_ANC_LINK_DDR_READ_TIMING                         0x00002024               
#define R_ANC_LINK_DDR_WRITE_TIMING                        0x00002028               
#define R_ANC_LINK_DQS_TIMING                              0x0000202c               
#define R_ANC_LINK_CMDQ_COUNT                              0x00002030               
#define R_ANC_LINK_BYPASS_COUNT                            0x00002034               
#define R_ANC_LINK_CMD_TIMEOUT                             0x00002038               
#define R_ANC_LINK_CMDQ_INT_CODE                           0x0000203c               
#define R_ANC_LINK_TIMER                                   0x00002040               
#define R_ANC_LINK_CRC                                     0x00002044               
#define R_ANC_LINK_NPL_INTERFACE                           0x00002048               
#define R_ANC_LINK_MACRO_STATUS                            0x0000204c               
#define R_ANC_LINK_BYPASS_MACRO_STATUS                     0x00002050               
#define R_ANC_LINK_NAND_STATUS                             0x00002054               
#define R_ANC_LINK_DLFIFO_LINK_COUNT                       0x00002058               
#define R_ANC_LINK_DLFIFO_COUNT                            0x0000205c               
#define R_ANC_LINK_PIO_LINK_COUNT                          0x00002060               
#define R_ANC_LINK_PIO_COUNT                               0x00002064               
#define R_ANC_LINK_PHY_COUNT                               0x00002068               
#define R_ANC_LINK_PHY_LINK_COUNT                          0x0000206c               
#define R_ANC_LINK_FSM_COUNTS                              0x00002070               
#define R_ANC_LINK_ECC_WATERMARK                           0x00002074               
#define R_ANC_LINK_ECC_CONFIG                              0x00002078      
#define R_ANC_LINK_LAST                                    0x00002078 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_LINK_CONFIG                                            
#define rANC_LINK_CONTROL                                           
#define rANC_LINK_STATUS                                            
#define rANC_LINK_CHIP_ENABLE                                       
#define rANC_LINK_READ_STATUS_CONFIG                                
#define rANC_LINK_COMMAND_ADDRESS_PULSE_TIMING                      
#define rANC_LINK_SDR_TIMING                                        
#define rANC_LINK_SDR_DATA_TIMING                                   
#define rANC_LINK_DDR_DATA_TIMING                                   
#define rANC_LINK_DDR_READ_TIMING                                   
#define rANC_LINK_DDR_WRITE_TIMING                                  
#define rANC_LINK_DQS_TIMING                                        
#define rANC_LINK_CMDQ_COUNT                                        
#define rANC_LINK_BYPASS_COUNT                                      
#define rANC_LINK_CMD_TIMEOUT                                       
#define rANC_LINK_CMDQ_INT_CODE                                     
#define rANC_LINK_TIMER                                             
#define rANC_LINK_CRC                                               
#define rANC_LINK_NPL_INTERFACE                                     
#define rANC_LINK_MACRO_STATUS                                      
#define rANC_LINK_BYPASS_MACRO_STATUS                               
#define rANC_LINK_NAND_STATUS                                       
#define rANC_LINK_DLFIFO_LINK_COUNT                                 
#define rANC_LINK_DLFIFO_COUNT                                      
#define rANC_LINK_PIO_LINK_COUNT                                    
#define rANC_LINK_PIO_COUNT                                         
#define rANC_LINK_PHY_COUNT                                         
#define rANC_LINK_PHY_LINK_COUNT                                    
#define rANC_LINK_FSM_COUNTS                                        
#define rANC_LINK_ECC_WATERMARK                                     
#define rANC_LINK_ECC_CONFIG                                    
#define rANC_LINK_LAST                                       
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_ECC_BASE                                     0x00002200
#define A_ANC_ECC_REG(REG)                                 (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_ECC_P_CHIEN_CLOCK_CONFIG                     0x00002200               
#define R_ANC_ECC_P_CONFIG                                 0x0000220c               
#define R_ANC_ECC_P_OFFSET1                                0x00002210               
#define R_ANC_ECC_P_BCH_DIRECTION_CNFG                     0x00002218               
#define R_ANC_ECC_P_PAGE_IDX                               0x0000221c               
#define R_ANC_ECC_P_PAGE_STRUCTURE                         0x00002220               
#define R_ANC_ECC_P_ECC_CLK_CNFG                           0x00002224               
#define R_ANC_ECC_P_OFFSET3                                0x00002228               
#define R_ANC_ECC_P_CHIEN_START_OFFSET                     0x0000222c               
#define R_ANC_ECC_P_CHIEN_EARLY_INTERRUPT                  0x00002230               
#define R_ANC_ECC_P_OFFSET4                                0x0000223c               
#define R_ANC_ECC_P_STATUS                                 0x000022a0               
#define R_ANC_ECC_P_BCH_DIR_CNTRS                          0x000022a4               
#define R_ANC_ECC_P_BMA_STATUS                             0x000022b4      
#define R_ANC_ECC_LAST                                     0x000022b4 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_ECC_P_CHIEN_CLOCK_CONFIG                               
#define rANC_ECC_P_CONFIG                                           
#define rANC_ECC_P_OFFSET1                                          
#define rANC_ECC_P_BCH_DIRECTION_CNFG                               
#define rANC_ECC_P_PAGE_IDX                                         
#define rANC_ECC_P_PAGE_STRUCTURE                                   
#define rANC_ECC_P_ECC_CLK_CNFG                                     
#define rANC_ECC_P_OFFSET3                                          
#define rANC_ECC_P_CHIEN_START_OFFSET                               
#define rANC_ECC_P_CHIEN_EARLY_INTERRUPT                            
#define rANC_ECC_P_OFFSET4                                          
#define rANC_ECC_P_STATUS                                           
#define rANC_ECC_P_BCH_DIR_CNTRS                                    
#define rANC_ECC_P_BMA_STATUS                                   
#define rANC_ECC_LAST                                        
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_MACRO_TABLE_BASE                             0x00003000
#define A_ANC_MACRO_TABLE_REG(REG)                         (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_MACRO_TABLE_COMMAND                          0x00003000      
#define R_ANC_MACRO_TABLE_LAST                             0x00003000 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_MACRO_TABLE_COMMAND                                
#define rANC_MACRO_TABLE_LAST                                
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_DMA_CMDQ_FIFO_DA_BASE                        0x00004000
#define A_ANC_DMA_CMDQ_FIFO_DA_REG(REG)                    (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_DMA_CMDQ_FIFO_DA_WORD                        0x00004000      
#define R_ANC_DMA_CMDQ_FIFO_DA_LAST                        0x00004000 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_DMA_CMDQ_FIFO_DA_WORD                              
#define rANC_DMA_CMDQ_FIFO_DA_LAST                           
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_LINK_CMDQ_FIFO_DA_BASE                       0x00004200
#define A_ANC_LINK_CMDQ_FIFO_DA_REG(REG)                   (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_LINK_CMDQ_FIFO_DA_WORD                       0x00004200      
#define R_ANC_LINK_CMDQ_FIFO_DA_LAST                       0x00004200 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_LINK_CMDQ_FIFO_DA_WORD                             
#define rANC_LINK_CMDQ_FIFO_DA_LAST                          
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_LINK_BYPASS_FIFO_DA_BASE                     0x00004400
#define A_ANC_LINK_BYPASS_FIFO_DA_REG(REG)                 (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_LINK_BYPASS_FIFO_DA_WORD                     0x00004400      
#define R_ANC_LINK_BYPASS_FIFO_DA_LAST                     0x00004400 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_LINK_BYPASS_FIFO_DA_WORD                           
#define rANC_LINK_BYPASS_FIFO_DA_LAST                        
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_PIO_READ_FIFO_DA_BASE                        0x00004500
#define A_ANC_PIO_READ_FIFO_DA_REG(REG)                    (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_PIO_READ_FIFO_DA_WORD                        0x00004500      
#define R_ANC_PIO_READ_FIFO_DA_LAST                        0x00004500 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_PIO_READ_FIFO_DA_WORD                              
#define rANC_PIO_READ_FIFO_DA_LAST                           
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_DMA_DEBUG_FIFO_BASE                          0x00004600
#define A_ANC_DMA_DEBUG_FIFO_REG(REG)                      (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_DMA_DEBUG_FIFO_COMMAND                       0x00004600      
#define R_ANC_DMA_DEBUG_FIFO_LAST                          0x00004600 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_DMA_DEBUG_FIFO_COMMAND                             
#define rANC_DMA_DEBUG_FIFO_LAST                             
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_LINK_DEBUG_FIFO_BASE                         0x00004700
#define A_ANC_LINK_DEBUG_FIFO_REG(REG)                     (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_LINK_DEBUG_FIFO_COMMAND                      0x00004700      
#define R_ANC_LINK_DEBUG_FIFO_LAST                         0x00004700 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_LINK_DEBUG_FIFO_COMMAND                            
#define rANC_LINK_DEBUG_FIFO_LAST                            
#endif        

/******************************************************************************/
/* Addresses */
/******************************************************************************/    
#define A_ANC_LINK_ECC_STATUS_FIFO_DA_BASE                 0x00004800
#define A_ANC_LINK_ECC_STATUS_FIFO_DA_REG(REG)             (REG) 

/******************************************************************************/
/* Registers */
/******************************************************************************/           
#define R_ANC_LINK_ECC_STATUS_FIFO_DA_WORD                 0x00004800      
#define R_ANC_LINK_ECC_STATUS_FIFO_DA_LAST                 0x00004800 

/******************************************************************************/
/* Registers2 */
/******************************************************************************/
#ifndef __ASSEMBLY__      
#define rANC_LINK_ECC_STATUS_FIFO_DA_WORD                       
#define rANC_LINK_ECC_STATUS_FIFO_DA_LAST                    
#endif 

/******************************************************************************/
/* Register Fields defines */
/******************************************************************************/          
#define S_ANC_CHAN_VERSION_MINOR_RELEASE                                          0  //Access: read-only
#define M_ANC_CHAN_VERSION_MINOR_RELEASE                                          _MM_MAKEMASK(8,S_ANC_CHAN_VERSION_MINOR_RELEASE)
#define V_ANC_CHAN_VERSION_MINOR_RELEASE(V)                                       _MM_MAKEVALUE((V),S_ANC_CHAN_VERSION_MINOR_RELEASE)
#define G_ANC_CHAN_VERSION_MINOR_RELEASE(V)                                       _MM_GETVALUE((V),S_ANC_CHAN_VERSION_MINOR_RELEASE,M_ANC_CHAN_VERSION_MINOR_RELEASE)
     
#define S_ANC_CHAN_VERSION_MAJOR_RELEASE                                          8  //Access: read-only
#define M_ANC_CHAN_VERSION_MAJOR_RELEASE                                          _MM_MAKEMASK(8,S_ANC_CHAN_VERSION_MAJOR_RELEASE)
#define V_ANC_CHAN_VERSION_MAJOR_RELEASE(V)                                       _MM_MAKEVALUE((V),S_ANC_CHAN_VERSION_MAJOR_RELEASE)
#define G_ANC_CHAN_VERSION_MAJOR_RELEASE(V)                                       _MM_GETVALUE((V),S_ANC_CHAN_VERSION_MAJOR_RELEASE,M_ANC_CHAN_VERSION_MAJOR_RELEASE)
     
#define S_ANC_CHAN_VERSION_VERSION                                                16  //Access: read-only
#define M_ANC_CHAN_VERSION_VERSION                                                _MM_MAKEMASK(8,S_ANC_CHAN_VERSION_VERSION)
#define V_ANC_CHAN_VERSION_VERSION(V)                                             _MM_MAKEVALUE((V),S_ANC_CHAN_VERSION_VERSION)
#define G_ANC_CHAN_VERSION_VERSION(V)                                             _MM_GETVALUE((V),S_ANC_CHAN_VERSION_VERSION,M_ANC_CHAN_VERSION_VERSION)
     
#define S_ANC_CHAN_VERSION_RSVD0                                                  24  //Access: read-as-zero
#define M_ANC_CHAN_VERSION_RSVD0                                                  _MM_MAKEMASK(8,S_ANC_CHAN_VERSION_RSVD0)
#define V_ANC_CHAN_VERSION_RSVD0(V)                                               _MM_MAKEVALUE((V),S_ANC_CHAN_VERSION_RSVD0)
#define G_ANC_CHAN_VERSION_RSVD0(V)                                               _MM_GETVALUE((V),S_ANC_CHAN_VERSION_RSVD0,M_ANC_CHAN_VERSION_RSVD0)

      
#define S_ANC_CHAN_CONFIG_AUTO_CLK_GATING_EN                                      0  //Access: read-write
#define M_ANC_CHAN_CONFIG_AUTO_CLK_GATING_EN                                      _MM_MAKEMASK(1,S_ANC_CHAN_CONFIG_AUTO_CLK_GATING_EN)
#define V_ANC_CHAN_CONFIG_AUTO_CLK_GATING_EN(V)                                   _MM_MAKEVALUE((V),S_ANC_CHAN_CONFIG_AUTO_CLK_GATING_EN)
#define G_ANC_CHAN_CONFIG_AUTO_CLK_GATING_EN(V)                                   _MM_GETVALUE((V),S_ANC_CHAN_CONFIG_AUTO_CLK_GATING_EN,M_ANC_CHAN_CONFIG_AUTO_CLK_GATING_EN)
     
#define S_ANC_CHAN_CONFIG_RSVD0                                                   1  //Access: read-as-zero
#define M_ANC_CHAN_CONFIG_RSVD0                                                   _MM_MAKEMASK(31,S_ANC_CHAN_CONFIG_RSVD0)
#define V_ANC_CHAN_CONFIG_RSVD0(V)                                                _MM_MAKEVALUE((V),S_ANC_CHAN_CONFIG_RSVD0)
#define G_ANC_CHAN_CONFIG_RSVD0(V)                                                _MM_GETVALUE((V),S_ANC_CHAN_CONFIG_RSVD0,M_ANC_CHAN_CONFIG_RSVD0)

      
#define S_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG                                        0  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG                                        _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG)
#define V_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG(V)                                     _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG)
#define G_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG(V)                                     _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG,M_ANC_CHAN_INT_STATUS_DMA_CMD_FLAG)
     
#define S_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG                                       1  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG                                       _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG)
#define V_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG(V)                                    _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG)
#define G_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG(V)                                    _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG,M_ANC_CHAN_INT_STATUS_LINK_CMD_FLAG)
     
#define S_ANC_CHAN_INT_STATUS_CHANNEL_STOPPED                                     2  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_CHANNEL_STOPPED                                     _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_CHANNEL_STOPPED)
#define V_ANC_CHAN_INT_STATUS_CHANNEL_STOPPED(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_CHANNEL_STOPPED)
#define G_ANC_CHAN_INT_STATUS_CHANNEL_STOPPED(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_CHANNEL_STOPPED,M_ANC_CHAN_INT_STATUS_CHANNEL_STOPPED)
     
#define S_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_LOW                                   3  //Access: read-only
#define M_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_LOW                                   _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_LOW)
#define V_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_LOW(V)                                _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_LOW)
#define G_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_LOW(V)                                _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_LOW,M_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_LOW)
     
#define S_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_LOW                                  4  //Access: read-only
#define M_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_LOW                                  _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_LOW)
#define V_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_LOW(V)                               _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_LOW)
#define G_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_LOW(V)                               _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_LOW,M_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_LOW)
     
#define S_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_LOW                                5  //Access: read-only
#define M_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_LOW                                _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_LOW)
#define V_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_LOW(V)                             _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_LOW)
#define G_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_LOW(V)                             _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_LOW,M_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_LOW)
     
#define S_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_HIGH                             6  //Access: read-only
#define M_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_HIGH                             _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_HIGH)
#define V_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_HIGH(V)                          _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_HIGH)
#define G_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_HIGH(V)                          _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_HIGH,M_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_HIGH)
     
#define S_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_OVERFLOW                              7  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_OVERFLOW                              _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_OVERFLOW)
#define V_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_OVERFLOW(V)                           _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_OVERFLOW)
#define G_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_OVERFLOW(V)                           _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_OVERFLOW,M_ANC_CHAN_INT_STATUS_DMA_CMDQ_FIFO_OVERFLOW)
     
#define S_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_OVERFLOW                             8  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_OVERFLOW                             _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_OVERFLOW)
#define V_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_OVERFLOW(V)                          _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_OVERFLOW)
#define G_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_OVERFLOW(V)                          _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_OVERFLOW,M_ANC_CHAN_INT_STATUS_LINK_CMDQ_FIFO_OVERFLOW)
     
#define S_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_OVERFLOW                           9  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_OVERFLOW                           _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_OVERFLOW)
#define V_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_OVERFLOW(V)                        _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_OVERFLOW)
#define G_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_OVERFLOW(V)                        _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_OVERFLOW,M_ANC_CHAN_INT_STATUS_LINK_BYPASS_FIFO_OVERFLOW)
     
#define S_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_UNDERFLOW                        10  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_UNDERFLOW                        _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_UNDERFLOW)
#define V_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_UNDERFLOW(V)                     _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_UNDERFLOW)
#define G_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_UNDERFLOW(V)                     _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_UNDERFLOW,M_ANC_CHAN_INT_STATUS_LINK_PIO_READ_FIFO_UNDERFLOW)
     
#define S_ANC_CHAN_INT_STATUS_CRC_ERR                                             11  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_CRC_ERR                                             _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_CRC_ERR)
#define V_ANC_CHAN_INT_STATUS_CRC_ERR(V)                                          _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_CRC_ERR)
#define G_ANC_CHAN_INT_STATUS_CRC_ERR(V)                                          _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_CRC_ERR,M_ANC_CHAN_INT_STATUS_CRC_ERR)
     
#define S_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE                            12  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE                            _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE)
#define V_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE(V)                         _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE)
#define G_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE(V)                         _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE,M_ANC_CHAN_INT_STATUS_READ_STATUS_ERR_RESPONSE)
     
#define S_ANC_CHAN_INT_STATUS_INVALID_DMA_COMMAND                                 13  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_INVALID_DMA_COMMAND                                 _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_INVALID_DMA_COMMAND)
#define V_ANC_CHAN_INT_STATUS_INVALID_DMA_COMMAND(V)                              _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_INVALID_DMA_COMMAND)
#define G_ANC_CHAN_INT_STATUS_INVALID_DMA_COMMAND(V)                              _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_INVALID_DMA_COMMAND,M_ANC_CHAN_INT_STATUS_INVALID_DMA_COMMAND)
     
#define S_ANC_CHAN_INT_STATUS_INVALID_LINK_COMMAND                                14  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_INVALID_LINK_COMMAND                                _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_INVALID_LINK_COMMAND)
#define V_ANC_CHAN_INT_STATUS_INVALID_LINK_COMMAND(V)                             _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_INVALID_LINK_COMMAND)
#define G_ANC_CHAN_INT_STATUS_INVALID_LINK_COMMAND(V)                             _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_INVALID_LINK_COMMAND,M_ANC_CHAN_INT_STATUS_INVALID_LINK_COMMAND)
     
#define S_ANC_CHAN_INT_STATUS_AXI_RESPONSE_NOT_OKAY                               15  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_AXI_RESPONSE_NOT_OKAY                               _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_AXI_RESPONSE_NOT_OKAY)
#define V_ANC_CHAN_INT_STATUS_AXI_RESPONSE_NOT_OKAY(V)                            _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_AXI_RESPONSE_NOT_OKAY)
#define G_ANC_CHAN_INT_STATUS_AXI_RESPONSE_NOT_OKAY(V)                            _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_AXI_RESPONSE_NOT_OKAY,M_ANC_CHAN_INT_STATUS_AXI_RESPONSE_NOT_OKAY)
     
#define S_ANC_CHAN_INT_STATUS_AES_ERR                                             16  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_AES_ERR                                             _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_AES_ERR)
#define V_ANC_CHAN_INT_STATUS_AES_ERR(V)                                          _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_AES_ERR)
#define G_ANC_CHAN_INT_STATUS_AES_ERR(V)                                          _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_AES_ERR,M_ANC_CHAN_INT_STATUS_AES_ERR)
     
#define S_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT                                     17  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT                                     _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT)
#define V_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT)
#define G_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT,M_ANC_CHAN_INT_STATUS_DMA_CMD_TIMEOUT)
     
#define S_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT                                    18  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT                                    _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT)
#define V_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT(V)                                 _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT)
#define G_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT(V)                                 _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT,M_ANC_CHAN_INT_STATUS_LINK_CMD_TIMEOUT)
     
#define S_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_HIGH                           19  //Access: read-only
#define M_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_HIGH                           _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_HIGH)
#define V_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_HIGH(V)                        _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_HIGH)
#define G_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_HIGH(V)                        _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_HIGH,M_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_HIGH)
     
#define S_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_UNDERFLOW                      20  //Access: write-once-clear
#define M_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_UNDERFLOW                      _MM_MAKEMASK(1,S_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_UNDERFLOW)
#define V_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_UNDERFLOW(V)                   _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_UNDERFLOW)
#define G_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_UNDERFLOW(V)                   _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_UNDERFLOW,M_ANC_CHAN_INT_STATUS_LINK_ECC_STATUS_FIFO_UNDERFLOW)
     
#define S_ANC_CHAN_INT_STATUS_RSVD0                                               21  //Access: read-as-zero
#define M_ANC_CHAN_INT_STATUS_RSVD0                                               _MM_MAKEMASK(11,S_ANC_CHAN_INT_STATUS_RSVD0)
#define V_ANC_CHAN_INT_STATUS_RSVD0(V)                                            _MM_MAKEVALUE((V),S_ANC_CHAN_INT_STATUS_RSVD0)
#define G_ANC_CHAN_INT_STATUS_RSVD0(V)                                            _MM_GETVALUE((V),S_ANC_CHAN_INT_STATUS_RSVD0,M_ANC_CHAN_INT_STATUS_RSVD0)

      
#define S_ANC_CHAN_INT_ENABLE_DMA_CMD_FLAG                                        0  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_DMA_CMD_FLAG                                        _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_DMA_CMD_FLAG)
#define V_ANC_CHAN_INT_ENABLE_DMA_CMD_FLAG(V)                                     _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_DMA_CMD_FLAG)
#define G_ANC_CHAN_INT_ENABLE_DMA_CMD_FLAG(V)                                     _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_DMA_CMD_FLAG,M_ANC_CHAN_INT_ENABLE_DMA_CMD_FLAG)
     
#define S_ANC_CHAN_INT_ENABLE_LINK_CMD_FLAG                                       1  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_LINK_CMD_FLAG                                       _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_LINK_CMD_FLAG)
#define V_ANC_CHAN_INT_ENABLE_LINK_CMD_FLAG(V)                                    _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_CMD_FLAG)
#define G_ANC_CHAN_INT_ENABLE_LINK_CMD_FLAG(V)                                    _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_CMD_FLAG,M_ANC_CHAN_INT_ENABLE_LINK_CMD_FLAG)
     
#define S_ANC_CHAN_INT_ENABLE_CHANNEL_STOPPED                                     2  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_CHANNEL_STOPPED                                     _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_CHANNEL_STOPPED)
#define V_ANC_CHAN_INT_ENABLE_CHANNEL_STOPPED(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_CHANNEL_STOPPED)
#define G_ANC_CHAN_INT_ENABLE_CHANNEL_STOPPED(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_CHANNEL_STOPPED,M_ANC_CHAN_INT_ENABLE_CHANNEL_STOPPED)
     
#define S_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_LOW                                   3  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_LOW                                   _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_LOW)
#define V_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_LOW(V)                                _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_LOW)
#define G_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_LOW(V)                                _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_LOW,M_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_LOW)
     
#define S_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_LOW                                  4  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_LOW                                  _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_LOW)
#define V_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_LOW(V)                               _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_LOW)
#define G_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_LOW(V)                               _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_LOW,M_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_LOW)
     
#define S_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_LOW                                5  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_LOW                                _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_LOW)
#define V_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_LOW(V)                             _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_LOW)
#define G_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_LOW(V)                             _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_LOW,M_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_LOW)
     
#define S_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_HIGH                             6  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_HIGH                             _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_HIGH)
#define V_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_HIGH(V)                          _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_HIGH)
#define G_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_HIGH(V)                          _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_HIGH,M_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_HIGH)
     
#define S_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_OVERFLOW                              7  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_OVERFLOW                              _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_OVERFLOW)
#define V_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_OVERFLOW(V)                           _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_OVERFLOW)
#define G_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_OVERFLOW(V)                           _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_OVERFLOW,M_ANC_CHAN_INT_ENABLE_DMA_CMDQ_FIFO_OVERFLOW)
     
#define S_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_OVERFLOW                             8  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_OVERFLOW                             _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_OVERFLOW)
#define V_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_OVERFLOW(V)                          _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_OVERFLOW)
#define G_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_OVERFLOW(V)                          _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_OVERFLOW,M_ANC_CHAN_INT_ENABLE_LINK_CMDQ_FIFO_OVERFLOW)
     
#define S_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_OVERFLOW                           9  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_OVERFLOW                           _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_OVERFLOW)
#define V_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_OVERFLOW(V)                        _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_OVERFLOW)
#define G_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_OVERFLOW(V)                        _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_OVERFLOW,M_ANC_CHAN_INT_ENABLE_LINK_BYPASS_FIFO_OVERFLOW)
     
#define S_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_UNDERFLOW                        10  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_UNDERFLOW                        _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_UNDERFLOW)
#define V_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_UNDERFLOW(V)                     _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_UNDERFLOW)
#define G_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_UNDERFLOW(V)                     _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_UNDERFLOW,M_ANC_CHAN_INT_ENABLE_LINK_PIO_READ_FIFO_UNDERFLOW)
     
#define S_ANC_CHAN_INT_ENABLE_CRC_ERR                                             11  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_CRC_ERR                                             _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_CRC_ERR)
#define V_ANC_CHAN_INT_ENABLE_CRC_ERR(V)                                          _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_CRC_ERR)
#define G_ANC_CHAN_INT_ENABLE_CRC_ERR(V)                                          _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_CRC_ERR,M_ANC_CHAN_INT_ENABLE_CRC_ERR)
     
#define S_ANC_CHAN_INT_ENABLE_READ_STATUS_ERR_RESPONSE                            12  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_READ_STATUS_ERR_RESPONSE                            _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_READ_STATUS_ERR_RESPONSE)
#define V_ANC_CHAN_INT_ENABLE_READ_STATUS_ERR_RESPONSE(V)                         _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_READ_STATUS_ERR_RESPONSE)
#define G_ANC_CHAN_INT_ENABLE_READ_STATUS_ERR_RESPONSE(V)                         _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_READ_STATUS_ERR_RESPONSE,M_ANC_CHAN_INT_ENABLE_READ_STATUS_ERR_RESPONSE)
     
#define S_ANC_CHAN_INT_ENABLE_INVALID_DMA_COMMAND                                 13  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_INVALID_DMA_COMMAND                                 _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_INVALID_DMA_COMMAND)
#define V_ANC_CHAN_INT_ENABLE_INVALID_DMA_COMMAND(V)                              _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_INVALID_DMA_COMMAND)
#define G_ANC_CHAN_INT_ENABLE_INVALID_DMA_COMMAND(V)                              _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_INVALID_DMA_COMMAND,M_ANC_CHAN_INT_ENABLE_INVALID_DMA_COMMAND)
     
#define S_ANC_CHAN_INT_ENABLE_INVALID_LINK_COMMAND                                14  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_INVALID_LINK_COMMAND                                _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_INVALID_LINK_COMMAND)
#define V_ANC_CHAN_INT_ENABLE_INVALID_LINK_COMMAND(V)                             _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_INVALID_LINK_COMMAND)
#define G_ANC_CHAN_INT_ENABLE_INVALID_LINK_COMMAND(V)                             _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_INVALID_LINK_COMMAND,M_ANC_CHAN_INT_ENABLE_INVALID_LINK_COMMAND)
     
#define S_ANC_CHAN_INT_ENABLE_AXI_RESPONSE_NOT_OKAY                               15  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_AXI_RESPONSE_NOT_OKAY                               _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_AXI_RESPONSE_NOT_OKAY)
#define V_ANC_CHAN_INT_ENABLE_AXI_RESPONSE_NOT_OKAY(V)                            _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_AXI_RESPONSE_NOT_OKAY)
#define G_ANC_CHAN_INT_ENABLE_AXI_RESPONSE_NOT_OKAY(V)                            _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_AXI_RESPONSE_NOT_OKAY,M_ANC_CHAN_INT_ENABLE_AXI_RESPONSE_NOT_OKAY)
     
#define S_ANC_CHAN_INT_ENABLE_AES_ERR                                             16  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_AES_ERR                                             _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_AES_ERR)
#define V_ANC_CHAN_INT_ENABLE_AES_ERR(V)                                          _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_AES_ERR)
#define G_ANC_CHAN_INT_ENABLE_AES_ERR(V)                                          _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_AES_ERR,M_ANC_CHAN_INT_ENABLE_AES_ERR)
     
#define S_ANC_CHAN_INT_ENABLE_DMA_CMD_TIMEOUT                                     17  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_DMA_CMD_TIMEOUT                                     _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_DMA_CMD_TIMEOUT)
#define V_ANC_CHAN_INT_ENABLE_DMA_CMD_TIMEOUT(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_DMA_CMD_TIMEOUT)
#define G_ANC_CHAN_INT_ENABLE_DMA_CMD_TIMEOUT(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_DMA_CMD_TIMEOUT,M_ANC_CHAN_INT_ENABLE_DMA_CMD_TIMEOUT)
     
#define S_ANC_CHAN_INT_ENABLE_LINK_CMD_TIMEOUT                                    18  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_LINK_CMD_TIMEOUT                                    _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_LINK_CMD_TIMEOUT)
#define V_ANC_CHAN_INT_ENABLE_LINK_CMD_TIMEOUT(V)                                 _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_CMD_TIMEOUT)
#define G_ANC_CHAN_INT_ENABLE_LINK_CMD_TIMEOUT(V)                                 _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_CMD_TIMEOUT,M_ANC_CHAN_INT_ENABLE_LINK_CMD_TIMEOUT)
     
#define S_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_HIGH                           19  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_HIGH                           _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_HIGH)
#define V_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_HIGH(V)                        _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_HIGH)
#define G_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_HIGH(V)                        _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_HIGH,M_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_HIGH)
     
#define S_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_UNDERFLOW                      20  //Access: read-write
#define M_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_UNDERFLOW                      _MM_MAKEMASK(1,S_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_UNDERFLOW)
#define V_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_UNDERFLOW(V)                   _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_UNDERFLOW)
#define G_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_UNDERFLOW(V)                   _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_UNDERFLOW,M_ANC_CHAN_INT_ENABLE_LINK_ECC_STATUS_FIFO_UNDERFLOW)
     
#define S_ANC_CHAN_INT_ENABLE_RSVD0                                               21  //Access: read-as-zero
#define M_ANC_CHAN_INT_ENABLE_RSVD0                                               _MM_MAKEMASK(11,S_ANC_CHAN_INT_ENABLE_RSVD0)
#define V_ANC_CHAN_INT_ENABLE_RSVD0(V)                                            _MM_MAKEVALUE((V),S_ANC_CHAN_INT_ENABLE_RSVD0)
#define G_ANC_CHAN_INT_ENABLE_RSVD0(V)                                            _MM_GETVALUE((V),S_ANC_CHAN_INT_ENABLE_RSVD0,M_ANC_CHAN_INT_ENABLE_RSVD0)

      
#define S_ANC_CHAN_DMA_WATERMARKS_DMA_CMDQ_FIFO_LOW                               0  //Access: read-write
#define M_ANC_CHAN_DMA_WATERMARKS_DMA_CMDQ_FIFO_LOW                               _MM_MAKEMASK(7,S_ANC_CHAN_DMA_WATERMARKS_DMA_CMDQ_FIFO_LOW)
#define V_ANC_CHAN_DMA_WATERMARKS_DMA_CMDQ_FIFO_LOW(V)                            _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_WATERMARKS_DMA_CMDQ_FIFO_LOW)
#define G_ANC_CHAN_DMA_WATERMARKS_DMA_CMDQ_FIFO_LOW(V)                            _MM_GETVALUE((V),S_ANC_CHAN_DMA_WATERMARKS_DMA_CMDQ_FIFO_LOW,M_ANC_CHAN_DMA_WATERMARKS_DMA_CMDQ_FIFO_LOW)
     
#define S_ANC_CHAN_DMA_WATERMARKS_RSVD0                                           7  //Access: read-as-zero
#define M_ANC_CHAN_DMA_WATERMARKS_RSVD0                                           _MM_MAKEMASK(25,S_ANC_CHAN_DMA_WATERMARKS_RSVD0)
#define V_ANC_CHAN_DMA_WATERMARKS_RSVD0(V)                                        _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_WATERMARKS_RSVD0)
#define G_ANC_CHAN_DMA_WATERMARKS_RSVD0(V)                                        _MM_GETVALUE((V),S_ANC_CHAN_DMA_WATERMARKS_RSVD0,M_ANC_CHAN_DMA_WATERMARKS_RSVD0)

      
#define S_ANC_CHAN_LINK_WATERMARKS_LINK_CMDQ_FIFO_LOW                             0  //Access: read-write
#define M_ANC_CHAN_LINK_WATERMARKS_LINK_CMDQ_FIFO_LOW                             _MM_MAKEMASK(8,S_ANC_CHAN_LINK_WATERMARKS_LINK_CMDQ_FIFO_LOW)
#define V_ANC_CHAN_LINK_WATERMARKS_LINK_CMDQ_FIFO_LOW(V)                          _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_LINK_CMDQ_FIFO_LOW)
#define G_ANC_CHAN_LINK_WATERMARKS_LINK_CMDQ_FIFO_LOW(V)                          _MM_GETVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_LINK_CMDQ_FIFO_LOW,M_ANC_CHAN_LINK_WATERMARKS_LINK_CMDQ_FIFO_LOW)
     
#define S_ANC_CHAN_LINK_WATERMARKS_RSVD0                                          8  //Access: read-as-zero
#define M_ANC_CHAN_LINK_WATERMARKS_RSVD0                                          _MM_MAKEMASK(8,S_ANC_CHAN_LINK_WATERMARKS_RSVD0)
#define V_ANC_CHAN_LINK_WATERMARKS_RSVD0(V)                                       _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_RSVD0)
#define G_ANC_CHAN_LINK_WATERMARKS_RSVD0(V)                                       _MM_GETVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_RSVD0,M_ANC_CHAN_LINK_WATERMARKS_RSVD0)
     
#define S_ANC_CHAN_LINK_WATERMARKS_LINK_BYPASS_FIFO_LOW                           16  //Access: read-write
#define M_ANC_CHAN_LINK_WATERMARKS_LINK_BYPASS_FIFO_LOW                           _MM_MAKEMASK(4,S_ANC_CHAN_LINK_WATERMARKS_LINK_BYPASS_FIFO_LOW)
#define V_ANC_CHAN_LINK_WATERMARKS_LINK_BYPASS_FIFO_LOW(V)                        _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_LINK_BYPASS_FIFO_LOW)
#define G_ANC_CHAN_LINK_WATERMARKS_LINK_BYPASS_FIFO_LOW(V)                        _MM_GETVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_LINK_BYPASS_FIFO_LOW,M_ANC_CHAN_LINK_WATERMARKS_LINK_BYPASS_FIFO_LOW)
     
#define S_ANC_CHAN_LINK_WATERMARKS_RSVD1                                          20  //Access: read-as-zero
#define M_ANC_CHAN_LINK_WATERMARKS_RSVD1                                          _MM_MAKEMASK(4,S_ANC_CHAN_LINK_WATERMARKS_RSVD1)
#define V_ANC_CHAN_LINK_WATERMARKS_RSVD1(V)                                       _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_RSVD1)
#define G_ANC_CHAN_LINK_WATERMARKS_RSVD1(V)                                       _MM_GETVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_RSVD1,M_ANC_CHAN_LINK_WATERMARKS_RSVD1)
     
#define S_ANC_CHAN_LINK_WATERMARKS_LINK_PIO_READ_FIFO_HIGH                        24  //Access: read-write
#define M_ANC_CHAN_LINK_WATERMARKS_LINK_PIO_READ_FIFO_HIGH                        _MM_MAKEMASK(7,S_ANC_CHAN_LINK_WATERMARKS_LINK_PIO_READ_FIFO_HIGH)
#define V_ANC_CHAN_LINK_WATERMARKS_LINK_PIO_READ_FIFO_HIGH(V)                     _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_LINK_PIO_READ_FIFO_HIGH)
#define G_ANC_CHAN_LINK_WATERMARKS_LINK_PIO_READ_FIFO_HIGH(V)                     _MM_GETVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_LINK_PIO_READ_FIFO_HIGH,M_ANC_CHAN_LINK_WATERMARKS_LINK_PIO_READ_FIFO_HIGH)
     
#define S_ANC_CHAN_LINK_WATERMARKS_RSVD2                                          31  //Access: read-as-zero
#define M_ANC_CHAN_LINK_WATERMARKS_RSVD2                                          _MM_MAKEMASK(1,S_ANC_CHAN_LINK_WATERMARKS_RSVD2)
#define V_ANC_CHAN_LINK_WATERMARKS_RSVD2(V)                                       _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_RSVD2)
#define G_ANC_CHAN_LINK_WATERMARKS_RSVD2(V)                                       _MM_GETVALUE((V),S_ANC_CHAN_LINK_WATERMARKS_RSVD2,M_ANC_CHAN_LINK_WATERMARKS_RSVD2)

      
#define S_ANC_CHAN_STATUS_WATERMARKS_LINK_ECC_STATUS_FIFO_HIGH                    0  //Access: read-write
#define M_ANC_CHAN_STATUS_WATERMARKS_LINK_ECC_STATUS_FIFO_HIGH                    _MM_MAKEMASK(7,S_ANC_CHAN_STATUS_WATERMARKS_LINK_ECC_STATUS_FIFO_HIGH)
#define V_ANC_CHAN_STATUS_WATERMARKS_LINK_ECC_STATUS_FIFO_HIGH(V)                 _MM_MAKEVALUE((V),S_ANC_CHAN_STATUS_WATERMARKS_LINK_ECC_STATUS_FIFO_HIGH)
#define G_ANC_CHAN_STATUS_WATERMARKS_LINK_ECC_STATUS_FIFO_HIGH(V)                 _MM_GETVALUE((V),S_ANC_CHAN_STATUS_WATERMARKS_LINK_ECC_STATUS_FIFO_HIGH,M_ANC_CHAN_STATUS_WATERMARKS_LINK_ECC_STATUS_FIFO_HIGH)
     
#define S_ANC_CHAN_STATUS_WATERMARKS_RSVD0                                        7  //Access: read-as-zero
#define M_ANC_CHAN_STATUS_WATERMARKS_RSVD0                                        _MM_MAKEMASK(25,S_ANC_CHAN_STATUS_WATERMARKS_RSVD0)
#define V_ANC_CHAN_STATUS_WATERMARKS_RSVD0(V)                                     _MM_MAKEVALUE((V),S_ANC_CHAN_STATUS_WATERMARKS_RSVD0)
#define G_ANC_CHAN_STATUS_WATERMARKS_RSVD0(V)                                     _MM_GETVALUE((V),S_ANC_CHAN_STATUS_WATERMARKS_RSVD0,M_ANC_CHAN_STATUS_WATERMARKS_RSVD0)

      
#define S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LOW                                       0  //Access: read-only
#define M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LOW                                       _MM_MAKEMASK(1,S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LOW)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LOW(V)                                    _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LOW)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LOW(V)                                    _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LOW,M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LOW)
     
#define S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_FULL                                      1  //Access: read-only
#define M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_FULL                                      _MM_MAKEMASK(1,S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_FULL)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_FULL(V)                                   _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_FULL)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_FULL(V)                                   _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_FULL,M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_FULL)
     
#define S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_OVERFLOW                                  2  //Access: read-only
#define M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_OVERFLOW                                  _MM_MAKEMASK(1,S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_OVERFLOW)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_OVERFLOW(V)                               _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_OVERFLOW)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_OVERFLOW(V)                               _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_OVERFLOW,M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_OVERFLOW)
     
#define S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD0                                     3  //Access: read-as-zero
#define M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD0                                     _MM_MAKEMASK(5,S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD0)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD0(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD0)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD0(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD0,M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD0)
     
#define S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LEVEL                                     8  //Access: read-only
#define M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LEVEL                                     _MM_MAKEMASK(7,S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LEVEL)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LEVEL(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LEVEL)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LEVEL(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LEVEL,M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_LEVEL)
     
#define S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD1                                     15  //Access: read-as-zero
#define M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD1                                     _MM_MAKEMASK(1,S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD1)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD1(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD1)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD1(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD1,M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD1)
     
#define S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_READ_POINTER                              16  //Access: read-only
#define M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_READ_POINTER                              _MM_MAKEMASK(6,S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_READ_POINTER)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_READ_POINTER(V)                           _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_READ_POINTER)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_READ_POINTER(V)                           _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_READ_POINTER,M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_READ_POINTER)
     
#define S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD2                                     22  //Access: read-as-zero
#define M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD2                                     _MM_MAKEMASK(2,S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD2)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD2(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD2)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD2(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD2,M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD2)
     
#define S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_WRITE_POINTER                             24  //Access: read-only
#define M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_WRITE_POINTER                             _MM_MAKEMASK(6,S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_WRITE_POINTER)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_WRITE_POINTER(V)                          _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_WRITE_POINTER)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_WRITE_POINTER(V)                          _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_WRITE_POINTER,M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_WRITE_POINTER)
     
#define S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD3                                     30  //Access: read-as-zero
#define M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD3                                     _MM_MAKEMASK(2,S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD3)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD3(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD3)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD3(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD3,M_ANC_CHAN_DMA_CMDQ_FIFO_STATUS_RSVD3)

      
#define S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LOW                                      0  //Access: read-only
#define M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LOW                                      _MM_MAKEMASK(1,S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LOW)
#define V_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LOW(V)                                   _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LOW)
#define G_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LOW(V)                                   _MM_GETVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LOW,M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LOW)
     
#define S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_FULL                                     1  //Access: read-only
#define M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_FULL                                     _MM_MAKEMASK(1,S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_FULL)
#define V_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_FULL(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_FULL)
#define G_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_FULL(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_FULL,M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_FULL)
     
#define S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_OVERFLOW                                 2  //Access: read-only
#define M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_OVERFLOW                                 _MM_MAKEMASK(1,S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_OVERFLOW)
#define V_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_OVERFLOW(V)                              _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_OVERFLOW)
#define G_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_OVERFLOW(V)                              _MM_GETVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_OVERFLOW,M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_OVERFLOW)
     
#define S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD0                                    3  //Access: read-as-zero
#define M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD0                                    _MM_MAKEMASK(5,S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD0)
#define V_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD0(V)                                 _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD0)
#define G_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD0(V)                                 _MM_GETVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD0,M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD0)
     
#define S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LEVEL                                    8  //Access: read-only
#define M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LEVEL                                    _MM_MAKEMASK(8,S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LEVEL)
#define V_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LEVEL(V)                                 _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LEVEL)
#define G_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LEVEL(V)                                 _MM_GETVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LEVEL,M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_LEVEL)
     
#define S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_READ_POINTER                             16  //Access: read-only
#define M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_READ_POINTER                             _MM_MAKEMASK(7,S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_READ_POINTER)
#define V_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_READ_POINTER(V)                          _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_READ_POINTER)
#define G_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_READ_POINTER(V)                          _MM_GETVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_READ_POINTER,M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_READ_POINTER)
     
#define S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD1                                    23  //Access: read-as-zero
#define M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD1                                    _MM_MAKEMASK(1,S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD1)
#define V_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD1(V)                                 _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD1)
#define G_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD1(V)                                 _MM_GETVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD1,M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD1)
     
#define S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_WRITE_POINTER                            24  //Access: read-only
#define M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_WRITE_POINTER                            _MM_MAKEMASK(7,S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_WRITE_POINTER)
#define V_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_WRITE_POINTER(V)                         _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_WRITE_POINTER)
#define G_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_WRITE_POINTER(V)                         _MM_GETVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_WRITE_POINTER,M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_WRITE_POINTER)
     
#define S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD2                                    31  //Access: read-as-zero
#define M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD2                                    _MM_MAKEMASK(1,S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD2)
#define V_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD2(V)                                 _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD2)
#define G_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD2(V)                                 _MM_GETVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD2,M_ANC_CHAN_LINK_CMDQ_FIFO_STATUS_RSVD2)

      
#define S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LOW                                    0  //Access: read-only
#define M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LOW                                    _MM_MAKEMASK(1,S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LOW)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LOW(V)                                 _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LOW)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LOW(V)                                 _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LOW,M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LOW)
     
#define S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_FULL                                   1  //Access: read-only
#define M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_FULL                                   _MM_MAKEMASK(1,S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_FULL)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_FULL(V)                                _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_FULL)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_FULL(V)                                _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_FULL,M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_FULL)
     
#define S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_OVERFLOW                               2  //Access: read-only
#define M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_OVERFLOW                               _MM_MAKEMASK(1,S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_OVERFLOW)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_OVERFLOW(V)                            _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_OVERFLOW)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_OVERFLOW(V)                            _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_OVERFLOW,M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_OVERFLOW)
     
#define S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD0                                  3  //Access: read-as-zero
#define M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD0                                  _MM_MAKEMASK(5,S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD0)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD0(V)                               _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD0)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD0(V)                               _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD0,M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD0)
     
#define S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LEVEL                                  8  //Access: read-only
#define M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LEVEL                                  _MM_MAKEMASK(4,S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LEVEL)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LEVEL(V)                               _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LEVEL)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LEVEL(V)                               _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LEVEL,M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_LEVEL)
     
#define S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD1                                  12  //Access: read-as-zero
#define M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD1                                  _MM_MAKEMASK(4,S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD1)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD1(V)                               _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD1)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD1(V)                               _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD1,M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD1)
     
#define S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_READ_POINTER                           16  //Access: read-only
#define M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_READ_POINTER                           _MM_MAKEMASK(3,S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_READ_POINTER)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_READ_POINTER(V)                        _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_READ_POINTER)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_READ_POINTER(V)                        _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_READ_POINTER,M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_READ_POINTER)
     
#define S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD2                                  19  //Access: read-as-zero
#define M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD2                                  _MM_MAKEMASK(5,S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD2)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD2(V)                               _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD2)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD2(V)                               _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD2,M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD2)
     
#define S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_WRITE_POINTER                          24  //Access: read-only
#define M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_WRITE_POINTER                          _MM_MAKEMASK(3,S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_WRITE_POINTER)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_WRITE_POINTER(V)                       _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_WRITE_POINTER)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_WRITE_POINTER(V)                       _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_WRITE_POINTER,M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_WRITE_POINTER)
     
#define S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD3                                  27  //Access: read-as-zero
#define M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD3                                  _MM_MAKEMASK(5,S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD3)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD3(V)                               _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD3)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD3(V)                               _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD3,M_ANC_CHAN_LINK_BYPASS_FIFO_STATUS_RSVD3)

      
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_HIGH                                 0  //Access: read-only
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_HIGH                                 _MM_MAKEMASK(1,S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_HIGH)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_HIGH(V)                              _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_HIGH)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_HIGH(V)                              _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_HIGH,M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_HIGH)
     
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_EMPTY                                1  //Access: read-only
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_EMPTY                                _MM_MAKEMASK(1,S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_EMPTY)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_EMPTY(V)                             _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_EMPTY)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_EMPTY(V)                             _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_EMPTY,M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_EMPTY)
     
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_UNDERFLOW                            2  //Access: read-only
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_UNDERFLOW                            _MM_MAKEMASK(1,S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_UNDERFLOW)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_UNDERFLOW(V)                         _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_UNDERFLOW)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_UNDERFLOW(V)                         _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_UNDERFLOW,M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_UNDERFLOW)
     
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD0                                3  //Access: read-as-zero
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD0                                _MM_MAKEMASK(5,S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD0)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD0(V)                             _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD0)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD0(V)                             _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD0,M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD0)
     
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_LEVEL                                8  //Access: read-only
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_LEVEL                                _MM_MAKEMASK(7,S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_LEVEL)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_LEVEL(V)                             _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_LEVEL)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_LEVEL(V)                             _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_LEVEL,M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_LEVEL)
     
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD1                                15  //Access: read-as-zero
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD1                                _MM_MAKEMASK(1,S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD1)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD1(V)                             _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD1)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD1(V)                             _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD1,M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD1)
     
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_READ_POINTER                         16  //Access: read-only
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_READ_POINTER                         _MM_MAKEMASK(6,S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_READ_POINTER)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_READ_POINTER(V)                      _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_READ_POINTER)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_READ_POINTER(V)                      _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_READ_POINTER,M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_READ_POINTER)
     
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD2                                22  //Access: read-as-zero
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD2                                _MM_MAKEMASK(2,S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD2)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD2(V)                             _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD2)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD2(V)                             _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD2,M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD2)
     
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_WRITE_POINTER                        24  //Access: read-only
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_WRITE_POINTER                        _MM_MAKEMASK(6,S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_WRITE_POINTER)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_WRITE_POINTER(V)                     _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_WRITE_POINTER)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_WRITE_POINTER(V)                     _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_WRITE_POINTER,M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_WRITE_POINTER)
     
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD3                                30  //Access: read-as-zero
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD3                                _MM_MAKEMASK(2,S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD3)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD3(V)                             _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD3)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD3(V)                             _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD3,M_ANC_CHAN_LINK_PIO_READ_FIFO_STATUS_RSVD3)

      
#define S_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_WRITE_POINTER                            0  //Access: read-only
#define M_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_WRITE_POINTER                            _MM_MAKEMASK(4,S_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_WRITE_POINTER)
#define V_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_WRITE_POINTER(V)                         _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_WRITE_POINTER)
#define G_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_WRITE_POINTER(V)                         _MM_GETVALUE((V),S_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_WRITE_POINTER,M_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_WRITE_POINTER)
     
#define S_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_RSVD0                                    4  //Access: read-as-zero
#define M_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_RSVD0                                    _MM_MAKEMASK(28,S_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_RSVD0)
#define V_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_RSVD0(V)                                 _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_RSVD0)
#define G_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_RSVD0(V)                                 _MM_GETVALUE((V),S_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_RSVD0,M_ANC_CHAN_DMA_DEBUG_FIFO_STATUS_RSVD0)

      
#define S_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_WRITE_POINTER                           0  //Access: read-only
#define M_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_WRITE_POINTER                           _MM_MAKEMASK(5,S_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_WRITE_POINTER)
#define V_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_WRITE_POINTER(V)                        _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_WRITE_POINTER)
#define G_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_WRITE_POINTER(V)                        _MM_GETVALUE((V),S_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_WRITE_POINTER,M_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_WRITE_POINTER)
     
#define S_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_RSVD0                                   5  //Access: read-as-zero
#define M_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_RSVD0                                   _MM_MAKEMASK(27,S_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_RSVD0)
#define V_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_RSVD0(V)                                _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_RSVD0)
#define G_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_RSVD0(V)                                _MM_GETVALUE((V),S_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_RSVD0,M_ANC_CHAN_LINK_DEBUG_FIFO_STATUS_RSVD0)

      
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_HIGH                               0  //Access: read-only
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_HIGH                               _MM_MAKEMASK(1,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_HIGH)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_HIGH(V)                            _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_HIGH)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_HIGH(V)                            _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_HIGH,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_HIGH)
     
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_EMPTY                              1  //Access: read-only
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_EMPTY                              _MM_MAKEMASK(1,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_EMPTY)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_EMPTY(V)                           _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_EMPTY)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_EMPTY(V)                           _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_EMPTY,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_EMPTY)
     
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_UNDERFLOW                          2  //Access: read-only
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_UNDERFLOW                          _MM_MAKEMASK(1,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_UNDERFLOW)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_UNDERFLOW(V)                       _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_UNDERFLOW)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_UNDERFLOW(V)                       _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_UNDERFLOW,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_UNDERFLOW)
     
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD0                              3  //Access: read-as-zero
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD0                              _MM_MAKEMASK(5,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD0)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD0(V)                           _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD0)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD0(V)                           _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD0,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD0)
     
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_LEVEL                              8  //Access: read-only
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_LEVEL                              _MM_MAKEMASK(7,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_LEVEL)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_LEVEL(V)                           _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_LEVEL)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_LEVEL(V)                           _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_LEVEL,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_LEVEL)
     
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD1                              15  //Access: read-as-zero
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD1                              _MM_MAKEMASK(1,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD1)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD1(V)                           _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD1)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD1(V)                           _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD1,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD1)
     
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_READ_POINTER                       16  //Access: read-only
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_READ_POINTER                       _MM_MAKEMASK(6,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_READ_POINTER)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_READ_POINTER(V)                    _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_READ_POINTER)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_READ_POINTER(V)                    _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_READ_POINTER,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_READ_POINTER)
     
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD2                              22  //Access: read-as-zero
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD2                              _MM_MAKEMASK(2,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD2)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD2(V)                           _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD2)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD2(V)                           _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD2,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD2)
     
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_WRITE_POINTER                      24  //Access: read-only
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_WRITE_POINTER                      _MM_MAKEMASK(6,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_WRITE_POINTER)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_WRITE_POINTER(V)                   _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_WRITE_POINTER)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_WRITE_POINTER(V)                   _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_WRITE_POINTER,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_WRITE_POINTER)
     
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD3                              30  //Access: read-as-zero
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD3                              _MM_MAKEMASK(2,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD3)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD3(V)                           _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD3)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD3(V)                           _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD3,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_STATUS_RSVD3)

      
#define S_ANC_CHAN_DMA_CMDQ_FIFO_PUSH_WORD                                        0  //Access: write-only
#define M_ANC_CHAN_DMA_CMDQ_FIFO_PUSH_WORD                                        _MM_MAKEMASK(32,S_ANC_CHAN_DMA_CMDQ_FIFO_PUSH_WORD)
#define V_ANC_CHAN_DMA_CMDQ_FIFO_PUSH_WORD(V)                                     _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_PUSH_WORD)
#define G_ANC_CHAN_DMA_CMDQ_FIFO_PUSH_WORD(V)                                     _MM_GETVALUE((V),S_ANC_CHAN_DMA_CMDQ_FIFO_PUSH_WORD,M_ANC_CHAN_DMA_CMDQ_FIFO_PUSH_WORD)

      
#define S_ANC_CHAN_LINK_CMDQ_FIFO_PUSH_WORD                                       0  //Access: write-only
#define M_ANC_CHAN_LINK_CMDQ_FIFO_PUSH_WORD                                       _MM_MAKEMASK(32,S_ANC_CHAN_LINK_CMDQ_FIFO_PUSH_WORD)
#define V_ANC_CHAN_LINK_CMDQ_FIFO_PUSH_WORD(V)                                    _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_PUSH_WORD)
#define G_ANC_CHAN_LINK_CMDQ_FIFO_PUSH_WORD(V)                                    _MM_GETVALUE((V),S_ANC_CHAN_LINK_CMDQ_FIFO_PUSH_WORD,M_ANC_CHAN_LINK_CMDQ_FIFO_PUSH_WORD)

      
#define S_ANC_CHAN_LINK_BYPASS_FIFO_PUSH_WORD                                     0  //Access: write-only
#define M_ANC_CHAN_LINK_BYPASS_FIFO_PUSH_WORD                                     _MM_MAKEMASK(32,S_ANC_CHAN_LINK_BYPASS_FIFO_PUSH_WORD)
#define V_ANC_CHAN_LINK_BYPASS_FIFO_PUSH_WORD(V)                                  _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_PUSH_WORD)
#define G_ANC_CHAN_LINK_BYPASS_FIFO_PUSH_WORD(V)                                  _MM_GETVALUE((V),S_ANC_CHAN_LINK_BYPASS_FIFO_PUSH_WORD,M_ANC_CHAN_LINK_BYPASS_FIFO_PUSH_WORD)

      
#define S_ANC_CHAN_LINK_PIO_READ_FIFO_POP_WORD                                    0  //Access: read-only
#define M_ANC_CHAN_LINK_PIO_READ_FIFO_POP_WORD                                    _MM_MAKEMASK(32,S_ANC_CHAN_LINK_PIO_READ_FIFO_POP_WORD)
#define V_ANC_CHAN_LINK_PIO_READ_FIFO_POP_WORD(V)                                 _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_POP_WORD)
#define G_ANC_CHAN_LINK_PIO_READ_FIFO_POP_WORD(V)                                 _MM_GETVALUE((V),S_ANC_CHAN_LINK_PIO_READ_FIFO_POP_WORD,M_ANC_CHAN_LINK_PIO_READ_FIFO_POP_WORD)

      
#define S_ANC_CHAN_DMA_DEBUG_FIFO_POP_WORD                                        0  //Access: read-only
#define M_ANC_CHAN_DMA_DEBUG_FIFO_POP_WORD                                        _MM_MAKEMASK(32,S_ANC_CHAN_DMA_DEBUG_FIFO_POP_WORD)
#define V_ANC_CHAN_DMA_DEBUG_FIFO_POP_WORD(V)                                     _MM_MAKEVALUE((V),S_ANC_CHAN_DMA_DEBUG_FIFO_POP_WORD)
#define G_ANC_CHAN_DMA_DEBUG_FIFO_POP_WORD(V)                                     _MM_GETVALUE((V),S_ANC_CHAN_DMA_DEBUG_FIFO_POP_WORD,M_ANC_CHAN_DMA_DEBUG_FIFO_POP_WORD)

      
#define S_ANC_CHAN_LINK_DEBUG_FIFO_POP_WORD                                       0  //Access: read-only
#define M_ANC_CHAN_LINK_DEBUG_FIFO_POP_WORD                                       _MM_MAKEMASK(32,S_ANC_CHAN_LINK_DEBUG_FIFO_POP_WORD)
#define V_ANC_CHAN_LINK_DEBUG_FIFO_POP_WORD(V)                                    _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_DEBUG_FIFO_POP_WORD)
#define G_ANC_CHAN_LINK_DEBUG_FIFO_POP_WORD(V)                                    _MM_GETVALUE((V),S_ANC_CHAN_LINK_DEBUG_FIFO_POP_WORD,M_ANC_CHAN_LINK_DEBUG_FIFO_POP_WORD)

      
#define S_ANC_CHAN_LINK_ECC_STATUS_FIFO_POP_WORD                                  0  //Access: read-only
#define M_ANC_CHAN_LINK_ECC_STATUS_FIFO_POP_WORD                                  _MM_MAKEMASK(32,S_ANC_CHAN_LINK_ECC_STATUS_FIFO_POP_WORD)
#define V_ANC_CHAN_LINK_ECC_STATUS_FIFO_POP_WORD(V)                               _MM_MAKEVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_POP_WORD)
#define G_ANC_CHAN_LINK_ECC_STATUS_FIFO_POP_WORD(V)                               _MM_GETVALUE((V),S_ANC_CHAN_LINK_ECC_STATUS_FIFO_POP_WORD,M_ANC_CHAN_LINK_ECC_STATUS_FIFO_POP_WORD)

          
#define S_ANC_DMA_CONFIG_AXI_AXQOS                                                0  //Access: read-write
#define M_ANC_DMA_CONFIG_AXI_AXQOS                                                _MM_MAKEMASK(4,S_ANC_DMA_CONFIG_AXI_AXQOS)
#define V_ANC_DMA_CONFIG_AXI_AXQOS(V)                                             _MM_MAKEVALUE((V),S_ANC_DMA_CONFIG_AXI_AXQOS)
#define G_ANC_DMA_CONFIG_AXI_AXQOS(V)                                             _MM_GETVALUE((V),S_ANC_DMA_CONFIG_AXI_AXQOS,M_ANC_DMA_CONFIG_AXI_AXQOS)
     
#define S_ANC_DMA_CONFIG_RSVD0                                                    4  //Access: read-as-zero
#define M_ANC_DMA_CONFIG_RSVD0                                                    _MM_MAKEMASK(4,S_ANC_DMA_CONFIG_RSVD0)
#define V_ANC_DMA_CONFIG_RSVD0(V)                                                 _MM_MAKEVALUE((V),S_ANC_DMA_CONFIG_RSVD0)
#define G_ANC_DMA_CONFIG_RSVD0(V)                                                 _MM_GETVALUE((V),S_ANC_DMA_CONFIG_RSVD0,M_ANC_DMA_CONFIG_RSVD0)
     
#define S_ANC_DMA_CONFIG_BURST_SIZE                                               8  //Access: read-write
#define M_ANC_DMA_CONFIG_BURST_SIZE                                               _MM_MAKEMASK(2,S_ANC_DMA_CONFIG_BURST_SIZE)
#define V_ANC_DMA_CONFIG_BURST_SIZE(V)                                            _MM_MAKEVALUE((V),S_ANC_DMA_CONFIG_BURST_SIZE)
#define G_ANC_DMA_CONFIG_BURST_SIZE(V)                                            _MM_GETVALUE((V),S_ANC_DMA_CONFIG_BURST_SIZE,M_ANC_DMA_CONFIG_BURST_SIZE)
     
#define S_ANC_DMA_CONFIG_RSVD1                                                    10  //Access: read-as-zero
#define M_ANC_DMA_CONFIG_RSVD1                                                    _MM_MAKEMASK(22,S_ANC_DMA_CONFIG_RSVD1)
#define V_ANC_DMA_CONFIG_RSVD1(V)                                                 _MM_MAKEVALUE((V),S_ANC_DMA_CONFIG_RSVD1)
#define G_ANC_DMA_CONFIG_RSVD1(V)                                                 _MM_GETVALUE((V),S_ANC_DMA_CONFIG_RSVD1,M_ANC_DMA_CONFIG_RSVD1)

      
#define S_ANC_DMA_CONTROL_START                                                   0  //Access: write-auto-clear
#define M_ANC_DMA_CONTROL_START                                                   _MM_MAKEMASK(1,S_ANC_DMA_CONTROL_START)
#define V_ANC_DMA_CONTROL_START(V)                                                _MM_MAKEVALUE((V),S_ANC_DMA_CONTROL_START)
#define G_ANC_DMA_CONTROL_START(V)                                                _MM_GETVALUE((V),S_ANC_DMA_CONTROL_START,M_ANC_DMA_CONTROL_START)
     
#define S_ANC_DMA_CONTROL_STOP                                                    1  //Access: write-auto-clear
#define M_ANC_DMA_CONTROL_STOP                                                    _MM_MAKEMASK(1,S_ANC_DMA_CONTROL_STOP)
#define V_ANC_DMA_CONTROL_STOP(V)                                                 _MM_MAKEVALUE((V),S_ANC_DMA_CONTROL_STOP)
#define G_ANC_DMA_CONTROL_STOP(V)                                                 _MM_GETVALUE((V),S_ANC_DMA_CONTROL_STOP,M_ANC_DMA_CONTROL_STOP)
     
#define S_ANC_DMA_CONTROL_RESET                                                   2  //Access: write-auto-clear
#define M_ANC_DMA_CONTROL_RESET                                                   _MM_MAKEMASK(1,S_ANC_DMA_CONTROL_RESET)
#define V_ANC_DMA_CONTROL_RESET(V)                                                _MM_MAKEVALUE((V),S_ANC_DMA_CONTROL_RESET)
#define G_ANC_DMA_CONTROL_RESET(V)                                                _MM_GETVALUE((V),S_ANC_DMA_CONTROL_RESET,M_ANC_DMA_CONTROL_RESET)
     
#define S_ANC_DMA_CONTROL_RSVD0                                                   3  //Access: read-as-zero
#define M_ANC_DMA_CONTROL_RSVD0                                                   _MM_MAKEMASK(29,S_ANC_DMA_CONTROL_RSVD0)
#define V_ANC_DMA_CONTROL_RSVD0(V)                                                _MM_MAKEVALUE((V),S_ANC_DMA_CONTROL_RSVD0)
#define G_ANC_DMA_CONTROL_RSVD0(V)                                                _MM_GETVALUE((V),S_ANC_DMA_CONTROL_RSVD0,M_ANC_DMA_CONTROL_RSVD0)

      
#define S_ANC_DMA_STATUS_DMA_ACTIVE                                               0  //Access: read-only
#define M_ANC_DMA_STATUS_DMA_ACTIVE                                               _MM_MAKEMASK(1,S_ANC_DMA_STATUS_DMA_ACTIVE)
#define V_ANC_DMA_STATUS_DMA_ACTIVE(V)                                            _MM_MAKEVALUE((V),S_ANC_DMA_STATUS_DMA_ACTIVE)
#define G_ANC_DMA_STATUS_DMA_ACTIVE(V)                                            _MM_GETVALUE((V),S_ANC_DMA_STATUS_DMA_ACTIVE,M_ANC_DMA_STATUS_DMA_ACTIVE)
     
#define S_ANC_DMA_STATUS_AXI_ACTIVE                                               1  //Access: read-only
#define M_ANC_DMA_STATUS_AXI_ACTIVE                                               _MM_MAKEMASK(1,S_ANC_DMA_STATUS_AXI_ACTIVE)
#define V_ANC_DMA_STATUS_AXI_ACTIVE(V)                                            _MM_MAKEVALUE((V),S_ANC_DMA_STATUS_AXI_ACTIVE)
#define G_ANC_DMA_STATUS_AXI_ACTIVE(V)                                            _MM_GETVALUE((V),S_ANC_DMA_STATUS_AXI_ACTIVE,M_ANC_DMA_STATUS_AXI_ACTIVE)
     
#define S_ANC_DMA_STATUS_AES_ACTIVE                                               2  //Access: read-only
#define M_ANC_DMA_STATUS_AES_ACTIVE                                               _MM_MAKEMASK(1,S_ANC_DMA_STATUS_AES_ACTIVE)
#define V_ANC_DMA_STATUS_AES_ACTIVE(V)                                            _MM_MAKEVALUE((V),S_ANC_DMA_STATUS_AES_ACTIVE)
#define G_ANC_DMA_STATUS_AES_ACTIVE(V)                                            _MM_GETVALUE((V),S_ANC_DMA_STATUS_AES_ACTIVE,M_ANC_DMA_STATUS_AES_ACTIVE)
     
#define S_ANC_DMA_STATUS_CMDQ_ENABLED                                             3  //Access: read-only
#define M_ANC_DMA_STATUS_CMDQ_ENABLED                                             _MM_MAKEMASK(1,S_ANC_DMA_STATUS_CMDQ_ENABLED)
#define V_ANC_DMA_STATUS_CMDQ_ENABLED(V)                                          _MM_MAKEVALUE((V),S_ANC_DMA_STATUS_CMDQ_ENABLED)
#define G_ANC_DMA_STATUS_CMDQ_ENABLED(V)                                          _MM_GETVALUE((V),S_ANC_DMA_STATUS_CMDQ_ENABLED,M_ANC_DMA_STATUS_CMDQ_ENABLED)
     
#define S_ANC_DMA_STATUS_AES_ENABLED                                              4  //Access: read-only
#define M_ANC_DMA_STATUS_AES_ENABLED                                              _MM_MAKEMASK(1,S_ANC_DMA_STATUS_AES_ENABLED)
#define V_ANC_DMA_STATUS_AES_ENABLED(V)                                           _MM_MAKEVALUE((V),S_ANC_DMA_STATUS_AES_ENABLED)
#define G_ANC_DMA_STATUS_AES_ENABLED(V)                                           _MM_GETVALUE((V),S_ANC_DMA_STATUS_AES_ENABLED,M_ANC_DMA_STATUS_AES_ENABLED)
     
#define S_ANC_DMA_STATUS_DMA_DIRECTION                                            5  //Access: read-only
#define M_ANC_DMA_STATUS_DMA_DIRECTION                                            _MM_MAKEMASK(1,S_ANC_DMA_STATUS_DMA_DIRECTION)
#define V_ANC_DMA_STATUS_DMA_DIRECTION(V)                                         _MM_MAKEVALUE((V),S_ANC_DMA_STATUS_DMA_DIRECTION)
#define G_ANC_DMA_STATUS_DMA_DIRECTION(V)                                         _MM_GETVALUE((V),S_ANC_DMA_STATUS_DMA_DIRECTION,M_ANC_DMA_STATUS_DMA_DIRECTION)
     
#define S_ANC_DMA_STATUS_RSVD0                                                    6  //Access: read-as-zero
#define M_ANC_DMA_STATUS_RSVD0                                                    _MM_MAKEMASK(26,S_ANC_DMA_STATUS_RSVD0)
#define V_ANC_DMA_STATUS_RSVD0(V)                                                 _MM_MAKEVALUE((V),S_ANC_DMA_STATUS_RSVD0)
#define G_ANC_DMA_STATUS_RSVD0(V)                                                 _MM_GETVALUE((V),S_ANC_DMA_STATUS_RSVD0,M_ANC_DMA_STATUS_RSVD0)

      
#define S_ANC_DMA_AES_STATUS_RSVD0                                                0  //Access: read-as-zero
#define M_ANC_DMA_AES_STATUS_RSVD0                                                _MM_MAKEMASK(1,S_ANC_DMA_AES_STATUS_RSVD0)
#define V_ANC_DMA_AES_STATUS_RSVD0(V)                                             _MM_MAKEVALUE((V),S_ANC_DMA_AES_STATUS_RSVD0)
#define G_ANC_DMA_AES_STATUS_RSVD0(V)                                             _MM_GETVALUE((V),S_ANC_DMA_AES_STATUS_RSVD0,M_ANC_DMA_AES_STATUS_RSVD0)
     
#define S_ANC_DMA_AES_STATUS_KEY_SELECT                                           1  //Access: read-only
#define M_ANC_DMA_AES_STATUS_KEY_SELECT                                           _MM_MAKEMASK(1,S_ANC_DMA_AES_STATUS_KEY_SELECT)
#define V_ANC_DMA_AES_STATUS_KEY_SELECT(V)                                        _MM_MAKEVALUE((V),S_ANC_DMA_AES_STATUS_KEY_SELECT)
#define G_ANC_DMA_AES_STATUS_KEY_SELECT(V)                                        _MM_GETVALUE((V),S_ANC_DMA_AES_STATUS_KEY_SELECT,M_ANC_DMA_AES_STATUS_KEY_SELECT)
     
#define S_ANC_DMA_AES_STATUS_KEY_LENGTH                                           2  //Access: read-only
#define M_ANC_DMA_AES_STATUS_KEY_LENGTH                                           _MM_MAKEMASK(2,S_ANC_DMA_AES_STATUS_KEY_LENGTH)
#define V_ANC_DMA_AES_STATUS_KEY_LENGTH(V)                                        _MM_MAKEVALUE((V),S_ANC_DMA_AES_STATUS_KEY_LENGTH)
#define G_ANC_DMA_AES_STATUS_KEY_LENGTH(V)                                        _MM_GETVALUE((V),S_ANC_DMA_AES_STATUS_KEY_LENGTH,M_ANC_DMA_AES_STATUS_KEY_LENGTH)
     
#define S_ANC_DMA_AES_STATUS_RSVD1                                                4  //Access: read-as-zero
#define M_ANC_DMA_AES_STATUS_RSVD1                                                _MM_MAKEMASK(1,S_ANC_DMA_AES_STATUS_RSVD1)
#define V_ANC_DMA_AES_STATUS_RSVD1(V)                                             _MM_MAKEVALUE((V),S_ANC_DMA_AES_STATUS_RSVD1)
#define G_ANC_DMA_AES_STATUS_RSVD1(V)                                             _MM_GETVALUE((V),S_ANC_DMA_AES_STATUS_RSVD1,M_ANC_DMA_AES_STATUS_RSVD1)
     
#define S_ANC_DMA_AES_STATUS_KEY_IN_CTX                                           5  //Access: read-only
#define M_ANC_DMA_AES_STATUS_KEY_IN_CTX                                           _MM_MAKEMASK(1,S_ANC_DMA_AES_STATUS_KEY_IN_CTX)
#define V_ANC_DMA_AES_STATUS_KEY_IN_CTX(V)                                        _MM_MAKEVALUE((V),S_ANC_DMA_AES_STATUS_KEY_IN_CTX)
#define G_ANC_DMA_AES_STATUS_KEY_IN_CTX(V)                                        _MM_GETVALUE((V),S_ANC_DMA_AES_STATUS_KEY_IN_CTX,M_ANC_DMA_AES_STATUS_KEY_IN_CTX)
     
#define S_ANC_DMA_AES_STATUS_IV_IN_CTX                                            6  //Access: read-only
#define M_ANC_DMA_AES_STATUS_IV_IN_CTX                                            _MM_MAKEMASK(1,S_ANC_DMA_AES_STATUS_IV_IN_CTX)
#define V_ANC_DMA_AES_STATUS_IV_IN_CTX(V)                                         _MM_MAKEVALUE((V),S_ANC_DMA_AES_STATUS_IV_IN_CTX)
#define G_ANC_DMA_AES_STATUS_IV_IN_CTX(V)                                         _MM_GETVALUE((V),S_ANC_DMA_AES_STATUS_IV_IN_CTX,M_ANC_DMA_AES_STATUS_IV_IN_CTX)
     
#define S_ANC_DMA_AES_STATUS_TXT_IN_KEY_CTX                                       7  //Access: read-only
#define M_ANC_DMA_AES_STATUS_TXT_IN_KEY_CTX                                       _MM_MAKEMASK(1,S_ANC_DMA_AES_STATUS_TXT_IN_KEY_CTX)
#define V_ANC_DMA_AES_STATUS_TXT_IN_KEY_CTX(V)                                    _MM_MAKEVALUE((V),S_ANC_DMA_AES_STATUS_TXT_IN_KEY_CTX)
#define G_ANC_DMA_AES_STATUS_TXT_IN_KEY_CTX(V)                                    _MM_GETVALUE((V),S_ANC_DMA_AES_STATUS_TXT_IN_KEY_CTX,M_ANC_DMA_AES_STATUS_TXT_IN_KEY_CTX)
     
#define S_ANC_DMA_AES_STATUS_TXT_IN_IV_CTX                                        8  //Access: read-only
#define M_ANC_DMA_AES_STATUS_TXT_IN_IV_CTX                                        _MM_MAKEMASK(1,S_ANC_DMA_AES_STATUS_TXT_IN_IV_CTX)
#define V_ANC_DMA_AES_STATUS_TXT_IN_IV_CTX(V)                                     _MM_MAKEVALUE((V),S_ANC_DMA_AES_STATUS_TXT_IN_IV_CTX)
#define G_ANC_DMA_AES_STATUS_TXT_IN_IV_CTX(V)                                     _MM_GETVALUE((V),S_ANC_DMA_AES_STATUS_TXT_IN_IV_CTX,M_ANC_DMA_AES_STATUS_TXT_IN_IV_CTX)
     
#define S_ANC_DMA_AES_STATUS_AES_ERR                                              9  //Access: read-only
#define M_ANC_DMA_AES_STATUS_AES_ERR                                              _MM_MAKEMASK(1,S_ANC_DMA_AES_STATUS_AES_ERR)
#define V_ANC_DMA_AES_STATUS_AES_ERR(V)                                           _MM_MAKEVALUE((V),S_ANC_DMA_AES_STATUS_AES_ERR)
#define G_ANC_DMA_AES_STATUS_AES_ERR(V)                                           _MM_GETVALUE((V),S_ANC_DMA_AES_STATUS_AES_ERR,M_ANC_DMA_AES_STATUS_AES_ERR)
     
#define S_ANC_DMA_AES_STATUS_RSVD2                                                10  //Access: read-as-zero
#define M_ANC_DMA_AES_STATUS_RSVD2                                                _MM_MAKEMASK(22,S_ANC_DMA_AES_STATUS_RSVD2)
#define V_ANC_DMA_AES_STATUS_RSVD2(V)                                             _MM_MAKEVALUE((V),S_ANC_DMA_AES_STATUS_RSVD2)
#define G_ANC_DMA_AES_STATUS_RSVD2(V)                                             _MM_GETVALUE((V),S_ANC_DMA_AES_STATUS_RSVD2,M_ANC_DMA_AES_STATUS_RSVD2)

      
#define S_ANC_DMA_CMDQ_COUNT_TOTAL                                                0  //Access: read-only
#define M_ANC_DMA_CMDQ_COUNT_TOTAL                                                _MM_MAKEMASK(32,S_ANC_DMA_CMDQ_COUNT_TOTAL)
#define V_ANC_DMA_CMDQ_COUNT_TOTAL(V)                                             _MM_MAKEVALUE((V),S_ANC_DMA_CMDQ_COUNT_TOTAL)
#define G_ANC_DMA_CMDQ_COUNT_TOTAL(V)                                             _MM_GETVALUE((V),S_ANC_DMA_CMDQ_COUNT_TOTAL,M_ANC_DMA_CMDQ_COUNT_TOTAL)

      
#define S_ANC_DMA_CMD_TIMEOUT_VALUE                                               0  //Access: read-write
#define M_ANC_DMA_CMD_TIMEOUT_VALUE                                               _MM_MAKEMASK(31,S_ANC_DMA_CMD_TIMEOUT_VALUE)
#define V_ANC_DMA_CMD_TIMEOUT_VALUE(V)                                            _MM_MAKEVALUE((V),S_ANC_DMA_CMD_TIMEOUT_VALUE)
#define G_ANC_DMA_CMD_TIMEOUT_VALUE(V)                                            _MM_GETVALUE((V),S_ANC_DMA_CMD_TIMEOUT_VALUE,M_ANC_DMA_CMD_TIMEOUT_VALUE)
     
#define S_ANC_DMA_CMD_TIMEOUT_ENABLE                                              31  //Access: read-write
#define M_ANC_DMA_CMD_TIMEOUT_ENABLE                                              _MM_MAKEMASK(1,S_ANC_DMA_CMD_TIMEOUT_ENABLE)
#define V_ANC_DMA_CMD_TIMEOUT_ENABLE(V)                                           _MM_MAKEVALUE((V),S_ANC_DMA_CMD_TIMEOUT_ENABLE)
#define G_ANC_DMA_CMD_TIMEOUT_ENABLE(V)                                           _MM_GETVALUE((V),S_ANC_DMA_CMD_TIMEOUT_ENABLE,M_ANC_DMA_CMD_TIMEOUT_ENABLE)

      
#define S_ANC_DMA_CMDQ_INT_CODE_CODE                                              0  //Access: read-only
#define M_ANC_DMA_CMDQ_INT_CODE_CODE                                              _MM_MAKEMASK(16,S_ANC_DMA_CMDQ_INT_CODE_CODE)
#define V_ANC_DMA_CMDQ_INT_CODE_CODE(V)                                           _MM_MAKEVALUE((V),S_ANC_DMA_CMDQ_INT_CODE_CODE)
#define G_ANC_DMA_CMDQ_INT_CODE_CODE(V)                                           _MM_GETVALUE((V),S_ANC_DMA_CMDQ_INT_CODE_CODE,M_ANC_DMA_CMDQ_INT_CODE_CODE)
     
#define S_ANC_DMA_CMDQ_INT_CODE_RSVD0                                             16  //Access: read-as-zero
#define M_ANC_DMA_CMDQ_INT_CODE_RSVD0                                             _MM_MAKEMASK(16,S_ANC_DMA_CMDQ_INT_CODE_RSVD0)
#define V_ANC_DMA_CMDQ_INT_CODE_RSVD0(V)                                          _MM_MAKEVALUE((V),S_ANC_DMA_CMDQ_INT_CODE_RSVD0)
#define G_ANC_DMA_CMDQ_INT_CODE_RSVD0(V)                                          _MM_GETVALUE((V),S_ANC_DMA_CMDQ_INT_CODE_RSVD0,M_ANC_DMA_CMDQ_INT_CODE_RSVD0)

      
#define S_ANC_DMA_AXI_ADDRESS_REQUESTS_OUTSTANDING                                0  //Access: read-only
#define M_ANC_DMA_AXI_ADDRESS_REQUESTS_OUTSTANDING                                _MM_MAKEMASK(5,S_ANC_DMA_AXI_ADDRESS_REQUESTS_OUTSTANDING)
#define V_ANC_DMA_AXI_ADDRESS_REQUESTS_OUTSTANDING(V)                             _MM_MAKEVALUE((V),S_ANC_DMA_AXI_ADDRESS_REQUESTS_OUTSTANDING)
#define G_ANC_DMA_AXI_ADDRESS_REQUESTS_OUTSTANDING(V)                             _MM_GETVALUE((V),S_ANC_DMA_AXI_ADDRESS_REQUESTS_OUTSTANDING,M_ANC_DMA_AXI_ADDRESS_REQUESTS_OUTSTANDING)
     
#define S_ANC_DMA_AXI_ADDRESS_REQUESTS_WORDS_REMAINING                            5  //Access: read-only
#define M_ANC_DMA_AXI_ADDRESS_REQUESTS_WORDS_REMAINING                            _MM_MAKEMASK(9,S_ANC_DMA_AXI_ADDRESS_REQUESTS_WORDS_REMAINING)
#define V_ANC_DMA_AXI_ADDRESS_REQUESTS_WORDS_REMAINING(V)                         _MM_MAKEVALUE((V),S_ANC_DMA_AXI_ADDRESS_REQUESTS_WORDS_REMAINING)
#define G_ANC_DMA_AXI_ADDRESS_REQUESTS_WORDS_REMAINING(V)                         _MM_GETVALUE((V),S_ANC_DMA_AXI_ADDRESS_REQUESTS_WORDS_REMAINING,M_ANC_DMA_AXI_ADDRESS_REQUESTS_WORDS_REMAINING)
     
#define S_ANC_DMA_AXI_RESPONSE                                                    14  //Access: read-only
#define M_ANC_DMA_AXI_RESPONSE                                                    _MM_MAKEMASK(2,S_ANC_DMA_AXI_RESPONSE)
#define V_ANC_DMA_AXI_RESPONSE(V)                                                 _MM_MAKEVALUE((V),S_ANC_DMA_AXI_RESPONSE)
#define G_ANC_DMA_AXI_RESPONSE(V)                                                 _MM_GETVALUE((V),S_ANC_DMA_AXI_RESPONSE,M_ANC_DMA_AXI_RESPONSE)
     
#define S_ANC_DMA_AXI_WRITE_DATA_AVAILABLE                                        16  //Access: read-only
#define M_ANC_DMA_AXI_WRITE_DATA_AVAILABLE                                        _MM_MAKEMASK(7,S_ANC_DMA_AXI_WRITE_DATA_AVAILABLE)
#define V_ANC_DMA_AXI_WRITE_DATA_AVAILABLE(V)                                     _MM_MAKEVALUE((V),S_ANC_DMA_AXI_WRITE_DATA_AVAILABLE)
#define G_ANC_DMA_AXI_WRITE_DATA_AVAILABLE(V)                                     _MM_GETVALUE((V),S_ANC_DMA_AXI_WRITE_DATA_AVAILABLE,M_ANC_DMA_AXI_WRITE_DATA_AVAILABLE)
     
#define S_ANC_DMA_AXI_READ_SPACE_AVAILABLE                                        23  //Access: read-only
#define M_ANC_DMA_AXI_READ_SPACE_AVAILABLE                                        _MM_MAKEMASK(7,S_ANC_DMA_AXI_READ_SPACE_AVAILABLE)
#define V_ANC_DMA_AXI_READ_SPACE_AVAILABLE(V)                                     _MM_MAKEVALUE((V),S_ANC_DMA_AXI_READ_SPACE_AVAILABLE)
#define G_ANC_DMA_AXI_READ_SPACE_AVAILABLE(V)                                     _MM_GETVALUE((V),S_ANC_DMA_AXI_READ_SPACE_AVAILABLE,M_ANC_DMA_AXI_READ_SPACE_AVAILABLE)
     
#define S_ANC_DMA_AXI_RSVD0                                                       30  //Access: read-as-zero
#define M_ANC_DMA_AXI_RSVD0                                                       _MM_MAKEMASK(2,S_ANC_DMA_AXI_RSVD0)
#define V_ANC_DMA_AXI_RSVD0(V)                                                    _MM_MAKEVALUE((V),S_ANC_DMA_AXI_RSVD0)
#define G_ANC_DMA_AXI_RSVD0(V)                                                    _MM_GETVALUE((V),S_ANC_DMA_AXI_RSVD0,M_ANC_DMA_AXI_RSVD0)

      
#define S_ANC_DMA_AXI_NEXT_REQUEST_LENGTH                                         0  //Access: read-only
#define M_ANC_DMA_AXI_NEXT_REQUEST_LENGTH                                         _MM_MAKEMASK(4,S_ANC_DMA_AXI_NEXT_REQUEST_LENGTH)
#define V_ANC_DMA_AXI_NEXT_REQUEST_LENGTH(V)                                      _MM_MAKEVALUE((V),S_ANC_DMA_AXI_NEXT_REQUEST_LENGTH)
#define G_ANC_DMA_AXI_NEXT_REQUEST_LENGTH(V)                                      _MM_GETVALUE((V),S_ANC_DMA_AXI_NEXT_REQUEST_LENGTH,M_ANC_DMA_AXI_NEXT_REQUEST_LENGTH)
     
#define S_ANC_DMA_AXI_NEXT_RSVD0                                                  4  //Access: read-as-zero
#define M_ANC_DMA_AXI_NEXT_RSVD0                                                  _MM_MAKEMASK(20,S_ANC_DMA_AXI_NEXT_RSVD0)
#define V_ANC_DMA_AXI_NEXT_RSVD0(V)                                               _MM_MAKEVALUE((V),S_ANC_DMA_AXI_NEXT_RSVD0)
#define G_ANC_DMA_AXI_NEXT_RSVD0(V)                                               _MM_GETVALUE((V),S_ANC_DMA_AXI_NEXT_RSVD0,M_ANC_DMA_AXI_NEXT_RSVD0)
     
#define S_ANC_DMA_AXI_NEXT_ADDRESS_HIGH_BYTE                                      24  //Access: read-only
#define M_ANC_DMA_AXI_NEXT_ADDRESS_HIGH_BYTE                                      _MM_MAKEMASK(8,S_ANC_DMA_AXI_NEXT_ADDRESS_HIGH_BYTE)
#define V_ANC_DMA_AXI_NEXT_ADDRESS_HIGH_BYTE(V)                                   _MM_MAKEVALUE((V),S_ANC_DMA_AXI_NEXT_ADDRESS_HIGH_BYTE)
#define G_ANC_DMA_AXI_NEXT_ADDRESS_HIGH_BYTE(V)                                   _MM_GETVALUE((V),S_ANC_DMA_AXI_NEXT_ADDRESS_HIGH_BYTE,M_ANC_DMA_AXI_NEXT_ADDRESS_HIGH_BYTE)

      
#define S_ANC_DMA_AXI_NEXT_ADDRESS_LOWER_FOUR_BYTES                               0  //Access: read-only
#define M_ANC_DMA_AXI_NEXT_ADDRESS_LOWER_FOUR_BYTES                               _MM_MAKEMASK(32,S_ANC_DMA_AXI_NEXT_ADDRESS_LOWER_FOUR_BYTES)
#define V_ANC_DMA_AXI_NEXT_ADDRESS_LOWER_FOUR_BYTES(V)                            _MM_MAKEVALUE((V),S_ANC_DMA_AXI_NEXT_ADDRESS_LOWER_FOUR_BYTES)
#define G_ANC_DMA_AXI_NEXT_ADDRESS_LOWER_FOUR_BYTES(V)                            _MM_GETVALUE((V),S_ANC_DMA_AXI_NEXT_ADDRESS_LOWER_FOUR_BYTES,M_ANC_DMA_AXI_NEXT_ADDRESS_LOWER_FOUR_BYTES)

      
#define S_ANC_DMA_AXI_DMA_COUNT_WRITE_WORDS                                       0  //Access: read-only
#define M_ANC_DMA_AXI_DMA_COUNT_WRITE_WORDS                                       _MM_MAKEMASK(32,S_ANC_DMA_AXI_DMA_COUNT_WRITE_WORDS)
#define V_ANC_DMA_AXI_DMA_COUNT_WRITE_WORDS(V)                                    _MM_MAKEVALUE((V),S_ANC_DMA_AXI_DMA_COUNT_WRITE_WORDS)
#define G_ANC_DMA_AXI_DMA_COUNT_WRITE_WORDS(V)                                    _MM_GETVALUE((V),S_ANC_DMA_AXI_DMA_COUNT_WRITE_WORDS,M_ANC_DMA_AXI_DMA_COUNT_WRITE_WORDS)

      
#define S_ANC_DMA_AXI_COUNT_READ_WORDS                                            0  //Access: read-only
#define M_ANC_DMA_AXI_COUNT_READ_WORDS                                            _MM_MAKEMASK(32,S_ANC_DMA_AXI_COUNT_READ_WORDS)
#define V_ANC_DMA_AXI_COUNT_READ_WORDS(V)                                         _MM_MAKEVALUE((V),S_ANC_DMA_AXI_COUNT_READ_WORDS)
#define G_ANC_DMA_AXI_COUNT_READ_WORDS(V)                                         _MM_GETVALUE((V),S_ANC_DMA_AXI_COUNT_READ_WORDS,M_ANC_DMA_AXI_COUNT_READ_WORDS)

      
#define S_ANC_DMA_DLFIFO_COUNT_WRITE_WORDS                                        0  //Access: read-only
#define M_ANC_DMA_DLFIFO_COUNT_WRITE_WORDS                                        _MM_MAKEMASK(32,S_ANC_DMA_DLFIFO_COUNT_WRITE_WORDS)
#define V_ANC_DMA_DLFIFO_COUNT_WRITE_WORDS(V)                                     _MM_MAKEVALUE((V),S_ANC_DMA_DLFIFO_COUNT_WRITE_WORDS)
#define G_ANC_DMA_DLFIFO_COUNT_WRITE_WORDS(V)                                     _MM_GETVALUE((V),S_ANC_DMA_DLFIFO_COUNT_WRITE_WORDS,M_ANC_DMA_DLFIFO_COUNT_WRITE_WORDS)

      
#define S_ANC_DMA_DLFIFO_DMA_COUNT_READ_WORDS                                     0  //Access: read-only
#define M_ANC_DMA_DLFIFO_DMA_COUNT_READ_WORDS                                     _MM_MAKEMASK(32,S_ANC_DMA_DLFIFO_DMA_COUNT_READ_WORDS)
#define V_ANC_DMA_DLFIFO_DMA_COUNT_READ_WORDS(V)                                  _MM_MAKEVALUE((V),S_ANC_DMA_DLFIFO_DMA_COUNT_READ_WORDS)
#define G_ANC_DMA_DLFIFO_DMA_COUNT_READ_WORDS(V)                                  _MM_GETVALUE((V),S_ANC_DMA_DLFIFO_DMA_COUNT_READ_WORDS,M_ANC_DMA_DLFIFO_DMA_COUNT_READ_WORDS)

      
#define S_ANC_DMA_AXI_DMA_LAST_COUNT_WRITE_WORDS                                  0  //Access: read-only
#define M_ANC_DMA_AXI_DMA_LAST_COUNT_WRITE_WORDS                                  _MM_MAKEMASK(32,S_ANC_DMA_AXI_DMA_LAST_COUNT_WRITE_WORDS)
#define V_ANC_DMA_AXI_DMA_LAST_COUNT_WRITE_WORDS(V)                               _MM_MAKEVALUE((V),S_ANC_DMA_AXI_DMA_LAST_COUNT_WRITE_WORDS)
#define G_ANC_DMA_AXI_DMA_LAST_COUNT_WRITE_WORDS(V)                               _MM_GETVALUE((V),S_ANC_DMA_AXI_DMA_LAST_COUNT_WRITE_WORDS,M_ANC_DMA_AXI_DMA_LAST_COUNT_WRITE_WORDS)

      
#define S_ANC_DMA_AXI_LAST_COUNT_READ_WORDS                                       0  //Access: read-only
#define M_ANC_DMA_AXI_LAST_COUNT_READ_WORDS                                       _MM_MAKEMASK(32,S_ANC_DMA_AXI_LAST_COUNT_READ_WORDS)
#define V_ANC_DMA_AXI_LAST_COUNT_READ_WORDS(V)                                    _MM_MAKEVALUE((V),S_ANC_DMA_AXI_LAST_COUNT_READ_WORDS)
#define G_ANC_DMA_AXI_LAST_COUNT_READ_WORDS(V)                                    _MM_GETVALUE((V),S_ANC_DMA_AXI_LAST_COUNT_READ_WORDS,M_ANC_DMA_AXI_LAST_COUNT_READ_WORDS)

      
#define S_ANC_DMA_DLFIFO_LAST_COUNT_WRITE_WORDS                                   0  //Access: read-only
#define M_ANC_DMA_DLFIFO_LAST_COUNT_WRITE_WORDS                                   _MM_MAKEMASK(32,S_ANC_DMA_DLFIFO_LAST_COUNT_WRITE_WORDS)
#define V_ANC_DMA_DLFIFO_LAST_COUNT_WRITE_WORDS(V)                                _MM_MAKEVALUE((V),S_ANC_DMA_DLFIFO_LAST_COUNT_WRITE_WORDS)
#define G_ANC_DMA_DLFIFO_LAST_COUNT_WRITE_WORDS(V)                                _MM_GETVALUE((V),S_ANC_DMA_DLFIFO_LAST_COUNT_WRITE_WORDS,M_ANC_DMA_DLFIFO_LAST_COUNT_WRITE_WORDS)

      
#define S_ANC_DMA_DLFIFO_DMA_LAST_COUNT_READ_WORDS                                0  //Access: read-only
#define M_ANC_DMA_DLFIFO_DMA_LAST_COUNT_READ_WORDS                                _MM_MAKEMASK(32,S_ANC_DMA_DLFIFO_DMA_LAST_COUNT_READ_WORDS)
#define V_ANC_DMA_DLFIFO_DMA_LAST_COUNT_READ_WORDS(V)                             _MM_MAKEVALUE((V),S_ANC_DMA_DLFIFO_DMA_LAST_COUNT_READ_WORDS)
#define G_ANC_DMA_DLFIFO_DMA_LAST_COUNT_READ_WORDS(V)                             _MM_GETVALUE((V),S_ANC_DMA_DLFIFO_DMA_LAST_COUNT_READ_WORDS,M_ANC_DMA_DLFIFO_DMA_LAST_COUNT_READ_WORDS)

          
#define S_ANC_LINK_CONFIG_DDR_MODE                                                0  //Access: read-write
#define M_ANC_LINK_CONFIG_DDR_MODE                                                _MM_MAKEMASK(1,S_ANC_LINK_CONFIG_DDR_MODE)
#define V_ANC_LINK_CONFIG_DDR_MODE(V)                                             _MM_MAKEVALUE((V),S_ANC_LINK_CONFIG_DDR_MODE)
#define G_ANC_LINK_CONFIG_DDR_MODE(V)                                             _MM_GETVALUE((V),S_ANC_LINK_CONFIG_DDR_MODE,M_ANC_LINK_CONFIG_DDR_MODE)
     
#define S_ANC_LINK_CONFIG_ENABLE_CRC                                              1  //Access: read-write
#define M_ANC_LINK_CONFIG_ENABLE_CRC                                              _MM_MAKEMASK(1,S_ANC_LINK_CONFIG_ENABLE_CRC)
#define V_ANC_LINK_CONFIG_ENABLE_CRC(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_CONFIG_ENABLE_CRC)
#define G_ANC_LINK_CONFIG_ENABLE_CRC(V)                                           _MM_GETVALUE((V),S_ANC_LINK_CONFIG_ENABLE_CRC,M_ANC_LINK_CONFIG_ENABLE_CRC)
     
#define S_ANC_LINK_CONFIG_AUTOPAD_CRC                                             2  //Access: read-write
#define M_ANC_LINK_CONFIG_AUTOPAD_CRC                                             _MM_MAKEMASK(1,S_ANC_LINK_CONFIG_AUTOPAD_CRC)
#define V_ANC_LINK_CONFIG_AUTOPAD_CRC(V)                                          _MM_MAKEVALUE((V),S_ANC_LINK_CONFIG_AUTOPAD_CRC)
#define G_ANC_LINK_CONFIG_AUTOPAD_CRC(V)                                          _MM_GETVALUE((V),S_ANC_LINK_CONFIG_AUTOPAD_CRC,M_ANC_LINK_CONFIG_AUTOPAD_CRC)
     
#define S_ANC_LINK_CONFIG_ENABLE_SCRAMBLER                                        3  //Access: read-write
#define M_ANC_LINK_CONFIG_ENABLE_SCRAMBLER                                        _MM_MAKEMASK(1,S_ANC_LINK_CONFIG_ENABLE_SCRAMBLER)
#define V_ANC_LINK_CONFIG_ENABLE_SCRAMBLER(V)                                     _MM_MAKEVALUE((V),S_ANC_LINK_CONFIG_ENABLE_SCRAMBLER)
#define G_ANC_LINK_CONFIG_ENABLE_SCRAMBLER(V)                                     _MM_GETVALUE((V),S_ANC_LINK_CONFIG_ENABLE_SCRAMBLER,M_ANC_LINK_CONFIG_ENABLE_SCRAMBLER)
     
#define S_ANC_LINK_CONFIG_RSVD0                                                   4  //Access: read-as-zero
#define M_ANC_LINK_CONFIG_RSVD0                                                   _MM_MAKEMASK(28,S_ANC_LINK_CONFIG_RSVD0)
#define V_ANC_LINK_CONFIG_RSVD0(V)                                                _MM_MAKEVALUE((V),S_ANC_LINK_CONFIG_RSVD0)
#define G_ANC_LINK_CONFIG_RSVD0(V)                                                _MM_GETVALUE((V),S_ANC_LINK_CONFIG_RSVD0,M_ANC_LINK_CONFIG_RSVD0)

      
#define S_ANC_LINK_CONTROL_START                                                  0  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_START                                                  _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_START)
#define V_ANC_LINK_CONTROL_START(V)                                               _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_START)
#define G_ANC_LINK_CONTROL_START(V)                                               _MM_GETVALUE((V),S_ANC_LINK_CONTROL_START,M_ANC_LINK_CONTROL_START)
     
#define S_ANC_LINK_CONTROL_STOP                                                   1  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_STOP                                                   _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_STOP)
#define V_ANC_LINK_CONTROL_STOP(V)                                                _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_STOP)
#define G_ANC_LINK_CONTROL_STOP(V)                                                _MM_GETVALUE((V),S_ANC_LINK_CONTROL_STOP,M_ANC_LINK_CONTROL_STOP)
     
#define S_ANC_LINK_CONTROL_YIELD                                                  2  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_YIELD                                                  _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_YIELD)
#define V_ANC_LINK_CONTROL_YIELD(V)                                               _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_YIELD)
#define G_ANC_LINK_CONTROL_YIELD(V)                                               _MM_GETVALUE((V),S_ANC_LINK_CONTROL_YIELD,M_ANC_LINK_CONTROL_YIELD)
     
#define S_ANC_LINK_CONTROL_ABORT                                                  3  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_ABORT                                                  _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_ABORT)
#define V_ANC_LINK_CONTROL_ABORT(V)                                               _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_ABORT)
#define G_ANC_LINK_CONTROL_ABORT(V)                                               _MM_GETVALUE((V),S_ANC_LINK_CONTROL_ABORT,M_ANC_LINK_CONTROL_ABORT)
     
#define S_ANC_LINK_CONTROL_RESET                                                  4  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_RESET                                                  _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_RESET)
#define V_ANC_LINK_CONTROL_RESET(V)                                               _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_RESET)
#define G_ANC_LINK_CONTROL_RESET(V)                                               _MM_GETVALUE((V),S_ANC_LINK_CONTROL_RESET,M_ANC_LINK_CONTROL_RESET)
     
#define S_ANC_LINK_CONTROL_START_CMDQ                                             5  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_START_CMDQ                                             _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_START_CMDQ)
#define V_ANC_LINK_CONTROL_START_CMDQ(V)                                          _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_START_CMDQ)
#define G_ANC_LINK_CONTROL_START_CMDQ(V)                                          _MM_GETVALUE((V),S_ANC_LINK_CONTROL_START_CMDQ,M_ANC_LINK_CONTROL_START_CMDQ)
     
#define S_ANC_LINK_CONTROL_STOP_CMDQ                                              6  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_STOP_CMDQ                                              _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_STOP_CMDQ)
#define V_ANC_LINK_CONTROL_STOP_CMDQ(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_STOP_CMDQ)
#define G_ANC_LINK_CONTROL_STOP_CMDQ(V)                                           _MM_GETVALUE((V),S_ANC_LINK_CONTROL_STOP_CMDQ,M_ANC_LINK_CONTROL_STOP_CMDQ)
     
#define S_ANC_LINK_CONTROL_RESET_CMDQ                                             7  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_RESET_CMDQ                                             _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_RESET_CMDQ)
#define V_ANC_LINK_CONTROL_RESET_CMDQ(V)                                          _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_RESET_CMDQ)
#define G_ANC_LINK_CONTROL_RESET_CMDQ(V)                                          _MM_GETVALUE((V),S_ANC_LINK_CONTROL_RESET_CMDQ,M_ANC_LINK_CONTROL_RESET_CMDQ)
     
#define S_ANC_LINK_CONTROL_START_BYPASS                                           8  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_START_BYPASS                                           _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_START_BYPASS)
#define V_ANC_LINK_CONTROL_START_BYPASS(V)                                        _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_START_BYPASS)
#define G_ANC_LINK_CONTROL_START_BYPASS(V)                                        _MM_GETVALUE((V),S_ANC_LINK_CONTROL_START_BYPASS,M_ANC_LINK_CONTROL_START_BYPASS)
     
#define S_ANC_LINK_CONTROL_STOP_BYPASS                                            9  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_STOP_BYPASS                                            _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_STOP_BYPASS)
#define V_ANC_LINK_CONTROL_STOP_BYPASS(V)                                         _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_STOP_BYPASS)
#define G_ANC_LINK_CONTROL_STOP_BYPASS(V)                                         _MM_GETVALUE((V),S_ANC_LINK_CONTROL_STOP_BYPASS,M_ANC_LINK_CONTROL_STOP_BYPASS)
     
#define S_ANC_LINK_CONTROL_RESET_BYPASS                                           10  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_RESET_BYPASS                                           _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_RESET_BYPASS)
#define V_ANC_LINK_CONTROL_RESET_BYPASS(V)                                        _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_RESET_BYPASS)
#define G_ANC_LINK_CONTROL_RESET_BYPASS(V)                                        _MM_GETVALUE((V),S_ANC_LINK_CONTROL_RESET_BYPASS,M_ANC_LINK_CONTROL_RESET_BYPASS)
     
#define S_ANC_LINK_CONTROL_START_TIMER                                            11  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_START_TIMER                                            _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_START_TIMER)
#define V_ANC_LINK_CONTROL_START_TIMER(V)                                         _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_START_TIMER)
#define G_ANC_LINK_CONTROL_START_TIMER(V)                                         _MM_GETVALUE((V),S_ANC_LINK_CONTROL_START_TIMER,M_ANC_LINK_CONTROL_START_TIMER)
     
#define S_ANC_LINK_CONTROL_STOP_TIMER                                             12  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_STOP_TIMER                                             _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_STOP_TIMER)
#define V_ANC_LINK_CONTROL_STOP_TIMER(V)                                          _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_STOP_TIMER)
#define G_ANC_LINK_CONTROL_STOP_TIMER(V)                                          _MM_GETVALUE((V),S_ANC_LINK_CONTROL_STOP_TIMER,M_ANC_LINK_CONTROL_STOP_TIMER)
     
#define S_ANC_LINK_CONTROL_RESET_TIMER                                            13  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_RESET_TIMER                                            _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_RESET_TIMER)
#define V_ANC_LINK_CONTROL_RESET_TIMER(V)                                         _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_RESET_TIMER)
#define G_ANC_LINK_CONTROL_RESET_TIMER(V)                                         _MM_GETVALUE((V),S_ANC_LINK_CONTROL_RESET_TIMER,M_ANC_LINK_CONTROL_RESET_TIMER)
     
#define S_ANC_LINK_CONTROL_RESET_PIO_READ                                         14  //Access: write-auto-clear
#define M_ANC_LINK_CONTROL_RESET_PIO_READ                                         _MM_MAKEMASK(1,S_ANC_LINK_CONTROL_RESET_PIO_READ)
#define V_ANC_LINK_CONTROL_RESET_PIO_READ(V)                                      _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_RESET_PIO_READ)
#define G_ANC_LINK_CONTROL_RESET_PIO_READ(V)                                      _MM_GETVALUE((V),S_ANC_LINK_CONTROL_RESET_PIO_READ,M_ANC_LINK_CONTROL_RESET_PIO_READ)
     
#define S_ANC_LINK_CONTROL_RSVD0                                                  15  //Access: read-as-zero
#define M_ANC_LINK_CONTROL_RSVD0                                                  _MM_MAKEMASK(17,S_ANC_LINK_CONTROL_RSVD0)
#define V_ANC_LINK_CONTROL_RSVD0(V)                                               _MM_MAKEVALUE((V),S_ANC_LINK_CONTROL_RSVD0)
#define G_ANC_LINK_CONTROL_RSVD0(V)                                               _MM_GETVALUE((V),S_ANC_LINK_CONTROL_RSVD0,M_ANC_LINK_CONTROL_RSVD0)

      
#define S_ANC_LINK_STATUS_CMDQ_ACTIVE                                             0  //Access: read-only
#define M_ANC_LINK_STATUS_CMDQ_ACTIVE                                             _MM_MAKEMASK(1,S_ANC_LINK_STATUS_CMDQ_ACTIVE)
#define V_ANC_LINK_STATUS_CMDQ_ACTIVE(V)                                          _MM_MAKEVALUE((V),S_ANC_LINK_STATUS_CMDQ_ACTIVE)
#define G_ANC_LINK_STATUS_CMDQ_ACTIVE(V)                                          _MM_GETVALUE((V),S_ANC_LINK_STATUS_CMDQ_ACTIVE,M_ANC_LINK_STATUS_CMDQ_ACTIVE)
     
#define S_ANC_LINK_STATUS_FSM_ACTIVE                                              1  //Access: read-only
#define M_ANC_LINK_STATUS_FSM_ACTIVE                                              _MM_MAKEMASK(1,S_ANC_LINK_STATUS_FSM_ACTIVE)
#define V_ANC_LINK_STATUS_FSM_ACTIVE(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_STATUS_FSM_ACTIVE)
#define G_ANC_LINK_STATUS_FSM_ACTIVE(V)                                           _MM_GETVALUE((V),S_ANC_LINK_STATUS_FSM_ACTIVE,M_ANC_LINK_STATUS_FSM_ACTIVE)
     
#define S_ANC_LINK_STATUS_READ_HOLD_ACTIVE                                        2  //Access: read-only
#define M_ANC_LINK_STATUS_READ_HOLD_ACTIVE                                        _MM_MAKEMASK(1,S_ANC_LINK_STATUS_READ_HOLD_ACTIVE)
#define V_ANC_LINK_STATUS_READ_HOLD_ACTIVE(V)                                     _MM_MAKEVALUE((V),S_ANC_LINK_STATUS_READ_HOLD_ACTIVE)
#define G_ANC_LINK_STATUS_READ_HOLD_ACTIVE(V)                                     _MM_GETVALUE((V),S_ANC_LINK_STATUS_READ_HOLD_ACTIVE,M_ANC_LINK_STATUS_READ_HOLD_ACTIVE)
     
#define S_ANC_LINK_STATUS_RSVD0                                                   3  //Access: read-as-zero
#define M_ANC_LINK_STATUS_RSVD0                                                   _MM_MAKEMASK(5,S_ANC_LINK_STATUS_RSVD0)
#define V_ANC_LINK_STATUS_RSVD0(V)                                                _MM_MAKEVALUE((V),S_ANC_LINK_STATUS_RSVD0)
#define G_ANC_LINK_STATUS_RSVD0(V)                                                _MM_GETVALUE((V),S_ANC_LINK_STATUS_RSVD0,M_ANC_LINK_STATUS_RSVD0)
     
#define S_ANC_LINK_STATUS_CMDQ_STATE                                              8  //Access: read-only
#define M_ANC_LINK_STATUS_CMDQ_STATE                                              _MM_MAKEMASK(4,S_ANC_LINK_STATUS_CMDQ_STATE)
#define V_ANC_LINK_STATUS_CMDQ_STATE(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_STATUS_CMDQ_STATE)
#define G_ANC_LINK_STATUS_CMDQ_STATE(V)                                           _MM_GETVALUE((V),S_ANC_LINK_STATUS_CMDQ_STATE,M_ANC_LINK_STATUS_CMDQ_STATE)
     
#define S_ANC_LINK_STATUS_RSVD1                                                   12  //Access: read-as-zero
#define M_ANC_LINK_STATUS_RSVD1                                                   _MM_MAKEMASK(4,S_ANC_LINK_STATUS_RSVD1)
#define V_ANC_LINK_STATUS_RSVD1(V)                                                _MM_MAKEVALUE((V),S_ANC_LINK_STATUS_RSVD1)
#define G_ANC_LINK_STATUS_RSVD1(V)                                                _MM_GETVALUE((V),S_ANC_LINK_STATUS_RSVD1,M_ANC_LINK_STATUS_RSVD1)
     
#define S_ANC_LINK_STATUS_FSM_STATE                                               16  //Access: read-only
#define M_ANC_LINK_STATUS_FSM_STATE                                               _MM_MAKEMASK(6,S_ANC_LINK_STATUS_FSM_STATE)
#define V_ANC_LINK_STATUS_FSM_STATE(V)                                            _MM_MAKEVALUE((V),S_ANC_LINK_STATUS_FSM_STATE)
#define G_ANC_LINK_STATUS_FSM_STATE(V)                                            _MM_GETVALUE((V),S_ANC_LINK_STATUS_FSM_STATE,M_ANC_LINK_STATUS_FSM_STATE)
     
#define S_ANC_LINK_STATUS_RSVD2                                                   22  //Access: read-as-zero
#define M_ANC_LINK_STATUS_RSVD2                                                   _MM_MAKEMASK(10,S_ANC_LINK_STATUS_RSVD2)
#define V_ANC_LINK_STATUS_RSVD2(V)                                                _MM_MAKEVALUE((V),S_ANC_LINK_STATUS_RSVD2)
#define G_ANC_LINK_STATUS_RSVD2(V)                                                _MM_GETVALUE((V),S_ANC_LINK_STATUS_RSVD2,M_ANC_LINK_STATUS_RSVD2)

      
#define S_ANC_LINK_CHIP_ENABLE_CE                                                 0  //Access: read-write
#define M_ANC_LINK_CHIP_ENABLE_CE                                                 _MM_MAKEMASK(8,S_ANC_LINK_CHIP_ENABLE_CE)
#define V_ANC_LINK_CHIP_ENABLE_CE(V)                                              _MM_MAKEVALUE((V),S_ANC_LINK_CHIP_ENABLE_CE)
#define G_ANC_LINK_CHIP_ENABLE_CE(V)                                              _MM_GETVALUE((V),S_ANC_LINK_CHIP_ENABLE_CE,M_ANC_LINK_CHIP_ENABLE_CE)
     
#define S_ANC_LINK_CHIP_ENABLE_RSVD0                                              8  //Access: read-as-zero
#define M_ANC_LINK_CHIP_ENABLE_RSVD0                                              _MM_MAKEMASK(24,S_ANC_LINK_CHIP_ENABLE_RSVD0)
#define V_ANC_LINK_CHIP_ENABLE_RSVD0(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_CHIP_ENABLE_RSVD0)
#define G_ANC_LINK_CHIP_ENABLE_RSVD0(V)                                           _MM_GETVALUE((V),S_ANC_LINK_CHIP_ENABLE_RSVD0,M_ANC_LINK_CHIP_ENABLE_RSVD0)

      
#define S_ANC_LINK_READ_STATUS_CONFIG_POSITION                                    0  //Access: read-write
#define M_ANC_LINK_READ_STATUS_CONFIG_POSITION                                    _MM_MAKEMASK(8,S_ANC_LINK_READ_STATUS_CONFIG_POSITION)
#define V_ANC_LINK_READ_STATUS_CONFIG_POSITION(V)                                 _MM_MAKEVALUE((V),S_ANC_LINK_READ_STATUS_CONFIG_POSITION)
#define G_ANC_LINK_READ_STATUS_CONFIG_POSITION(V)                                 _MM_GETVALUE((V),S_ANC_LINK_READ_STATUS_CONFIG_POSITION,M_ANC_LINK_READ_STATUS_CONFIG_POSITION)
     
#define S_ANC_LINK_READ_STATUS_CONFIG_POLARITY                                    8  //Access: read-write
#define M_ANC_LINK_READ_STATUS_CONFIG_POLARITY                                    _MM_MAKEMASK(8,S_ANC_LINK_READ_STATUS_CONFIG_POLARITY)
#define V_ANC_LINK_READ_STATUS_CONFIG_POLARITY(V)                                 _MM_MAKEVALUE((V),S_ANC_LINK_READ_STATUS_CONFIG_POLARITY)
#define G_ANC_LINK_READ_STATUS_CONFIG_POLARITY(V)                                 _MM_GETVALUE((V),S_ANC_LINK_READ_STATUS_CONFIG_POLARITY,M_ANC_LINK_READ_STATUS_CONFIG_POLARITY)
     
#define S_ANC_LINK_READ_STATUS_CONFIG_STATUS_CAPTURE_DELAY                        16  //Access: read-write
#define M_ANC_LINK_READ_STATUS_CONFIG_STATUS_CAPTURE_DELAY                        _MM_MAKEMASK(4,S_ANC_LINK_READ_STATUS_CONFIG_STATUS_CAPTURE_DELAY)
#define V_ANC_LINK_READ_STATUS_CONFIG_STATUS_CAPTURE_DELAY(V)                     _MM_MAKEVALUE((V),S_ANC_LINK_READ_STATUS_CONFIG_STATUS_CAPTURE_DELAY)
#define G_ANC_LINK_READ_STATUS_CONFIG_STATUS_CAPTURE_DELAY(V)                     _MM_GETVALUE((V),S_ANC_LINK_READ_STATUS_CONFIG_STATUS_CAPTURE_DELAY,M_ANC_LINK_READ_STATUS_CONFIG_STATUS_CAPTURE_DELAY)
     
#define S_ANC_LINK_READ_STATUS_CONFIG_READY_SAMPLE_DELAY                          20  //Access: read-write
#define M_ANC_LINK_READ_STATUS_CONFIG_READY_SAMPLE_DELAY                          _MM_MAKEMASK(4,S_ANC_LINK_READ_STATUS_CONFIG_READY_SAMPLE_DELAY)
#define V_ANC_LINK_READ_STATUS_CONFIG_READY_SAMPLE_DELAY(V)                       _MM_MAKEVALUE((V),S_ANC_LINK_READ_STATUS_CONFIG_READY_SAMPLE_DELAY)
#define G_ANC_LINK_READ_STATUS_CONFIG_READY_SAMPLE_DELAY(V)                       _MM_GETVALUE((V),S_ANC_LINK_READ_STATUS_CONFIG_READY_SAMPLE_DELAY,M_ANC_LINK_READ_STATUS_CONFIG_READY_SAMPLE_DELAY)
     
#define S_ANC_LINK_READ_STATUS_CONFIG_RSVD0                                       24  //Access: read-as-zero
#define M_ANC_LINK_READ_STATUS_CONFIG_RSVD0                                       _MM_MAKEMASK(8,S_ANC_LINK_READ_STATUS_CONFIG_RSVD0)
#define V_ANC_LINK_READ_STATUS_CONFIG_RSVD0(V)                                    _MM_MAKEVALUE((V),S_ANC_LINK_READ_STATUS_CONFIG_RSVD0)
#define G_ANC_LINK_READ_STATUS_CONFIG_RSVD0(V)                                    _MM_GETVALUE((V),S_ANC_LINK_READ_STATUS_CONFIG_RSVD0,M_ANC_LINK_READ_STATUS_CONFIG_RSVD0)

      
#define S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_SETUP_TIME                     0  //Access: read-write
#define M_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_SETUP_TIME                     _MM_MAKEMASK(5,S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_SETUP_TIME)
#define V_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_SETUP_TIME(V)                  _MM_MAKEVALUE((V),S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_SETUP_TIME)
#define G_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_SETUP_TIME(V)                  _MM_GETVALUE((V),S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_SETUP_TIME,M_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_SETUP_TIME)
     
#define S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD0                             5  //Access: read-as-zero
#define M_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD0                             _MM_MAKEMASK(3,S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD0)
#define V_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD0(V)                          _MM_MAKEVALUE((V),S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD0)
#define G_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD0(V)                          _MM_GETVALUE((V),S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD0,M_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD0)
     
#define S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_HOLD_TIME                      8  //Access: read-write
#define M_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_HOLD_TIME                      _MM_MAKEMASK(5,S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_HOLD_TIME)
#define V_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_HOLD_TIME(V)                   _MM_MAKEVALUE((V),S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_HOLD_TIME)
#define G_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_HOLD_TIME(V)                   _MM_GETVALUE((V),S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_HOLD_TIME,M_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_CA_HOLD_TIME)
     
#define S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD1                             13  //Access: read-as-zero
#define M_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD1                             _MM_MAKEMASK(19,S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD1)
#define V_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD1(V)                          _MM_MAKEVALUE((V),S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD1)
#define G_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD1(V)                          _MM_GETVALUE((V),S_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD1,M_ANC_LINK_COMMAND_ADDRESS_PULSE_TIMING_RSVD1)

      
#define S_ANC_LINK_SDR_TIMING_CLE_ALE_SETUP_TIME                                  0  //Access: read-write
#define M_ANC_LINK_SDR_TIMING_CLE_ALE_SETUP_TIME                                  _MM_MAKEMASK(5,S_ANC_LINK_SDR_TIMING_CLE_ALE_SETUP_TIME)
#define V_ANC_LINK_SDR_TIMING_CLE_ALE_SETUP_TIME(V)                               _MM_MAKEVALUE((V),S_ANC_LINK_SDR_TIMING_CLE_ALE_SETUP_TIME)
#define G_ANC_LINK_SDR_TIMING_CLE_ALE_SETUP_TIME(V)                               _MM_GETVALUE((V),S_ANC_LINK_SDR_TIMING_CLE_ALE_SETUP_TIME,M_ANC_LINK_SDR_TIMING_CLE_ALE_SETUP_TIME)
     
#define S_ANC_LINK_SDR_TIMING_RSVD0                                               5  //Access: read-as-zero
#define M_ANC_LINK_SDR_TIMING_RSVD0                                               _MM_MAKEMASK(3,S_ANC_LINK_SDR_TIMING_RSVD0)
#define V_ANC_LINK_SDR_TIMING_RSVD0(V)                                            _MM_MAKEVALUE((V),S_ANC_LINK_SDR_TIMING_RSVD0)
#define G_ANC_LINK_SDR_TIMING_RSVD0(V)                                            _MM_GETVALUE((V),S_ANC_LINK_SDR_TIMING_RSVD0,M_ANC_LINK_SDR_TIMING_RSVD0)
     
#define S_ANC_LINK_SDR_TIMING_CLE_ALE_HOLD_TIME                                   8  //Access: read-write
#define M_ANC_LINK_SDR_TIMING_CLE_ALE_HOLD_TIME                                   _MM_MAKEMASK(5,S_ANC_LINK_SDR_TIMING_CLE_ALE_HOLD_TIME)
#define V_ANC_LINK_SDR_TIMING_CLE_ALE_HOLD_TIME(V)                                _MM_MAKEVALUE((V),S_ANC_LINK_SDR_TIMING_CLE_ALE_HOLD_TIME)
#define G_ANC_LINK_SDR_TIMING_CLE_ALE_HOLD_TIME(V)                                _MM_GETVALUE((V),S_ANC_LINK_SDR_TIMING_CLE_ALE_HOLD_TIME,M_ANC_LINK_SDR_TIMING_CLE_ALE_HOLD_TIME)
     
#define S_ANC_LINK_SDR_TIMING_RSVD1                                               13  //Access: read-as-zero
#define M_ANC_LINK_SDR_TIMING_RSVD1                                               _MM_MAKEMASK(3,S_ANC_LINK_SDR_TIMING_RSVD1)
#define V_ANC_LINK_SDR_TIMING_RSVD1(V)                                            _MM_MAKEVALUE((V),S_ANC_LINK_SDR_TIMING_RSVD1)
#define G_ANC_LINK_SDR_TIMING_RSVD1(V)                                            _MM_GETVALUE((V),S_ANC_LINK_SDR_TIMING_RSVD1,M_ANC_LINK_SDR_TIMING_RSVD1)
     
#define S_ANC_LINK_SDR_TIMING_DATA_CAPTURE_DELAY                                  16  //Access: read-write
#define M_ANC_LINK_SDR_TIMING_DATA_CAPTURE_DELAY                                  _MM_MAKEMASK(5,S_ANC_LINK_SDR_TIMING_DATA_CAPTURE_DELAY)
#define V_ANC_LINK_SDR_TIMING_DATA_CAPTURE_DELAY(V)                               _MM_MAKEVALUE((V),S_ANC_LINK_SDR_TIMING_DATA_CAPTURE_DELAY)
#define G_ANC_LINK_SDR_TIMING_DATA_CAPTURE_DELAY(V)                               _MM_GETVALUE((V),S_ANC_LINK_SDR_TIMING_DATA_CAPTURE_DELAY,M_ANC_LINK_SDR_TIMING_DATA_CAPTURE_DELAY)
     
#define S_ANC_LINK_SDR_TIMING_RSVD2                                               21  //Access: read-as-zero
#define M_ANC_LINK_SDR_TIMING_RSVD2                                               _MM_MAKEMASK(11,S_ANC_LINK_SDR_TIMING_RSVD2)
#define V_ANC_LINK_SDR_TIMING_RSVD2(V)                                            _MM_MAKEVALUE((V),S_ANC_LINK_SDR_TIMING_RSVD2)
#define G_ANC_LINK_SDR_TIMING_RSVD2(V)                                            _MM_GETVALUE((V),S_ANC_LINK_SDR_TIMING_RSVD2,M_ANC_LINK_SDR_TIMING_RSVD2)

      
#define S_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME                                 0  //Access: read-write
#define M_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME                                 _MM_MAKEMASK(5,S_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME)
#define V_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME(V)                              _MM_MAKEVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME)
#define G_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME(V)                              _MM_GETVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME,M_ANC_LINK_SDR_DATA_TIMING_REN_SETUP_TIME)
     
#define S_ANC_LINK_SDR_DATA_TIMING_RSVD0                                          5  //Access: read-as-zero
#define M_ANC_LINK_SDR_DATA_TIMING_RSVD0                                          _MM_MAKEMASK(3,S_ANC_LINK_SDR_DATA_TIMING_RSVD0)
#define V_ANC_LINK_SDR_DATA_TIMING_RSVD0(V)                                       _MM_MAKEVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_RSVD0)
#define G_ANC_LINK_SDR_DATA_TIMING_RSVD0(V)                                       _MM_GETVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_RSVD0,M_ANC_LINK_SDR_DATA_TIMING_RSVD0)
     
#define S_ANC_LINK_SDR_DATA_TIMING_REN_HOLD_TIME                                  8  //Access: read-write
#define M_ANC_LINK_SDR_DATA_TIMING_REN_HOLD_TIME                                  _MM_MAKEMASK(5,S_ANC_LINK_SDR_DATA_TIMING_REN_HOLD_TIME)
#define V_ANC_LINK_SDR_DATA_TIMING_REN_HOLD_TIME(V)                               _MM_MAKEVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_REN_HOLD_TIME)
#define G_ANC_LINK_SDR_DATA_TIMING_REN_HOLD_TIME(V)                               _MM_GETVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_REN_HOLD_TIME,M_ANC_LINK_SDR_DATA_TIMING_REN_HOLD_TIME)
     
#define S_ANC_LINK_SDR_DATA_TIMING_RSVD1                                          13  //Access: read-as-zero
#define M_ANC_LINK_SDR_DATA_TIMING_RSVD1                                          _MM_MAKEMASK(3,S_ANC_LINK_SDR_DATA_TIMING_RSVD1)
#define V_ANC_LINK_SDR_DATA_TIMING_RSVD1(V)                                       _MM_MAKEVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_RSVD1)
#define G_ANC_LINK_SDR_DATA_TIMING_RSVD1(V)                                       _MM_GETVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_RSVD1,M_ANC_LINK_SDR_DATA_TIMING_RSVD1)
     
#define S_ANC_LINK_SDR_DATA_TIMING_WEN_SETUP_TIME                                 16  //Access: read-write
#define M_ANC_LINK_SDR_DATA_TIMING_WEN_SETUP_TIME                                 _MM_MAKEMASK(5,S_ANC_LINK_SDR_DATA_TIMING_WEN_SETUP_TIME)
#define V_ANC_LINK_SDR_DATA_TIMING_WEN_SETUP_TIME(V)                              _MM_MAKEVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_WEN_SETUP_TIME)
#define G_ANC_LINK_SDR_DATA_TIMING_WEN_SETUP_TIME(V)                              _MM_GETVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_WEN_SETUP_TIME,M_ANC_LINK_SDR_DATA_TIMING_WEN_SETUP_TIME)
     
#define S_ANC_LINK_SDR_DATA_TIMING_RSVD2                                          21  //Access: read-as-zero
#define M_ANC_LINK_SDR_DATA_TIMING_RSVD2                                          _MM_MAKEMASK(3,S_ANC_LINK_SDR_DATA_TIMING_RSVD2)
#define V_ANC_LINK_SDR_DATA_TIMING_RSVD2(V)                                       _MM_MAKEVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_RSVD2)
#define G_ANC_LINK_SDR_DATA_TIMING_RSVD2(V)                                       _MM_GETVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_RSVD2,M_ANC_LINK_SDR_DATA_TIMING_RSVD2)
     
#define S_ANC_LINK_SDR_DATA_TIMING_WEN_HOLD_TIME                                  24  //Access: read-write
#define M_ANC_LINK_SDR_DATA_TIMING_WEN_HOLD_TIME                                  _MM_MAKEMASK(5,S_ANC_LINK_SDR_DATA_TIMING_WEN_HOLD_TIME)
#define V_ANC_LINK_SDR_DATA_TIMING_WEN_HOLD_TIME(V)                               _MM_MAKEVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_WEN_HOLD_TIME)
#define G_ANC_LINK_SDR_DATA_TIMING_WEN_HOLD_TIME(V)                               _MM_GETVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_WEN_HOLD_TIME,M_ANC_LINK_SDR_DATA_TIMING_WEN_HOLD_TIME)
     
#define S_ANC_LINK_SDR_DATA_TIMING_RSVD3                                          29  //Access: read-as-zero
#define M_ANC_LINK_SDR_DATA_TIMING_RSVD3                                          _MM_MAKEMASK(3,S_ANC_LINK_SDR_DATA_TIMING_RSVD3)
#define V_ANC_LINK_SDR_DATA_TIMING_RSVD3(V)                                       _MM_MAKEVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_RSVD3)
#define G_ANC_LINK_SDR_DATA_TIMING_RSVD3(V)                                       _MM_GETVALUE((V),S_ANC_LINK_SDR_DATA_TIMING_RSVD3,M_ANC_LINK_SDR_DATA_TIMING_RSVD3)

      
#define S_ANC_LINK_DDR_DATA_TIMING_DDR_PULSE_WIDTH                                0  //Access: read-write
#define M_ANC_LINK_DDR_DATA_TIMING_DDR_PULSE_WIDTH                                _MM_MAKEMASK(5,S_ANC_LINK_DDR_DATA_TIMING_DDR_PULSE_WIDTH)
#define V_ANC_LINK_DDR_DATA_TIMING_DDR_PULSE_WIDTH(V)                             _MM_MAKEVALUE((V),S_ANC_LINK_DDR_DATA_TIMING_DDR_PULSE_WIDTH)
#define G_ANC_LINK_DDR_DATA_TIMING_DDR_PULSE_WIDTH(V)                             _MM_GETVALUE((V),S_ANC_LINK_DDR_DATA_TIMING_DDR_PULSE_WIDTH,M_ANC_LINK_DDR_DATA_TIMING_DDR_PULSE_WIDTH)
     
#define S_ANC_LINK_DDR_DATA_TIMING_RSVD0                                          5  //Access: read-as-zero
#define M_ANC_LINK_DDR_DATA_TIMING_RSVD0                                          _MM_MAKEMASK(27,S_ANC_LINK_DDR_DATA_TIMING_RSVD0)
#define V_ANC_LINK_DDR_DATA_TIMING_RSVD0(V)                                       _MM_MAKEVALUE((V),S_ANC_LINK_DDR_DATA_TIMING_RSVD0)
#define G_ANC_LINK_DDR_DATA_TIMING_RSVD0(V)                                       _MM_GETVALUE((V),S_ANC_LINK_DDR_DATA_TIMING_RSVD0,M_ANC_LINK_DDR_DATA_TIMING_RSVD0)

      
#define S_ANC_LINK_DDR_READ_TIMING_READ_PREAMBLE                                  0  //Access: read-write
#define M_ANC_LINK_DDR_READ_TIMING_READ_PREAMBLE                                  _MM_MAKEMASK(8,S_ANC_LINK_DDR_READ_TIMING_READ_PREAMBLE)
#define V_ANC_LINK_DDR_READ_TIMING_READ_PREAMBLE(V)                               _MM_MAKEVALUE((V),S_ANC_LINK_DDR_READ_TIMING_READ_PREAMBLE)
#define G_ANC_LINK_DDR_READ_TIMING_READ_PREAMBLE(V)                               _MM_GETVALUE((V),S_ANC_LINK_DDR_READ_TIMING_READ_PREAMBLE,M_ANC_LINK_DDR_READ_TIMING_READ_PREAMBLE)
     
#define S_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE                                 8  //Access: read-write
#define M_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE                                 _MM_MAKEMASK(8,S_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE)
#define V_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE(V)                              _MM_MAKEVALUE((V),S_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE)
#define G_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE(V)                              _MM_GETVALUE((V),S_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE,M_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE)
     
#define S_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE_HOLD                            16  //Access: read-write
#define M_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE_HOLD                            _MM_MAKEMASK(8,S_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE_HOLD)
#define V_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE_HOLD(V)                         _MM_MAKEVALUE((V),S_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE_HOLD)
#define G_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE_HOLD(V)                         _MM_GETVALUE((V),S_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE_HOLD,M_ANC_LINK_DDR_READ_TIMING_READ_POSTAMBLE_HOLD)
     
#define S_ANC_LINK_DDR_READ_TIMING_READ_TURNAROUND                                24  //Access: read-write
#define M_ANC_LINK_DDR_READ_TIMING_READ_TURNAROUND                                _MM_MAKEMASK(8,S_ANC_LINK_DDR_READ_TIMING_READ_TURNAROUND)
#define V_ANC_LINK_DDR_READ_TIMING_READ_TURNAROUND(V)                             _MM_MAKEVALUE((V),S_ANC_LINK_DDR_READ_TIMING_READ_TURNAROUND)
#define G_ANC_LINK_DDR_READ_TIMING_READ_TURNAROUND(V)                             _MM_GETVALUE((V),S_ANC_LINK_DDR_READ_TIMING_READ_TURNAROUND,M_ANC_LINK_DDR_READ_TIMING_READ_TURNAROUND)

      
#define S_ANC_LINK_DDR_WRITE_TIMING_WRITE_PREAMBLE                                0  //Access: read-write
#define M_ANC_LINK_DDR_WRITE_TIMING_WRITE_PREAMBLE                                _MM_MAKEMASK(8,S_ANC_LINK_DDR_WRITE_TIMING_WRITE_PREAMBLE)
#define V_ANC_LINK_DDR_WRITE_TIMING_WRITE_PREAMBLE(V)                             _MM_MAKEVALUE((V),S_ANC_LINK_DDR_WRITE_TIMING_WRITE_PREAMBLE)
#define G_ANC_LINK_DDR_WRITE_TIMING_WRITE_PREAMBLE(V)                             _MM_GETVALUE((V),S_ANC_LINK_DDR_WRITE_TIMING_WRITE_PREAMBLE,M_ANC_LINK_DDR_WRITE_TIMING_WRITE_PREAMBLE)
     
#define S_ANC_LINK_DDR_WRITE_TIMING_RSVD0                                         8  //Access: read-as-zero
#define M_ANC_LINK_DDR_WRITE_TIMING_RSVD0                                         _MM_MAKEMASK(8,S_ANC_LINK_DDR_WRITE_TIMING_RSVD0)
#define V_ANC_LINK_DDR_WRITE_TIMING_RSVD0(V)                                      _MM_MAKEVALUE((V),S_ANC_LINK_DDR_WRITE_TIMING_RSVD0)
#define G_ANC_LINK_DDR_WRITE_TIMING_RSVD0(V)                                      _MM_GETVALUE((V),S_ANC_LINK_DDR_WRITE_TIMING_RSVD0,M_ANC_LINK_DDR_WRITE_TIMING_RSVD0)
     
#define S_ANC_LINK_DDR_WRITE_TIMING_WRITE_POSTAMBLE                               16  //Access: read-write
#define M_ANC_LINK_DDR_WRITE_TIMING_WRITE_POSTAMBLE                               _MM_MAKEMASK(8,S_ANC_LINK_DDR_WRITE_TIMING_WRITE_POSTAMBLE)
#define V_ANC_LINK_DDR_WRITE_TIMING_WRITE_POSTAMBLE(V)                            _MM_MAKEVALUE((V),S_ANC_LINK_DDR_WRITE_TIMING_WRITE_POSTAMBLE)
#define G_ANC_LINK_DDR_WRITE_TIMING_WRITE_POSTAMBLE(V)                            _MM_GETVALUE((V),S_ANC_LINK_DDR_WRITE_TIMING_WRITE_POSTAMBLE,M_ANC_LINK_DDR_WRITE_TIMING_WRITE_POSTAMBLE)
     
#define S_ANC_LINK_DDR_WRITE_TIMING_RSVD1                                         24  //Access: read-as-zero
#define M_ANC_LINK_DDR_WRITE_TIMING_RSVD1                                         _MM_MAKEMASK(8,S_ANC_LINK_DDR_WRITE_TIMING_RSVD1)
#define V_ANC_LINK_DDR_WRITE_TIMING_RSVD1(V)                                      _MM_MAKEVALUE((V),S_ANC_LINK_DDR_WRITE_TIMING_RSVD1)
#define G_ANC_LINK_DDR_WRITE_TIMING_RSVD1(V)                                      _MM_GETVALUE((V),S_ANC_LINK_DDR_WRITE_TIMING_RSVD1,M_ANC_LINK_DDR_WRITE_TIMING_RSVD1)

      
#define S_ANC_LINK_DQS_TIMING_WRITE_DQS_DELAY                                     0  //Access: read-write
#define M_ANC_LINK_DQS_TIMING_WRITE_DQS_DELAY                                     _MM_MAKEMASK(6,S_ANC_LINK_DQS_TIMING_WRITE_DQS_DELAY)
#define V_ANC_LINK_DQS_TIMING_WRITE_DQS_DELAY(V)                                  _MM_MAKEVALUE((V),S_ANC_LINK_DQS_TIMING_WRITE_DQS_DELAY)
#define G_ANC_LINK_DQS_TIMING_WRITE_DQS_DELAY(V)                                  _MM_GETVALUE((V),S_ANC_LINK_DQS_TIMING_WRITE_DQS_DELAY,M_ANC_LINK_DQS_TIMING_WRITE_DQS_DELAY)
     
#define S_ANC_LINK_DQS_TIMING_RSVD0                                               6  //Access: read-as-zero
#define M_ANC_LINK_DQS_TIMING_RSVD0                                               _MM_MAKEMASK(26,S_ANC_LINK_DQS_TIMING_RSVD0)
#define V_ANC_LINK_DQS_TIMING_RSVD0(V)                                            _MM_MAKEVALUE((V),S_ANC_LINK_DQS_TIMING_RSVD0)
#define G_ANC_LINK_DQS_TIMING_RSVD0(V)                                            _MM_GETVALUE((V),S_ANC_LINK_DQS_TIMING_RSVD0,M_ANC_LINK_DQS_TIMING_RSVD0)

      
#define S_ANC_LINK_CMDQ_COUNT_TOTAL                                               0  //Access: read-only
#define M_ANC_LINK_CMDQ_COUNT_TOTAL                                               _MM_MAKEMASK(32,S_ANC_LINK_CMDQ_COUNT_TOTAL)
#define V_ANC_LINK_CMDQ_COUNT_TOTAL(V)                                            _MM_MAKEVALUE((V),S_ANC_LINK_CMDQ_COUNT_TOTAL)
#define G_ANC_LINK_CMDQ_COUNT_TOTAL(V)                                            _MM_GETVALUE((V),S_ANC_LINK_CMDQ_COUNT_TOTAL,M_ANC_LINK_CMDQ_COUNT_TOTAL)

      
#define S_ANC_LINK_BYPASS_COUNT_TOTAL                                             0  //Access: read-only
#define M_ANC_LINK_BYPASS_COUNT_TOTAL                                             _MM_MAKEMASK(32,S_ANC_LINK_BYPASS_COUNT_TOTAL)
#define V_ANC_LINK_BYPASS_COUNT_TOTAL(V)                                          _MM_MAKEVALUE((V),S_ANC_LINK_BYPASS_COUNT_TOTAL)
#define G_ANC_LINK_BYPASS_COUNT_TOTAL(V)                                          _MM_GETVALUE((V),S_ANC_LINK_BYPASS_COUNT_TOTAL,M_ANC_LINK_BYPASS_COUNT_TOTAL)

      
#define S_ANC_LINK_CMD_TIMEOUT_VALUE                                              0  //Access: read-write
#define M_ANC_LINK_CMD_TIMEOUT_VALUE                                              _MM_MAKEMASK(31,S_ANC_LINK_CMD_TIMEOUT_VALUE)
#define V_ANC_LINK_CMD_TIMEOUT_VALUE(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_CMD_TIMEOUT_VALUE)
#define G_ANC_LINK_CMD_TIMEOUT_VALUE(V)                                           _MM_GETVALUE((V),S_ANC_LINK_CMD_TIMEOUT_VALUE,M_ANC_LINK_CMD_TIMEOUT_VALUE)
     
#define S_ANC_LINK_CMD_TIMEOUT_ENABLE                                             31  //Access: read-write
#define M_ANC_LINK_CMD_TIMEOUT_ENABLE                                             _MM_MAKEMASK(1,S_ANC_LINK_CMD_TIMEOUT_ENABLE)
#define V_ANC_LINK_CMD_TIMEOUT_ENABLE(V)                                          _MM_MAKEVALUE((V),S_ANC_LINK_CMD_TIMEOUT_ENABLE)
#define G_ANC_LINK_CMD_TIMEOUT_ENABLE(V)                                          _MM_GETVALUE((V),S_ANC_LINK_CMD_TIMEOUT_ENABLE,M_ANC_LINK_CMD_TIMEOUT_ENABLE)

      
#define S_ANC_LINK_CMDQ_INT_CODE_CODE                                             0  //Access: read-only
#define M_ANC_LINK_CMDQ_INT_CODE_CODE                                             _MM_MAKEMASK(16,S_ANC_LINK_CMDQ_INT_CODE_CODE)
#define V_ANC_LINK_CMDQ_INT_CODE_CODE(V)                                          _MM_MAKEVALUE((V),S_ANC_LINK_CMDQ_INT_CODE_CODE)
#define G_ANC_LINK_CMDQ_INT_CODE_CODE(V)                                          _MM_GETVALUE((V),S_ANC_LINK_CMDQ_INT_CODE_CODE,M_ANC_LINK_CMDQ_INT_CODE_CODE)
     
#define S_ANC_LINK_CMDQ_INT_CODE_RSVD0                                            16  //Access: read-as-zero
#define M_ANC_LINK_CMDQ_INT_CODE_RSVD0                                            _MM_MAKEMASK(16,S_ANC_LINK_CMDQ_INT_CODE_RSVD0)
#define V_ANC_LINK_CMDQ_INT_CODE_RSVD0(V)                                         _MM_MAKEVALUE((V),S_ANC_LINK_CMDQ_INT_CODE_RSVD0)
#define G_ANC_LINK_CMDQ_INT_CODE_RSVD0(V)                                         _MM_GETVALUE((V),S_ANC_LINK_CMDQ_INT_CODE_RSVD0,M_ANC_LINK_CMDQ_INT_CODE_RSVD0)

      
#define S_ANC_LINK_TIMER_COUNT                                                    0  //Access: read-only
#define M_ANC_LINK_TIMER_COUNT                                                    _MM_MAKEMASK(32,S_ANC_LINK_TIMER_COUNT)
#define V_ANC_LINK_TIMER_COUNT(V)                                                 _MM_MAKEVALUE((V),S_ANC_LINK_TIMER_COUNT)
#define G_ANC_LINK_TIMER_COUNT(V)                                                 _MM_GETVALUE((V),S_ANC_LINK_TIMER_COUNT,M_ANC_LINK_TIMER_COUNT)

      
#define S_ANC_LINK_CRC_PARITY_DATA                                                0  //Access: read-only
#define M_ANC_LINK_CRC_PARITY_DATA                                                _MM_MAKEMASK(32,S_ANC_LINK_CRC_PARITY_DATA)
#define V_ANC_LINK_CRC_PARITY_DATA(V)                                             _MM_MAKEVALUE((V),S_ANC_LINK_CRC_PARITY_DATA)
#define G_ANC_LINK_CRC_PARITY_DATA(V)                                             _MM_GETVALUE((V),S_ANC_LINK_CRC_PARITY_DATA,M_ANC_LINK_CRC_PARITY_DATA)

      
#define S_ANC_LINK_NPL_INTERFACE_CLE                                              0  //Access: read-only
#define M_ANC_LINK_NPL_INTERFACE_CLE                                              _MM_MAKEMASK(1,S_ANC_LINK_NPL_INTERFACE_CLE)
#define V_ANC_LINK_NPL_INTERFACE_CLE(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_CLE)
#define G_ANC_LINK_NPL_INTERFACE_CLE(V)                                           _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_CLE,M_ANC_LINK_NPL_INTERFACE_CLE)
     
#define S_ANC_LINK_NPL_INTERFACE_ALE                                              1  //Access: read-only
#define M_ANC_LINK_NPL_INTERFACE_ALE                                              _MM_MAKEMASK(1,S_ANC_LINK_NPL_INTERFACE_ALE)
#define V_ANC_LINK_NPL_INTERFACE_ALE(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_ALE)
#define G_ANC_LINK_NPL_INTERFACE_ALE(V)                                           _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_ALE,M_ANC_LINK_NPL_INTERFACE_ALE)
     
#define S_ANC_LINK_NPL_INTERFACE_REN                                              2  //Access: read-only
#define M_ANC_LINK_NPL_INTERFACE_REN                                              _MM_MAKEMASK(1,S_ANC_LINK_NPL_INTERFACE_REN)
#define V_ANC_LINK_NPL_INTERFACE_REN(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_REN)
#define G_ANC_LINK_NPL_INTERFACE_REN(V)                                           _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_REN,M_ANC_LINK_NPL_INTERFACE_REN)
     
#define S_ANC_LINK_NPL_INTERFACE_WEN                                              3  //Access: read-only
#define M_ANC_LINK_NPL_INTERFACE_WEN                                              _MM_MAKEMASK(1,S_ANC_LINK_NPL_INTERFACE_WEN)
#define V_ANC_LINK_NPL_INTERFACE_WEN(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_WEN)
#define G_ANC_LINK_NPL_INTERFACE_WEN(V)                                           _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_WEN,M_ANC_LINK_NPL_INTERFACE_WEN)
     
#define S_ANC_LINK_NPL_INTERFACE_DATA_OUT                                         4  //Access: read-only
#define M_ANC_LINK_NPL_INTERFACE_DATA_OUT                                         _MM_MAKEMASK(8,S_ANC_LINK_NPL_INTERFACE_DATA_OUT)
#define V_ANC_LINK_NPL_INTERFACE_DATA_OUT(V)                                      _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_DATA_OUT)
#define G_ANC_LINK_NPL_INTERFACE_DATA_OUT(V)                                      _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_DATA_OUT,M_ANC_LINK_NPL_INTERFACE_DATA_OUT)
     
#define S_ANC_LINK_NPL_INTERFACE_DATA_OUT_ENABLE                                  12  //Access: read-only
#define M_ANC_LINK_NPL_INTERFACE_DATA_OUT_ENABLE                                  _MM_MAKEMASK(1,S_ANC_LINK_NPL_INTERFACE_DATA_OUT_ENABLE)
#define V_ANC_LINK_NPL_INTERFACE_DATA_OUT_ENABLE(V)                               _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_DATA_OUT_ENABLE)
#define G_ANC_LINK_NPL_INTERFACE_DATA_OUT_ENABLE(V)                               _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_DATA_OUT_ENABLE,M_ANC_LINK_NPL_INTERFACE_DATA_OUT_ENABLE)
     
#define S_ANC_LINK_NPL_INTERFACE_WRITE_DQS                                        13  //Access: read-only
#define M_ANC_LINK_NPL_INTERFACE_WRITE_DQS                                        _MM_MAKEMASK(1,S_ANC_LINK_NPL_INTERFACE_WRITE_DQS)
#define V_ANC_LINK_NPL_INTERFACE_WRITE_DQS(V)                                     _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_WRITE_DQS)
#define G_ANC_LINK_NPL_INTERFACE_WRITE_DQS(V)                                     _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_WRITE_DQS,M_ANC_LINK_NPL_INTERFACE_WRITE_DQS)
     
#define S_ANC_LINK_NPL_INTERFACE_WRITE_DQS_ENABLE                                 14  //Access: read-only
#define M_ANC_LINK_NPL_INTERFACE_WRITE_DQS_ENABLE                                 _MM_MAKEMASK(1,S_ANC_LINK_NPL_INTERFACE_WRITE_DQS_ENABLE)
#define V_ANC_LINK_NPL_INTERFACE_WRITE_DQS_ENABLE(V)                              _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_WRITE_DQS_ENABLE)
#define G_ANC_LINK_NPL_INTERFACE_WRITE_DQS_ENABLE(V)                              _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_WRITE_DQS_ENABLE,M_ANC_LINK_NPL_INTERFACE_WRITE_DQS_ENABLE)
     
#define S_ANC_LINK_NPL_INTERFACE_PULLDOWN                                         15  //Access: read-only
#define M_ANC_LINK_NPL_INTERFACE_PULLDOWN                                         _MM_MAKEMASK(1,S_ANC_LINK_NPL_INTERFACE_PULLDOWN)
#define V_ANC_LINK_NPL_INTERFACE_PULLDOWN(V)                                      _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_PULLDOWN)
#define G_ANC_LINK_NPL_INTERFACE_PULLDOWN(V)                                      _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_PULLDOWN,M_ANC_LINK_NPL_INTERFACE_PULLDOWN)
     
#define S_ANC_LINK_NPL_INTERFACE_DATA_IN                                          16  //Access: read-only
#define M_ANC_LINK_NPL_INTERFACE_DATA_IN                                          _MM_MAKEMASK(8,S_ANC_LINK_NPL_INTERFACE_DATA_IN)
#define V_ANC_LINK_NPL_INTERFACE_DATA_IN(V)                                       _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_DATA_IN)
#define G_ANC_LINK_NPL_INTERFACE_DATA_IN(V)                                       _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_DATA_IN,M_ANC_LINK_NPL_INTERFACE_DATA_IN)
     
#define S_ANC_LINK_NPL_INTERFACE_RSVD0                                            24  //Access: read-as-zero
#define M_ANC_LINK_NPL_INTERFACE_RSVD0                                            _MM_MAKEMASK(8,S_ANC_LINK_NPL_INTERFACE_RSVD0)
#define V_ANC_LINK_NPL_INTERFACE_RSVD0(V)                                         _MM_MAKEVALUE((V),S_ANC_LINK_NPL_INTERFACE_RSVD0)
#define G_ANC_LINK_NPL_INTERFACE_RSVD0(V)                                         _MM_GETVALUE((V),S_ANC_LINK_NPL_INTERFACE_RSVD0,M_ANC_LINK_NPL_INTERFACE_RSVD0)

      
#define S_ANC_LINK_MACRO_STATUS_ADDRESS                                           0  //Access: read-only
#define M_ANC_LINK_MACRO_STATUS_ADDRESS                                           _MM_MAKEMASK(8,S_ANC_LINK_MACRO_STATUS_ADDRESS)
#define V_ANC_LINK_MACRO_STATUS_ADDRESS(V)                                        _MM_MAKEVALUE((V),S_ANC_LINK_MACRO_STATUS_ADDRESS)
#define G_ANC_LINK_MACRO_STATUS_ADDRESS(V)                                        _MM_GETVALUE((V),S_ANC_LINK_MACRO_STATUS_ADDRESS,M_ANC_LINK_MACRO_STATUS_ADDRESS)
     
#define S_ANC_LINK_MACRO_STATUS_WORD_COUNT                                        8  //Access: read-only
#define M_ANC_LINK_MACRO_STATUS_WORD_COUNT                                        _MM_MAKEMASK(8,S_ANC_LINK_MACRO_STATUS_WORD_COUNT)
#define V_ANC_LINK_MACRO_STATUS_WORD_COUNT(V)                                     _MM_MAKEVALUE((V),S_ANC_LINK_MACRO_STATUS_WORD_COUNT)
#define G_ANC_LINK_MACRO_STATUS_WORD_COUNT(V)                                     _MM_GETVALUE((V),S_ANC_LINK_MACRO_STATUS_WORD_COUNT,M_ANC_LINK_MACRO_STATUS_WORD_COUNT)
     
#define S_ANC_LINK_MACRO_STATUS_LOOP_COUNT                                        16  //Access: read-only
#define M_ANC_LINK_MACRO_STATUS_LOOP_COUNT                                        _MM_MAKEMASK(8,S_ANC_LINK_MACRO_STATUS_LOOP_COUNT)
#define V_ANC_LINK_MACRO_STATUS_LOOP_COUNT(V)                                     _MM_MAKEVALUE((V),S_ANC_LINK_MACRO_STATUS_LOOP_COUNT)
#define G_ANC_LINK_MACRO_STATUS_LOOP_COUNT(V)                                     _MM_GETVALUE((V),S_ANC_LINK_MACRO_STATUS_LOOP_COUNT,M_ANC_LINK_MACRO_STATUS_LOOP_COUNT)
     
#define S_ANC_LINK_MACRO_STATUS_ACTIVE                                            24  //Access: read-only
#define M_ANC_LINK_MACRO_STATUS_ACTIVE                                            _MM_MAKEMASK(1,S_ANC_LINK_MACRO_STATUS_ACTIVE)
#define V_ANC_LINK_MACRO_STATUS_ACTIVE(V)                                         _MM_MAKEVALUE((V),S_ANC_LINK_MACRO_STATUS_ACTIVE)
#define G_ANC_LINK_MACRO_STATUS_ACTIVE(V)                                         _MM_GETVALUE((V),S_ANC_LINK_MACRO_STATUS_ACTIVE,M_ANC_LINK_MACRO_STATUS_ACTIVE)
     
#define S_ANC_LINK_MACRO_STATUS_RSVD0                                             25  //Access: read-as-zero
#define M_ANC_LINK_MACRO_STATUS_RSVD0                                             _MM_MAKEMASK(7,S_ANC_LINK_MACRO_STATUS_RSVD0)
#define V_ANC_LINK_MACRO_STATUS_RSVD0(V)                                          _MM_MAKEVALUE((V),S_ANC_LINK_MACRO_STATUS_RSVD0)
#define G_ANC_LINK_MACRO_STATUS_RSVD0(V)                                          _MM_GETVALUE((V),S_ANC_LINK_MACRO_STATUS_RSVD0,M_ANC_LINK_MACRO_STATUS_RSVD0)

      
#define S_ANC_LINK_BYPASS_MACRO_STATUS_ADDRESS                                    0  //Access: read-only
#define M_ANC_LINK_BYPASS_MACRO_STATUS_ADDRESS                                    _MM_MAKEMASK(8,S_ANC_LINK_BYPASS_MACRO_STATUS_ADDRESS)
#define V_ANC_LINK_BYPASS_MACRO_STATUS_ADDRESS(V)                                 _MM_MAKEVALUE((V),S_ANC_LINK_BYPASS_MACRO_STATUS_ADDRESS)
#define G_ANC_LINK_BYPASS_MACRO_STATUS_ADDRESS(V)                                 _MM_GETVALUE((V),S_ANC_LINK_BYPASS_MACRO_STATUS_ADDRESS,M_ANC_LINK_BYPASS_MACRO_STATUS_ADDRESS)
     
#define S_ANC_LINK_BYPASS_MACRO_STATUS_WORD_COUNT                                 8  //Access: read-only
#define M_ANC_LINK_BYPASS_MACRO_STATUS_WORD_COUNT                                 _MM_MAKEMASK(8,S_ANC_LINK_BYPASS_MACRO_STATUS_WORD_COUNT)
#define V_ANC_LINK_BYPASS_MACRO_STATUS_WORD_COUNT(V)                              _MM_MAKEVALUE((V),S_ANC_LINK_BYPASS_MACRO_STATUS_WORD_COUNT)
#define G_ANC_LINK_BYPASS_MACRO_STATUS_WORD_COUNT(V)                              _MM_GETVALUE((V),S_ANC_LINK_BYPASS_MACRO_STATUS_WORD_COUNT,M_ANC_LINK_BYPASS_MACRO_STATUS_WORD_COUNT)
     
#define S_ANC_LINK_BYPASS_MACRO_STATUS_LOOP_COUNT                                 16  //Access: read-only
#define M_ANC_LINK_BYPASS_MACRO_STATUS_LOOP_COUNT                                 _MM_MAKEMASK(8,S_ANC_LINK_BYPASS_MACRO_STATUS_LOOP_COUNT)
#define V_ANC_LINK_BYPASS_MACRO_STATUS_LOOP_COUNT(V)                              _MM_MAKEVALUE((V),S_ANC_LINK_BYPASS_MACRO_STATUS_LOOP_COUNT)
#define G_ANC_LINK_BYPASS_MACRO_STATUS_LOOP_COUNT(V)                              _MM_GETVALUE((V),S_ANC_LINK_BYPASS_MACRO_STATUS_LOOP_COUNT,M_ANC_LINK_BYPASS_MACRO_STATUS_LOOP_COUNT)
     
#define S_ANC_LINK_BYPASS_MACRO_STATUS_ACTIVE                                     24  //Access: read-only
#define M_ANC_LINK_BYPASS_MACRO_STATUS_ACTIVE                                     _MM_MAKEMASK(1,S_ANC_LINK_BYPASS_MACRO_STATUS_ACTIVE)
#define V_ANC_LINK_BYPASS_MACRO_STATUS_ACTIVE(V)                                  _MM_MAKEVALUE((V),S_ANC_LINK_BYPASS_MACRO_STATUS_ACTIVE)
#define G_ANC_LINK_BYPASS_MACRO_STATUS_ACTIVE(V)                                  _MM_GETVALUE((V),S_ANC_LINK_BYPASS_MACRO_STATUS_ACTIVE,M_ANC_LINK_BYPASS_MACRO_STATUS_ACTIVE)
     
#define S_ANC_LINK_BYPASS_MACRO_STATUS_RSVD0                                      25  //Access: read-as-zero
#define M_ANC_LINK_BYPASS_MACRO_STATUS_RSVD0                                      _MM_MAKEMASK(7,S_ANC_LINK_BYPASS_MACRO_STATUS_RSVD0)
#define V_ANC_LINK_BYPASS_MACRO_STATUS_RSVD0(V)                                   _MM_MAKEVALUE((V),S_ANC_LINK_BYPASS_MACRO_STATUS_RSVD0)
#define G_ANC_LINK_BYPASS_MACRO_STATUS_RSVD0(V)                                   _MM_GETVALUE((V),S_ANC_LINK_BYPASS_MACRO_STATUS_RSVD0,M_ANC_LINK_BYPASS_MACRO_STATUS_RSVD0)

      
#define S_ANC_LINK_NAND_STATUS_VALUE                                              0  //Access: read-only
#define M_ANC_LINK_NAND_STATUS_VALUE                                              _MM_MAKEMASK(8,S_ANC_LINK_NAND_STATUS_VALUE)
#define V_ANC_LINK_NAND_STATUS_VALUE(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_NAND_STATUS_VALUE)
#define G_ANC_LINK_NAND_STATUS_VALUE(V)                                           _MM_GETVALUE((V),S_ANC_LINK_NAND_STATUS_VALUE,M_ANC_LINK_NAND_STATUS_VALUE)
     
#define S_ANC_LINK_NAND_STATUS_RSVD0                                              8  //Access: read-as-zero
#define M_ANC_LINK_NAND_STATUS_RSVD0                                              _MM_MAKEMASK(24,S_ANC_LINK_NAND_STATUS_RSVD0)
#define V_ANC_LINK_NAND_STATUS_RSVD0(V)                                           _MM_MAKEVALUE((V),S_ANC_LINK_NAND_STATUS_RSVD0)
#define G_ANC_LINK_NAND_STATUS_RSVD0(V)                                           _MM_GETVALUE((V),S_ANC_LINK_NAND_STATUS_RSVD0,M_ANC_LINK_NAND_STATUS_RSVD0)

      
#define S_ANC_LINK_DLFIFO_LINK_COUNT_WRITE_BYTES                                  0  //Access: read-only
#define M_ANC_LINK_DLFIFO_LINK_COUNT_WRITE_BYTES                                  _MM_MAKEMASK(32,S_ANC_LINK_DLFIFO_LINK_COUNT_WRITE_BYTES)
#define V_ANC_LINK_DLFIFO_LINK_COUNT_WRITE_BYTES(V)                               _MM_MAKEVALUE((V),S_ANC_LINK_DLFIFO_LINK_COUNT_WRITE_BYTES)
#define G_ANC_LINK_DLFIFO_LINK_COUNT_WRITE_BYTES(V)                               _MM_GETVALUE((V),S_ANC_LINK_DLFIFO_LINK_COUNT_WRITE_BYTES,M_ANC_LINK_DLFIFO_LINK_COUNT_WRITE_BYTES)

      
#define S_ANC_LINK_DLFIFO_COUNT_READ_BYTES                                        0  //Access: read-only
#define M_ANC_LINK_DLFIFO_COUNT_READ_BYTES                                        _MM_MAKEMASK(32,S_ANC_LINK_DLFIFO_COUNT_READ_BYTES)
#define V_ANC_LINK_DLFIFO_COUNT_READ_BYTES(V)                                     _MM_MAKEVALUE((V),S_ANC_LINK_DLFIFO_COUNT_READ_BYTES)
#define G_ANC_LINK_DLFIFO_COUNT_READ_BYTES(V)                                     _MM_GETVALUE((V),S_ANC_LINK_DLFIFO_COUNT_READ_BYTES,M_ANC_LINK_DLFIFO_COUNT_READ_BYTES)

      
#define S_ANC_LINK_PIO_LINK_COUNT_WRITE_BYTES                                     0  //Access: read-only
#define M_ANC_LINK_PIO_LINK_COUNT_WRITE_BYTES                                     _MM_MAKEMASK(32,S_ANC_LINK_PIO_LINK_COUNT_WRITE_BYTES)
#define V_ANC_LINK_PIO_LINK_COUNT_WRITE_BYTES(V)                                  _MM_MAKEVALUE((V),S_ANC_LINK_PIO_LINK_COUNT_WRITE_BYTES)
#define G_ANC_LINK_PIO_LINK_COUNT_WRITE_BYTES(V)                                  _MM_GETVALUE((V),S_ANC_LINK_PIO_LINK_COUNT_WRITE_BYTES,M_ANC_LINK_PIO_LINK_COUNT_WRITE_BYTES)

      
#define S_ANC_LINK_PIO_COUNT_READ_BYTES                                           0  //Access: read-only
#define M_ANC_LINK_PIO_COUNT_READ_BYTES                                           _MM_MAKEMASK(32,S_ANC_LINK_PIO_COUNT_READ_BYTES)
#define V_ANC_LINK_PIO_COUNT_READ_BYTES(V)                                        _MM_MAKEVALUE((V),S_ANC_LINK_PIO_COUNT_READ_BYTES)
#define G_ANC_LINK_PIO_COUNT_READ_BYTES(V)                                        _MM_GETVALUE((V),S_ANC_LINK_PIO_COUNT_READ_BYTES,M_ANC_LINK_PIO_COUNT_READ_BYTES)

      
#define S_ANC_LINK_PHY_COUNT_WRITE_BYTES                                          0  //Access: read-only
#define M_ANC_LINK_PHY_COUNT_WRITE_BYTES                                          _MM_MAKEMASK(32,S_ANC_LINK_PHY_COUNT_WRITE_BYTES)
#define V_ANC_LINK_PHY_COUNT_WRITE_BYTES(V)                                       _MM_MAKEVALUE((V),S_ANC_LINK_PHY_COUNT_WRITE_BYTES)
#define G_ANC_LINK_PHY_COUNT_WRITE_BYTES(V)                                       _MM_GETVALUE((V),S_ANC_LINK_PHY_COUNT_WRITE_BYTES,M_ANC_LINK_PHY_COUNT_WRITE_BYTES)

      
#define S_ANC_LINK_PHY_LINK_COUNT_READ_BYTES                                      0  //Access: read-only
#define M_ANC_LINK_PHY_LINK_COUNT_READ_BYTES                                      _MM_MAKEMASK(32,S_ANC_LINK_PHY_LINK_COUNT_READ_BYTES)
#define V_ANC_LINK_PHY_LINK_COUNT_READ_BYTES(V)                                   _MM_MAKEVALUE((V),S_ANC_LINK_PHY_LINK_COUNT_READ_BYTES)
#define G_ANC_LINK_PHY_LINK_COUNT_READ_BYTES(V)                                   _MM_GETVALUE((V),S_ANC_LINK_PHY_LINK_COUNT_READ_BYTES,M_ANC_LINK_PHY_LINK_COUNT_READ_BYTES)

      
#define S_ANC_LINK_FSM_COUNTS_DATA_COUNT                                          0  //Access: read-only
#define M_ANC_LINK_FSM_COUNTS_DATA_COUNT                                          _MM_MAKEMASK(16,S_ANC_LINK_FSM_COUNTS_DATA_COUNT)
#define V_ANC_LINK_FSM_COUNTS_DATA_COUNT(V)                                       _MM_MAKEVALUE((V),S_ANC_LINK_FSM_COUNTS_DATA_COUNT)
#define G_ANC_LINK_FSM_COUNTS_DATA_COUNT(V)                                       _MM_GETVALUE((V),S_ANC_LINK_FSM_COUNTS_DATA_COUNT,M_ANC_LINK_FSM_COUNTS_DATA_COUNT)
     
#define S_ANC_LINK_FSM_COUNTS_DATA_LENGTH                                         16  //Access: read-only
#define M_ANC_LINK_FSM_COUNTS_DATA_LENGTH                                         _MM_MAKEMASK(16,S_ANC_LINK_FSM_COUNTS_DATA_LENGTH)
#define V_ANC_LINK_FSM_COUNTS_DATA_LENGTH(V)                                      _MM_MAKEVALUE((V),S_ANC_LINK_FSM_COUNTS_DATA_LENGTH)
#define G_ANC_LINK_FSM_COUNTS_DATA_LENGTH(V)                                      _MM_GETVALUE((V),S_ANC_LINK_FSM_COUNTS_DATA_LENGTH,M_ANC_LINK_FSM_COUNTS_DATA_LENGTH)

      
#define S_ANC_LINK_ECC_WATERMARK_WATERMARK                                        0  //Access: read-write
#define M_ANC_LINK_ECC_WATERMARK_WATERMARK                                        _MM_MAKEMASK(8,S_ANC_LINK_ECC_WATERMARK_WATERMARK)
#define V_ANC_LINK_ECC_WATERMARK_WATERMARK(V)                                     _MM_MAKEVALUE((V),S_ANC_LINK_ECC_WATERMARK_WATERMARK)
#define G_ANC_LINK_ECC_WATERMARK_WATERMARK(V)                                     _MM_GETVALUE((V),S_ANC_LINK_ECC_WATERMARK_WATERMARK,M_ANC_LINK_ECC_WATERMARK_WATERMARK)
     
#define S_ANC_LINK_ECC_WATERMARK_WATERMARK_ENABLE                                 8  //Access: read-write
#define M_ANC_LINK_ECC_WATERMARK_WATERMARK_ENABLE                                 _MM_MAKEMASK(1,S_ANC_LINK_ECC_WATERMARK_WATERMARK_ENABLE)
#define V_ANC_LINK_ECC_WATERMARK_WATERMARK_ENABLE(V)                              _MM_MAKEVALUE((V),S_ANC_LINK_ECC_WATERMARK_WATERMARK_ENABLE)
#define G_ANC_LINK_ECC_WATERMARK_WATERMARK_ENABLE(V)                              _MM_GETVALUE((V),S_ANC_LINK_ECC_WATERMARK_WATERMARK_ENABLE,M_ANC_LINK_ECC_WATERMARK_WATERMARK_ENABLE)
     
#define S_ANC_LINK_ECC_WATERMARK_RSVD0                                            9  //Access: read-as-zero
#define M_ANC_LINK_ECC_WATERMARK_RSVD0                                            _MM_MAKEMASK(23,S_ANC_LINK_ECC_WATERMARK_RSVD0)
#define V_ANC_LINK_ECC_WATERMARK_RSVD0(V)                                         _MM_MAKEVALUE((V),S_ANC_LINK_ECC_WATERMARK_RSVD0)
#define G_ANC_LINK_ECC_WATERMARK_RSVD0(V)                                         _MM_GETVALUE((V),S_ANC_LINK_ECC_WATERMARK_RSVD0,M_ANC_LINK_ECC_WATERMARK_RSVD0)

      
#define S_ANC_LINK_ECC_CONFIG_MAX_BIT_FLIPS                                       0  //Access: read-write
#define M_ANC_LINK_ECC_CONFIG_MAX_BIT_FLIPS                                       _MM_MAKEMASK(8,S_ANC_LINK_ECC_CONFIG_MAX_BIT_FLIPS)
#define V_ANC_LINK_ECC_CONFIG_MAX_BIT_FLIPS(V)                                    _MM_MAKEVALUE((V),S_ANC_LINK_ECC_CONFIG_MAX_BIT_FLIPS)
#define G_ANC_LINK_ECC_CONFIG_MAX_BIT_FLIPS(V)                                    _MM_GETVALUE((V),S_ANC_LINK_ECC_CONFIG_MAX_BIT_FLIPS,M_ANC_LINK_ECC_CONFIG_MAX_BIT_FLIPS)
     
#define S_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_00                               8  //Access: read-write
#define M_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_00                               _MM_MAKEMASK(8,S_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_00)
#define V_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_00(V)                            _MM_MAKEVALUE((V),S_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_00)
#define G_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_00(V)                            _MM_GETVALUE((V),S_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_00,M_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_00)
     
#define S_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_FF                               16  //Access: read-write
#define M_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_FF                               _MM_MAKEMASK(8,S_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_FF)
#define V_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_FF(V)                            _MM_MAKEVALUE((V),S_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_FF)
#define G_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_FF(V)                            _MM_GETVALUE((V),S_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_FF,M_ANC_LINK_ECC_CONFIG_ALLOWED_STUCK_BITS_FF)
     
#define S_ANC_LINK_ECC_CONFIG_RSVD0                                               24  //Access: read-as-zero
#define M_ANC_LINK_ECC_CONFIG_RSVD0                                               _MM_MAKEMASK(8,S_ANC_LINK_ECC_CONFIG_RSVD0)
#define V_ANC_LINK_ECC_CONFIG_RSVD0(V)                                            _MM_MAKEVALUE((V),S_ANC_LINK_ECC_CONFIG_RSVD0)
#define G_ANC_LINK_ECC_CONFIG_RSVD0(V)                                            _MM_GETVALUE((V),S_ANC_LINK_ECC_CONFIG_RSVD0,M_ANC_LINK_ECC_CONFIG_RSVD0)

          
#define S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_4_CHIN_CLK                             0  //Access: write-only
#define M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_4_CHIN_CLK                             _MM_MAKEMASK(7,S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_4_CHIN_CLK)
#define V_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_4_CHIN_CLK(V)                          _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_4_CHIN_CLK)
#define G_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_4_CHIN_CLK(V)                          _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_4_CHIN_CLK,M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_4_CHIN_CLK)
     
#define S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD0                                      7  //Access: read-as-zero
#define M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD0                                      _MM_MAKEMASK(1,S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD0)
#define V_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD0(V)                                   _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD0)
#define G_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD0(V)                                   _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD0,M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD0)
     
#define S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_3_CHIN_CLK                             8  //Access: write-only
#define M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_3_CHIN_CLK                             _MM_MAKEMASK(7,S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_3_CHIN_CLK)
#define V_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_3_CHIN_CLK(V)                          _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_3_CHIN_CLK)
#define G_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_3_CHIN_CLK(V)                          _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_3_CHIN_CLK,M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_3_CHIN_CLK)
     
#define S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD1                                      15  //Access: read-as-zero
#define M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD1                                      _MM_MAKEMASK(1,S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD1)
#define V_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD1(V)                                   _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD1)
#define G_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD1(V)                                   _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD1,M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD1)
     
#define S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_2_CHIN_CLK                             16  //Access: write-only
#define M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_2_CHIN_CLK                             _MM_MAKEMASK(7,S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_2_CHIN_CLK)
#define V_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_2_CHIN_CLK(V)                          _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_2_CHIN_CLK)
#define G_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_2_CHIN_CLK(V)                          _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_2_CHIN_CLK,M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DIV_2_CHIN_CLK)
     
#define S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD2                                      23  //Access: read-as-zero
#define M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD2                                      _MM_MAKEMASK(1,S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD2)
#define V_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD2(V)                                   _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD2)
#define G_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD2(V)                                   _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD2,M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD2)
     
#define S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DONT_START_CHIEN_CONFIG                    24  //Access: write-only
#define M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DONT_START_CHIEN_CONFIG                    _MM_MAKEMASK(7,S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DONT_START_CHIEN_CONFIG)
#define V_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DONT_START_CHIEN_CONFIG(V)                 _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DONT_START_CHIEN_CONFIG)
#define G_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DONT_START_CHIEN_CONFIG(V)                 _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DONT_START_CHIEN_CONFIG,M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_DONT_START_CHIEN_CONFIG)
     
#define S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD3                                      31  //Access: read-as-zero
#define M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD3                                      _MM_MAKEMASK(1,S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD3)
#define V_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD3(V)                                   _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD3)
#define G_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD3(V)                                   _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD3,M_ANC_ECC_P_CHIEN_CLOCK_CONFIG_RSVD3)

      
#define S_ANC_ECC_P_CONFIG_RSVD0                                                  0  //Access: read-as-zero
#define M_ANC_ECC_P_CONFIG_RSVD0                                                  _MM_MAKEMASK(1,S_ANC_ECC_P_CONFIG_RSVD0)
#define V_ANC_ECC_P_CONFIG_RSVD0(V)                                               _MM_MAKEVALUE((V),S_ANC_ECC_P_CONFIG_RSVD0)
#define G_ANC_ECC_P_CONFIG_RSVD0(V)                                               _MM_GETVALUE((V),S_ANC_ECC_P_CONFIG_RSVD0,M_ANC_ECC_P_CONFIG_RSVD0)
     
#define S_ANC_ECC_P_CONFIG_STOP_IF_ALL_ERROR_FOUND                                1  //Access: write-only
#define M_ANC_ECC_P_CONFIG_STOP_IF_ALL_ERROR_FOUND                                _MM_MAKEMASK(1,S_ANC_ECC_P_CONFIG_STOP_IF_ALL_ERROR_FOUND)
#define V_ANC_ECC_P_CONFIG_STOP_IF_ALL_ERROR_FOUND(V)                             _MM_MAKEVALUE((V),S_ANC_ECC_P_CONFIG_STOP_IF_ALL_ERROR_FOUND)
#define G_ANC_ECC_P_CONFIG_STOP_IF_ALL_ERROR_FOUND(V)                             _MM_GETVALUE((V),S_ANC_ECC_P_CONFIG_STOP_IF_ALL_ERROR_FOUND,M_ANC_ECC_P_CONFIG_STOP_IF_ALL_ERROR_FOUND)
     
#define S_ANC_ECC_P_CONFIG_EARLY_TERMINATION_MASK                                 2  //Access: write-only
#define M_ANC_ECC_P_CONFIG_EARLY_TERMINATION_MASK                                 _MM_MAKEMASK(1,S_ANC_ECC_P_CONFIG_EARLY_TERMINATION_MASK)
#define V_ANC_ECC_P_CONFIG_EARLY_TERMINATION_MASK(V)                              _MM_MAKEVALUE((V),S_ANC_ECC_P_CONFIG_EARLY_TERMINATION_MASK)
#define G_ANC_ECC_P_CONFIG_EARLY_TERMINATION_MASK(V)                              _MM_GETVALUE((V),S_ANC_ECC_P_CONFIG_EARLY_TERMINATION_MASK,M_ANC_ECC_P_CONFIG_EARLY_TERMINATION_MASK)
     
#define S_ANC_ECC_P_CONFIG_RSVD1                                                  3  //Access: read-as-zero
#define M_ANC_ECC_P_CONFIG_RSVD1                                                  _MM_MAKEMASK(5,S_ANC_ECC_P_CONFIG_RSVD1)
#define V_ANC_ECC_P_CONFIG_RSVD1(V)                                               _MM_MAKEVALUE((V),S_ANC_ECC_P_CONFIG_RSVD1)
#define G_ANC_ECC_P_CONFIG_RSVD1(V)                                               _MM_GETVALUE((V),S_ANC_ECC_P_CONFIG_RSVD1,M_ANC_ECC_P_CONFIG_RSVD1)
     
#define S_ANC_ECC_P_CONFIG_CHIEN_BMA_ABORT                                        8  //Access: write-only
#define M_ANC_ECC_P_CONFIG_CHIEN_BMA_ABORT                                        _MM_MAKEMASK(1,S_ANC_ECC_P_CONFIG_CHIEN_BMA_ABORT)
#define V_ANC_ECC_P_CONFIG_CHIEN_BMA_ABORT(V)                                     _MM_MAKEVALUE((V),S_ANC_ECC_P_CONFIG_CHIEN_BMA_ABORT)
#define G_ANC_ECC_P_CONFIG_CHIEN_BMA_ABORT(V)                                     _MM_GETVALUE((V),S_ANC_ECC_P_CONFIG_CHIEN_BMA_ABORT,M_ANC_ECC_P_CONFIG_CHIEN_BMA_ABORT)
     
#define S_ANC_ECC_P_CONFIG_RSVD2                                                  9  //Access: read-as-zero
#define M_ANC_ECC_P_CONFIG_RSVD2                                                  _MM_MAKEMASK(7,S_ANC_ECC_P_CONFIG_RSVD2)
#define V_ANC_ECC_P_CONFIG_RSVD2(V)                                               _MM_MAKEVALUE((V),S_ANC_ECC_P_CONFIG_RSVD2)
#define G_ANC_ECC_P_CONFIG_RSVD2(V)                                               _MM_GETVALUE((V),S_ANC_ECC_P_CONFIG_RSVD2,M_ANC_ECC_P_CONFIG_RSVD2)
     
#define S_ANC_ECC_P_CONFIG_SYND_ABORT                                             16  //Access: write-only
#define M_ANC_ECC_P_CONFIG_SYND_ABORT                                             _MM_MAKEMASK(1,S_ANC_ECC_P_CONFIG_SYND_ABORT)
#define V_ANC_ECC_P_CONFIG_SYND_ABORT(V)                                          _MM_MAKEVALUE((V),S_ANC_ECC_P_CONFIG_SYND_ABORT)
#define G_ANC_ECC_P_CONFIG_SYND_ABORT(V)                                          _MM_GETVALUE((V),S_ANC_ECC_P_CONFIG_SYND_ABORT,M_ANC_ECC_P_CONFIG_SYND_ABORT)
     
#define S_ANC_ECC_P_CONFIG_RSVD3                                                  17  //Access: read-as-zero
#define M_ANC_ECC_P_CONFIG_RSVD3                                                  _MM_MAKEMASK(7,S_ANC_ECC_P_CONFIG_RSVD3)
#define V_ANC_ECC_P_CONFIG_RSVD3(V)                                               _MM_MAKEVALUE((V),S_ANC_ECC_P_CONFIG_RSVD3)
#define G_ANC_ECC_P_CONFIG_RSVD3(V)                                               _MM_GETVALUE((V),S_ANC_ECC_P_CONFIG_RSVD3,M_ANC_ECC_P_CONFIG_RSVD3)
     
#define S_ANC_ECC_P_CONFIG_MASK_INT_EN                                            24  //Access: write-only
#define M_ANC_ECC_P_CONFIG_MASK_INT_EN                                            _MM_MAKEMASK(1,S_ANC_ECC_P_CONFIG_MASK_INT_EN)
#define V_ANC_ECC_P_CONFIG_MASK_INT_EN(V)                                         _MM_MAKEVALUE((V),S_ANC_ECC_P_CONFIG_MASK_INT_EN)
#define G_ANC_ECC_P_CONFIG_MASK_INT_EN(V)                                         _MM_GETVALUE((V),S_ANC_ECC_P_CONFIG_MASK_INT_EN,M_ANC_ECC_P_CONFIG_MASK_INT_EN)
     
#define S_ANC_ECC_P_CONFIG_RSVD4                                                  25  //Access: read-as-zero
#define M_ANC_ECC_P_CONFIG_RSVD4                                                  _MM_MAKEMASK(7,S_ANC_ECC_P_CONFIG_RSVD4)
#define V_ANC_ECC_P_CONFIG_RSVD4(V)                                               _MM_MAKEVALUE((V),S_ANC_ECC_P_CONFIG_RSVD4)
#define G_ANC_ECC_P_CONFIG_RSVD4(V)                                               _MM_GETVALUE((V),S_ANC_ECC_P_CONFIG_RSVD4,M_ANC_ECC_P_CONFIG_RSVD4)

      
#define S_ANC_ECC_P_OFFSET1_START_ADD_OFFSET                                      0  //Access: write-only
#define M_ANC_ECC_P_OFFSET1_START_ADD_OFFSET                                      _MM_MAKEMASK(13,S_ANC_ECC_P_OFFSET1_START_ADD_OFFSET)
#define V_ANC_ECC_P_OFFSET1_START_ADD_OFFSET(V)                                   _MM_MAKEVALUE((V),S_ANC_ECC_P_OFFSET1_START_ADD_OFFSET)
#define G_ANC_ECC_P_OFFSET1_START_ADD_OFFSET(V)                                   _MM_GETVALUE((V),S_ANC_ECC_P_OFFSET1_START_ADD_OFFSET,M_ANC_ECC_P_OFFSET1_START_ADD_OFFSET)
     
#define S_ANC_ECC_P_OFFSET1_RSVD0                                                 13  //Access: read-as-zero
#define M_ANC_ECC_P_OFFSET1_RSVD0                                                 _MM_MAKEMASK(19,S_ANC_ECC_P_OFFSET1_RSVD0)
#define V_ANC_ECC_P_OFFSET1_RSVD0(V)                                              _MM_MAKEVALUE((V),S_ANC_ECC_P_OFFSET1_RSVD0)
#define G_ANC_ECC_P_OFFSET1_RSVD0(V)                                              _MM_GETVALUE((V),S_ANC_ECC_P_OFFSET1_RSVD0,M_ANC_ECC_P_OFFSET1_RSVD0)

      
#define S_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_ADD_TH                             0  //Access: write-only
#define M_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_ADD_TH                             _MM_MAKEMASK(13,S_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_ADD_TH)
#define V_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_ADD_TH(V)                          _MM_MAKEVALUE((V),S_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_ADD_TH)
#define G_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_ADD_TH(V)                          _MM_GETVALUE((V),S_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_ADD_TH,M_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_ADD_TH)
     
#define S_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD0                                      13  //Access: read-as-zero
#define M_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD0                                      _MM_MAKEMASK(3,S_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD0)
#define V_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD0(V)                                   _MM_MAKEVALUE((V),S_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD0)
#define G_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD0(V)                                   _MM_GETVALUE((V),S_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD0,M_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD0)
     
#define S_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_CPL1_EN                            16  //Access: write-only
#define M_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_CPL1_EN                            _MM_MAKEMASK(3,S_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_CPL1_EN)
#define V_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_CPL1_EN(V)                         _MM_MAKEVALUE((V),S_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_CPL1_EN)
#define G_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_CPL1_EN(V)                         _MM_GETVALUE((V),S_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_CPL1_EN,M_ANC_ECC_P_BCH_DIRECTION_CNFG_BCH_DIR_CPL1_EN)
     
#define S_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD1                                      19  //Access: read-as-zero
#define M_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD1                                      _MM_MAKEMASK(13,S_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD1)
#define V_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD1(V)                                   _MM_MAKEVALUE((V),S_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD1)
#define G_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD1(V)                                   _MM_GETVALUE((V),S_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD1,M_ANC_ECC_P_BCH_DIRECTION_CNFG_RSVD1)

      
#define S_ANC_ECC_P_PAGE_IDX_P_BCH_DEC_PAGE                                       0  //Access: write-only
#define M_ANC_ECC_P_PAGE_IDX_P_BCH_DEC_PAGE                                       _MM_MAKEMASK(5,S_ANC_ECC_P_PAGE_IDX_P_BCH_DEC_PAGE)
#define V_ANC_ECC_P_PAGE_IDX_P_BCH_DEC_PAGE(V)                                    _MM_MAKEVALUE((V),S_ANC_ECC_P_PAGE_IDX_P_BCH_DEC_PAGE)
#define G_ANC_ECC_P_PAGE_IDX_P_BCH_DEC_PAGE(V)                                    _MM_GETVALUE((V),S_ANC_ECC_P_PAGE_IDX_P_BCH_DEC_PAGE,M_ANC_ECC_P_PAGE_IDX_P_BCH_DEC_PAGE)
     
#define S_ANC_ECC_P_PAGE_IDX_RSVD0                                                5  //Access: read-as-zero
#define M_ANC_ECC_P_PAGE_IDX_RSVD0                                                _MM_MAKEMASK(11,S_ANC_ECC_P_PAGE_IDX_RSVD0)
#define V_ANC_ECC_P_PAGE_IDX_RSVD0(V)                                             _MM_MAKEVALUE((V),S_ANC_ECC_P_PAGE_IDX_RSVD0)
#define G_ANC_ECC_P_PAGE_IDX_RSVD0(V)                                             _MM_GETVALUE((V),S_ANC_ECC_P_PAGE_IDX_RSVD0,M_ANC_ECC_P_PAGE_IDX_RSVD0)
     
#define S_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE1                                   16  //Access: write-only
#define M_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE1                                   _MM_MAKEMASK(5,S_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE1)
#define V_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE1(V)                                _MM_MAKEVALUE((V),S_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE1)
#define G_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE1(V)                                _MM_GETVALUE((V),S_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE1,M_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE1)
     
#define S_ANC_ECC_P_PAGE_IDX_RSVD1                                                21  //Access: read-as-zero
#define M_ANC_ECC_P_PAGE_IDX_RSVD1                                                _MM_MAKEMASK(3,S_ANC_ECC_P_PAGE_IDX_RSVD1)
#define V_ANC_ECC_P_PAGE_IDX_RSVD1(V)                                             _MM_MAKEVALUE((V),S_ANC_ECC_P_PAGE_IDX_RSVD1)
#define G_ANC_ECC_P_PAGE_IDX_RSVD1(V)                                             _MM_GETVALUE((V),S_ANC_ECC_P_PAGE_IDX_RSVD1,M_ANC_ECC_P_PAGE_IDX_RSVD1)
     
#define S_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE2                                   24  //Access: write-only
#define M_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE2                                   _MM_MAKEMASK(5,S_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE2)
#define V_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE2(V)                                _MM_MAKEVALUE((V),S_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE2)
#define G_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE2(V)                                _MM_GETVALUE((V),S_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE2,M_ANC_ECC_P_PAGE_IDX_P_BCH_DIR_RD_PAGE2)
     
#define S_ANC_ECC_P_PAGE_IDX_RSVD2                                                29  //Access: read-as-zero
#define M_ANC_ECC_P_PAGE_IDX_RSVD2                                                _MM_MAKEMASK(3,S_ANC_ECC_P_PAGE_IDX_RSVD2)
#define V_ANC_ECC_P_PAGE_IDX_RSVD2(V)                                             _MM_MAKEVALUE((V),S_ANC_ECC_P_PAGE_IDX_RSVD2)
#define G_ANC_ECC_P_PAGE_IDX_RSVD2(V)                                             _MM_GETVALUE((V),S_ANC_ECC_P_PAGE_IDX_RSVD2,M_ANC_ECC_P_PAGE_IDX_RSVD2)

      
#define S_ANC_ECC_P_PAGE_STRUCTURE_T_CODE                                         0  //Access: write-only
#define M_ANC_ECC_P_PAGE_STRUCTURE_T_CODE                                         _MM_MAKEMASK(8,S_ANC_ECC_P_PAGE_STRUCTURE_T_CODE)
#define V_ANC_ECC_P_PAGE_STRUCTURE_T_CODE(V)                                      _MM_MAKEVALUE((V),S_ANC_ECC_P_PAGE_STRUCTURE_T_CODE)
#define G_ANC_ECC_P_PAGE_STRUCTURE_T_CODE(V)                                      _MM_GETVALUE((V),S_ANC_ECC_P_PAGE_STRUCTURE_T_CODE,M_ANC_ECC_P_PAGE_STRUCTURE_T_CODE)
     
#define S_ANC_ECC_P_PAGE_STRUCTURE_RSVD0                                          8  //Access: read-as-zero
#define M_ANC_ECC_P_PAGE_STRUCTURE_RSVD0                                          _MM_MAKEMASK(8,S_ANC_ECC_P_PAGE_STRUCTURE_RSVD0)
#define V_ANC_ECC_P_PAGE_STRUCTURE_RSVD0(V)                                       _MM_MAKEVALUE((V),S_ANC_ECC_P_PAGE_STRUCTURE_RSVD0)
#define G_ANC_ECC_P_PAGE_STRUCTURE_RSVD0(V)                                       _MM_GETVALUE((V),S_ANC_ECC_P_PAGE_STRUCTURE_RSVD0,M_ANC_ECC_P_PAGE_STRUCTURE_RSVD0)
     
#define S_ANC_ECC_P_PAGE_STRUCTURE_N_CODE                                         16  //Access: write-only
#define M_ANC_ECC_P_PAGE_STRUCTURE_N_CODE                                         _MM_MAKEMASK(13,S_ANC_ECC_P_PAGE_STRUCTURE_N_CODE)
#define V_ANC_ECC_P_PAGE_STRUCTURE_N_CODE(V)                                      _MM_MAKEVALUE((V),S_ANC_ECC_P_PAGE_STRUCTURE_N_CODE)
#define G_ANC_ECC_P_PAGE_STRUCTURE_N_CODE(V)                                      _MM_GETVALUE((V),S_ANC_ECC_P_PAGE_STRUCTURE_N_CODE,M_ANC_ECC_P_PAGE_STRUCTURE_N_CODE)
     
#define S_ANC_ECC_P_PAGE_STRUCTURE_RSVD1                                          29  //Access: read-as-zero
#define M_ANC_ECC_P_PAGE_STRUCTURE_RSVD1                                          _MM_MAKEMASK(3,S_ANC_ECC_P_PAGE_STRUCTURE_RSVD1)
#define V_ANC_ECC_P_PAGE_STRUCTURE_RSVD1(V)                                       _MM_MAKEVALUE((V),S_ANC_ECC_P_PAGE_STRUCTURE_RSVD1)
#define G_ANC_ECC_P_PAGE_STRUCTURE_RSVD1(V)                                       _MM_GETVALUE((V),S_ANC_ECC_P_PAGE_STRUCTURE_RSVD1,M_ANC_ECC_P_PAGE_STRUCTURE_RSVD1)

      
#define S_ANC_ECC_P_ECC_CLK_CNFG_P_SYND_FORCE_CLK_EN                              0  //Access: write-only
#define M_ANC_ECC_P_ECC_CLK_CNFG_P_SYND_FORCE_CLK_EN                              _MM_MAKEMASK(1,S_ANC_ECC_P_ECC_CLK_CNFG_P_SYND_FORCE_CLK_EN)
#define V_ANC_ECC_P_ECC_CLK_CNFG_P_SYND_FORCE_CLK_EN(V)                           _MM_MAKEVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_P_SYND_FORCE_CLK_EN)
#define G_ANC_ECC_P_ECC_CLK_CNFG_P_SYND_FORCE_CLK_EN(V)                           _MM_GETVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_P_SYND_FORCE_CLK_EN,M_ANC_ECC_P_ECC_CLK_CNFG_P_SYND_FORCE_CLK_EN)
     
#define S_ANC_ECC_P_ECC_CLK_CNFG_RSVD0                                            1  //Access: read-as-zero
#define M_ANC_ECC_P_ECC_CLK_CNFG_RSVD0                                            _MM_MAKEMASK(7,S_ANC_ECC_P_ECC_CLK_CNFG_RSVD0)
#define V_ANC_ECC_P_ECC_CLK_CNFG_RSVD0(V)                                         _MM_MAKEVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_RSVD0)
#define G_ANC_ECC_P_ECC_CLK_CNFG_RSVD0(V)                                         _MM_GETVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_RSVD0,M_ANC_ECC_P_ECC_CLK_CNFG_RSVD0)
     
#define S_ANC_ECC_P_ECC_CLK_CNFG_P_BMA_FORCE_CLK_EN                               8  //Access: write-only
#define M_ANC_ECC_P_ECC_CLK_CNFG_P_BMA_FORCE_CLK_EN                               _MM_MAKEMASK(1,S_ANC_ECC_P_ECC_CLK_CNFG_P_BMA_FORCE_CLK_EN)
#define V_ANC_ECC_P_ECC_CLK_CNFG_P_BMA_FORCE_CLK_EN(V)                            _MM_MAKEVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_P_BMA_FORCE_CLK_EN)
#define G_ANC_ECC_P_ECC_CLK_CNFG_P_BMA_FORCE_CLK_EN(V)                            _MM_GETVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_P_BMA_FORCE_CLK_EN,M_ANC_ECC_P_ECC_CLK_CNFG_P_BMA_FORCE_CLK_EN)
     
#define S_ANC_ECC_P_ECC_CLK_CNFG_RSVD1                                            9  //Access: read-as-zero
#define M_ANC_ECC_P_ECC_CLK_CNFG_RSVD1                                            _MM_MAKEMASK(7,S_ANC_ECC_P_ECC_CLK_CNFG_RSVD1)
#define V_ANC_ECC_P_ECC_CLK_CNFG_RSVD1(V)                                         _MM_MAKEVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_RSVD1)
#define G_ANC_ECC_P_ECC_CLK_CNFG_RSVD1(V)                                         _MM_GETVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_RSVD1,M_ANC_ECC_P_ECC_CLK_CNFG_RSVD1)
     
#define S_ANC_ECC_P_ECC_CLK_CNFG_P_CHIEN_FORCE_CLK_EN                             16  //Access: write-only
#define M_ANC_ECC_P_ECC_CLK_CNFG_P_CHIEN_FORCE_CLK_EN                             _MM_MAKEMASK(1,S_ANC_ECC_P_ECC_CLK_CNFG_P_CHIEN_FORCE_CLK_EN)
#define V_ANC_ECC_P_ECC_CLK_CNFG_P_CHIEN_FORCE_CLK_EN(V)                          _MM_MAKEVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_P_CHIEN_FORCE_CLK_EN)
#define G_ANC_ECC_P_ECC_CLK_CNFG_P_CHIEN_FORCE_CLK_EN(V)                          _MM_GETVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_P_CHIEN_FORCE_CLK_EN,M_ANC_ECC_P_ECC_CLK_CNFG_P_CHIEN_FORCE_CLK_EN)
     
#define S_ANC_ECC_P_ECC_CLK_CNFG_RSVD2                                            17  //Access: read-as-zero
#define M_ANC_ECC_P_ECC_CLK_CNFG_RSVD2                                            _MM_MAKEMASK(15,S_ANC_ECC_P_ECC_CLK_CNFG_RSVD2)
#define V_ANC_ECC_P_ECC_CLK_CNFG_RSVD2(V)                                         _MM_MAKEVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_RSVD2)
#define G_ANC_ECC_P_ECC_CLK_CNFG_RSVD2(V)                                         _MM_GETVALUE((V),S_ANC_ECC_P_ECC_CLK_CNFG_RSVD2,M_ANC_ECC_P_ECC_CLK_CNFG_RSVD2)

      
#define S_ANC_ECC_P_OFFSET3_P_OFFSET3                                             0  //Access: write-only
#define M_ANC_ECC_P_OFFSET3_P_OFFSET3                                             _MM_MAKEMASK(13,S_ANC_ECC_P_OFFSET3_P_OFFSET3)
#define V_ANC_ECC_P_OFFSET3_P_OFFSET3(V)                                          _MM_MAKEVALUE((V),S_ANC_ECC_P_OFFSET3_P_OFFSET3)
#define G_ANC_ECC_P_OFFSET3_P_OFFSET3(V)                                          _MM_GETVALUE((V),S_ANC_ECC_P_OFFSET3_P_OFFSET3,M_ANC_ECC_P_OFFSET3_P_OFFSET3)
     
#define S_ANC_ECC_P_OFFSET3_RSVD0                                                 13  //Access: read-as-zero
#define M_ANC_ECC_P_OFFSET3_RSVD0                                                 _MM_MAKEMASK(19,S_ANC_ECC_P_OFFSET3_RSVD0)
#define V_ANC_ECC_P_OFFSET3_RSVD0(V)                                              _MM_MAKEVALUE((V),S_ANC_ECC_P_OFFSET3_RSVD0)
#define G_ANC_ECC_P_OFFSET3_RSVD0(V)                                              _MM_GETVALUE((V),S_ANC_ECC_P_OFFSET3_RSVD0,M_ANC_ECC_P_OFFSET3_RSVD0)

      
#define S_ANC_ECC_P_CHIEN_START_OFFSET_CHIEN_START_OFFSET                         0  //Access: write-only
#define M_ANC_ECC_P_CHIEN_START_OFFSET_CHIEN_START_OFFSET                         _MM_MAKEMASK(16,S_ANC_ECC_P_CHIEN_START_OFFSET_CHIEN_START_OFFSET)
#define V_ANC_ECC_P_CHIEN_START_OFFSET_CHIEN_START_OFFSET(V)                      _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_START_OFFSET_CHIEN_START_OFFSET)
#define G_ANC_ECC_P_CHIEN_START_OFFSET_CHIEN_START_OFFSET(V)                      _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_START_OFFSET_CHIEN_START_OFFSET,M_ANC_ECC_P_CHIEN_START_OFFSET_CHIEN_START_OFFSET)
     
#define S_ANC_ECC_P_CHIEN_START_OFFSET_RSVD0                                      16  //Access: read-as-zero
#define M_ANC_ECC_P_CHIEN_START_OFFSET_RSVD0                                      _MM_MAKEMASK(16,S_ANC_ECC_P_CHIEN_START_OFFSET_RSVD0)
#define V_ANC_ECC_P_CHIEN_START_OFFSET_RSVD0(V)                                   _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_START_OFFSET_RSVD0)
#define G_ANC_ECC_P_CHIEN_START_OFFSET_RSVD0(V)                                   _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_START_OFFSET_RSVD0,M_ANC_ECC_P_CHIEN_START_OFFSET_RSVD0)

      
#define S_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_CHIEN_EARLY_INT_TIMING                  0  //Access: write-only
#define M_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_CHIEN_EARLY_INT_TIMING                  _MM_MAKEMASK(13,S_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_CHIEN_EARLY_INT_TIMING)
#define V_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_CHIEN_EARLY_INT_TIMING(V)               _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_CHIEN_EARLY_INT_TIMING)
#define G_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_CHIEN_EARLY_INT_TIMING(V)               _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_CHIEN_EARLY_INT_TIMING,M_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_CHIEN_EARLY_INT_TIMING)
     
#define S_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_RSVD0                                   13  //Access: read-as-zero
#define M_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_RSVD0                                   _MM_MAKEMASK(19,S_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_RSVD0)
#define V_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_RSVD0(V)                                _MM_MAKEVALUE((V),S_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_RSVD0)
#define G_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_RSVD0(V)                                _MM_GETVALUE((V),S_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_RSVD0,M_ANC_ECC_P_CHIEN_EARLY_INTERRUPT_RSVD0)

      
#define S_ANC_ECC_P_OFFSET4_CHIEN_LAST_FIX_BYTE                                   0  //Access: write-only
#define M_ANC_ECC_P_OFFSET4_CHIEN_LAST_FIX_BYTE                                   _MM_MAKEMASK(13,S_ANC_ECC_P_OFFSET4_CHIEN_LAST_FIX_BYTE)
#define V_ANC_ECC_P_OFFSET4_CHIEN_LAST_FIX_BYTE(V)                                _MM_MAKEVALUE((V),S_ANC_ECC_P_OFFSET4_CHIEN_LAST_FIX_BYTE)
#define G_ANC_ECC_P_OFFSET4_CHIEN_LAST_FIX_BYTE(V)                                _MM_GETVALUE((V),S_ANC_ECC_P_OFFSET4_CHIEN_LAST_FIX_BYTE,M_ANC_ECC_P_OFFSET4_CHIEN_LAST_FIX_BYTE)
     
#define S_ANC_ECC_P_OFFSET4_RSVD0                                                 13  //Access: read-as-zero
#define M_ANC_ECC_P_OFFSET4_RSVD0                                                 _MM_MAKEMASK(19,S_ANC_ECC_P_OFFSET4_RSVD0)
#define V_ANC_ECC_P_OFFSET4_RSVD0(V)                                              _MM_MAKEVALUE((V),S_ANC_ECC_P_OFFSET4_RSVD0)
#define G_ANC_ECC_P_OFFSET4_RSVD0(V)                                              _MM_GETVALUE((V),S_ANC_ECC_P_OFFSET4_RSVD0,M_ANC_ECC_P_OFFSET4_RSVD0)

      
#define S_ANC_ECC_P_STATUS_STATUS                                                 0  //Access: read-only
#define M_ANC_ECC_P_STATUS_STATUS                                                 _MM_MAKEMASK(1,S_ANC_ECC_P_STATUS_STATUS)
#define V_ANC_ECC_P_STATUS_STATUS(V)                                              _MM_MAKEVALUE((V),S_ANC_ECC_P_STATUS_STATUS)
#define G_ANC_ECC_P_STATUS_STATUS(V)                                              _MM_GETVALUE((V),S_ANC_ECC_P_STATUS_STATUS,M_ANC_ECC_P_STATUS_STATUS)
     
#define S_ANC_ECC_P_STATUS_RSVD0                                                  1  //Access: read-as-zero
#define M_ANC_ECC_P_STATUS_RSVD0                                                  _MM_MAKEMASK(7,S_ANC_ECC_P_STATUS_RSVD0)
#define V_ANC_ECC_P_STATUS_RSVD0(V)                                               _MM_MAKEVALUE((V),S_ANC_ECC_P_STATUS_RSVD0)
#define G_ANC_ECC_P_STATUS_RSVD0(V)                                               _MM_GETVALUE((V),S_ANC_ECC_P_STATUS_RSVD0,M_ANC_ECC_P_STATUS_RSVD0)
     
#define S_ANC_ECC_P_STATUS_NUMBER_OF_FOUND_ERROR                                  8  //Access: read-only
#define M_ANC_ECC_P_STATUS_NUMBER_OF_FOUND_ERROR                                  _MM_MAKEMASK(7,S_ANC_ECC_P_STATUS_NUMBER_OF_FOUND_ERROR)
#define V_ANC_ECC_P_STATUS_NUMBER_OF_FOUND_ERROR(V)                               _MM_MAKEVALUE((V),S_ANC_ECC_P_STATUS_NUMBER_OF_FOUND_ERROR)
#define G_ANC_ECC_P_STATUS_NUMBER_OF_FOUND_ERROR(V)                               _MM_GETVALUE((V),S_ANC_ECC_P_STATUS_NUMBER_OF_FOUND_ERROR,M_ANC_ECC_P_STATUS_NUMBER_OF_FOUND_ERROR)
     
#define S_ANC_ECC_P_STATUS_RSVD1                                                  15  //Access: read-as-zero
#define M_ANC_ECC_P_STATUS_RSVD1                                                  _MM_MAKEMASK(1,S_ANC_ECC_P_STATUS_RSVD1)
#define V_ANC_ECC_P_STATUS_RSVD1(V)                                               _MM_MAKEVALUE((V),S_ANC_ECC_P_STATUS_RSVD1)
#define G_ANC_ECC_P_STATUS_RSVD1(V)                                               _MM_GETVALUE((V),S_ANC_ECC_P_STATUS_RSVD1,M_ANC_ECC_P_STATUS_RSVD1)
     
#define S_ANC_ECC_P_STATUS_BCH_CE                                                 16  //Access: read-only
#define M_ANC_ECC_P_STATUS_BCH_CE                                                 _MM_MAKEMASK(2,S_ANC_ECC_P_STATUS_BCH_CE)
#define V_ANC_ECC_P_STATUS_BCH_CE(V)                                              _MM_MAKEVALUE((V),S_ANC_ECC_P_STATUS_BCH_CE)
#define G_ANC_ECC_P_STATUS_BCH_CE(V)                                              _MM_GETVALUE((V),S_ANC_ECC_P_STATUS_BCH_CE,M_ANC_ECC_P_STATUS_BCH_CE)
     
#define S_ANC_ECC_P_STATUS_RSVD2                                                  18  //Access: read-as-zero
#define M_ANC_ECC_P_STATUS_RSVD2                                                  _MM_MAKEMASK(14,S_ANC_ECC_P_STATUS_RSVD2)
#define V_ANC_ECC_P_STATUS_RSVD2(V)                                               _MM_MAKEVALUE((V),S_ANC_ECC_P_STATUS_RSVD2)
#define G_ANC_ECC_P_STATUS_RSVD2(V)                                               _MM_GETVALUE((V),S_ANC_ECC_P_STATUS_RSVD2,M_ANC_ECC_P_STATUS_RSVD2)

      
#define S_ANC_ECC_P_BCH_DIR_CNTRS_ZERO_TO_ONE_CNTR                                0  //Access: read-only
#define M_ANC_ECC_P_BCH_DIR_CNTRS_ZERO_TO_ONE_CNTR                                _MM_MAKEMASK(7,S_ANC_ECC_P_BCH_DIR_CNTRS_ZERO_TO_ONE_CNTR)
#define V_ANC_ECC_P_BCH_DIR_CNTRS_ZERO_TO_ONE_CNTR(V)                             _MM_MAKEVALUE((V),S_ANC_ECC_P_BCH_DIR_CNTRS_ZERO_TO_ONE_CNTR)
#define G_ANC_ECC_P_BCH_DIR_CNTRS_ZERO_TO_ONE_CNTR(V)                             _MM_GETVALUE((V),S_ANC_ECC_P_BCH_DIR_CNTRS_ZERO_TO_ONE_CNTR,M_ANC_ECC_P_BCH_DIR_CNTRS_ZERO_TO_ONE_CNTR)
     
#define S_ANC_ECC_P_BCH_DIR_CNTRS_RSVD0                                           7  //Access: read-as-zero
#define M_ANC_ECC_P_BCH_DIR_CNTRS_RSVD0                                           _MM_MAKEMASK(9,S_ANC_ECC_P_BCH_DIR_CNTRS_RSVD0)
#define V_ANC_ECC_P_BCH_DIR_CNTRS_RSVD0(V)                                        _MM_MAKEVALUE((V),S_ANC_ECC_P_BCH_DIR_CNTRS_RSVD0)
#define G_ANC_ECC_P_BCH_DIR_CNTRS_RSVD0(V)                                        _MM_GETVALUE((V),S_ANC_ECC_P_BCH_DIR_CNTRS_RSVD0,M_ANC_ECC_P_BCH_DIR_CNTRS_RSVD0)
     
#define S_ANC_ECC_P_BCH_DIR_CNTRS_ONE_TO_ZERO_CNTR                                16  //Access: read-only
#define M_ANC_ECC_P_BCH_DIR_CNTRS_ONE_TO_ZERO_CNTR                                _MM_MAKEMASK(7,S_ANC_ECC_P_BCH_DIR_CNTRS_ONE_TO_ZERO_CNTR)
#define V_ANC_ECC_P_BCH_DIR_CNTRS_ONE_TO_ZERO_CNTR(V)                             _MM_MAKEVALUE((V),S_ANC_ECC_P_BCH_DIR_CNTRS_ONE_TO_ZERO_CNTR)
#define G_ANC_ECC_P_BCH_DIR_CNTRS_ONE_TO_ZERO_CNTR(V)                             _MM_GETVALUE((V),S_ANC_ECC_P_BCH_DIR_CNTRS_ONE_TO_ZERO_CNTR,M_ANC_ECC_P_BCH_DIR_CNTRS_ONE_TO_ZERO_CNTR)
     
#define S_ANC_ECC_P_BCH_DIR_CNTRS_RSVD1                                           23  //Access: read-as-zero
#define M_ANC_ECC_P_BCH_DIR_CNTRS_RSVD1                                           _MM_MAKEMASK(9,S_ANC_ECC_P_BCH_DIR_CNTRS_RSVD1)
#define V_ANC_ECC_P_BCH_DIR_CNTRS_RSVD1(V)                                        _MM_MAKEVALUE((V),S_ANC_ECC_P_BCH_DIR_CNTRS_RSVD1)
#define G_ANC_ECC_P_BCH_DIR_CNTRS_RSVD1(V)                                        _MM_GETVALUE((V),S_ANC_ECC_P_BCH_DIR_CNTRS_RSVD1,M_ANC_ECC_P_BCH_DIR_CNTRS_RSVD1)

      
#define S_ANC_ECC_P_BMA_STATUS_BMA_DEGREE                                         0  //Access: read-only
#define M_ANC_ECC_P_BMA_STATUS_BMA_DEGREE                                         _MM_MAKEMASK(7,S_ANC_ECC_P_BMA_STATUS_BMA_DEGREE)
#define V_ANC_ECC_P_BMA_STATUS_BMA_DEGREE(V)                                      _MM_MAKEVALUE((V),S_ANC_ECC_P_BMA_STATUS_BMA_DEGREE)
#define G_ANC_ECC_P_BMA_STATUS_BMA_DEGREE(V)                                      _MM_GETVALUE((V),S_ANC_ECC_P_BMA_STATUS_BMA_DEGREE,M_ANC_ECC_P_BMA_STATUS_BMA_DEGREE)
     
#define S_ANC_ECC_P_BMA_STATUS_RSVD0                                              7  //Access: read-as-zero
#define M_ANC_ECC_P_BMA_STATUS_RSVD0                                              _MM_MAKEMASK(1,S_ANC_ECC_P_BMA_STATUS_RSVD0)
#define V_ANC_ECC_P_BMA_STATUS_RSVD0(V)                                           _MM_MAKEVALUE((V),S_ANC_ECC_P_BMA_STATUS_RSVD0)
#define G_ANC_ECC_P_BMA_STATUS_RSVD0(V)                                           _MM_GETVALUE((V),S_ANC_ECC_P_BMA_STATUS_RSVD0,M_ANC_ECC_P_BMA_STATUS_RSVD0)
     
#define S_ANC_ECC_P_BMA_STATUS_BMA_EARLY_TERMINATION_INDICATION                   8  //Access: read-only
#define M_ANC_ECC_P_BMA_STATUS_BMA_EARLY_TERMINATION_INDICATION                   _MM_MAKEMASK(1,S_ANC_ECC_P_BMA_STATUS_BMA_EARLY_TERMINATION_INDICATION)
#define V_ANC_ECC_P_BMA_STATUS_BMA_EARLY_TERMINATION_INDICATION(V)                _MM_MAKEVALUE((V),S_ANC_ECC_P_BMA_STATUS_BMA_EARLY_TERMINATION_INDICATION)
#define G_ANC_ECC_P_BMA_STATUS_BMA_EARLY_TERMINATION_INDICATION(V)                _MM_GETVALUE((V),S_ANC_ECC_P_BMA_STATUS_BMA_EARLY_TERMINATION_INDICATION,M_ANC_ECC_P_BMA_STATUS_BMA_EARLY_TERMINATION_INDICATION)
     
#define S_ANC_ECC_P_BMA_STATUS_RSVD1                                              9  //Access: read-as-zero
#define M_ANC_ECC_P_BMA_STATUS_RSVD1                                              _MM_MAKEMASK(23,S_ANC_ECC_P_BMA_STATUS_RSVD1)
#define V_ANC_ECC_P_BMA_STATUS_RSVD1(V)                                           _MM_MAKEVALUE((V),S_ANC_ECC_P_BMA_STATUS_RSVD1)
#define G_ANC_ECC_P_BMA_STATUS_RSVD1(V)                                           _MM_GETVALUE((V),S_ANC_ECC_P_BMA_STATUS_RSVD1,M_ANC_ECC_P_BMA_STATUS_RSVD1)

          
#define S_ANC_MACRO_TABLE_COMMAND_RSVD0                                           0  //Access: read-as-zero
#define M_ANC_MACRO_TABLE_COMMAND_RSVD0                                           _MM_MAKEMASK(256,S_ANC_MACRO_TABLE_COMMAND_RSVD0)
#define V_ANC_MACRO_TABLE_COMMAND_RSVD0(V)                                        _MM_MAKEVALUE((V),S_ANC_MACRO_TABLE_COMMAND_RSVD0)
#define G_ANC_MACRO_TABLE_COMMAND_RSVD0(V)                                        _MM_GETVALUE((V),S_ANC_MACRO_TABLE_COMMAND_RSVD0,M_ANC_MACRO_TABLE_COMMAND_RSVD0)

          
#define S_ANC_DMA_CMDQ_FIFO_DA_WORD_RSVD0                                         0  //Access: read-as-zero
#define M_ANC_DMA_CMDQ_FIFO_DA_WORD_RSVD0                                         _MM_MAKEMASK(128,S_ANC_DMA_CMDQ_FIFO_DA_WORD_RSVD0)
#define V_ANC_DMA_CMDQ_FIFO_DA_WORD_RSVD0(V)                                      _MM_MAKEVALUE((V),S_ANC_DMA_CMDQ_FIFO_DA_WORD_RSVD0)
#define G_ANC_DMA_CMDQ_FIFO_DA_WORD_RSVD0(V)                                      _MM_GETVALUE((V),S_ANC_DMA_CMDQ_FIFO_DA_WORD_RSVD0,M_ANC_DMA_CMDQ_FIFO_DA_WORD_RSVD0)

          
#define S_ANC_LINK_CMDQ_FIFO_DA_WORD_RSVD0                                        0  //Access: read-as-zero
#define M_ANC_LINK_CMDQ_FIFO_DA_WORD_RSVD0                                        _MM_MAKEMASK(128,S_ANC_LINK_CMDQ_FIFO_DA_WORD_RSVD0)
#define V_ANC_LINK_CMDQ_FIFO_DA_WORD_RSVD0(V)                                     _MM_MAKEVALUE((V),S_ANC_LINK_CMDQ_FIFO_DA_WORD_RSVD0)
#define G_ANC_LINK_CMDQ_FIFO_DA_WORD_RSVD0(V)                                     _MM_GETVALUE((V),S_ANC_LINK_CMDQ_FIFO_DA_WORD_RSVD0,M_ANC_LINK_CMDQ_FIFO_DA_WORD_RSVD0)

          
#define S_ANC_LINK_BYPASS_FIFO_DA_WORD_RSVD0                                      0  //Access: read-as-zero
#define M_ANC_LINK_BYPASS_FIFO_DA_WORD_RSVD0                                      _MM_MAKEMASK(8,S_ANC_LINK_BYPASS_FIFO_DA_WORD_RSVD0)
#define V_ANC_LINK_BYPASS_FIFO_DA_WORD_RSVD0(V)                                   _MM_MAKEVALUE((V),S_ANC_LINK_BYPASS_FIFO_DA_WORD_RSVD0)
#define G_ANC_LINK_BYPASS_FIFO_DA_WORD_RSVD0(V)                                   _MM_GETVALUE((V),S_ANC_LINK_BYPASS_FIFO_DA_WORD_RSVD0,M_ANC_LINK_BYPASS_FIFO_DA_WORD_RSVD0)

          
#define S_ANC_PIO_READ_FIFO_DA_WORD_RSVD0                                         0  //Access: read-as-zero
#define M_ANC_PIO_READ_FIFO_DA_WORD_RSVD0                                         _MM_MAKEMASK(64,S_ANC_PIO_READ_FIFO_DA_WORD_RSVD0)
#define V_ANC_PIO_READ_FIFO_DA_WORD_RSVD0(V)                                      _MM_MAKEVALUE((V),S_ANC_PIO_READ_FIFO_DA_WORD_RSVD0)
#define G_ANC_PIO_READ_FIFO_DA_WORD_RSVD0(V)                                      _MM_GETVALUE((V),S_ANC_PIO_READ_FIFO_DA_WORD_RSVD0,M_ANC_PIO_READ_FIFO_DA_WORD_RSVD0)

          
#define S_ANC_DMA_DEBUG_FIFO_COMMAND_RSVD0                                        0  //Access: read-as-zero
#define M_ANC_DMA_DEBUG_FIFO_COMMAND_RSVD0                                        _MM_MAKEMASK(32,S_ANC_DMA_DEBUG_FIFO_COMMAND_RSVD0)
#define V_ANC_DMA_DEBUG_FIFO_COMMAND_RSVD0(V)                                     _MM_MAKEVALUE((V),S_ANC_DMA_DEBUG_FIFO_COMMAND_RSVD0)
#define G_ANC_DMA_DEBUG_FIFO_COMMAND_RSVD0(V)                                     _MM_GETVALUE((V),S_ANC_DMA_DEBUG_FIFO_COMMAND_RSVD0,M_ANC_DMA_DEBUG_FIFO_COMMAND_RSVD0)

          
#define S_ANC_LINK_DEBUG_FIFO_COMMAND_RSVD0                                       0  //Access: read-as-zero
#define M_ANC_LINK_DEBUG_FIFO_COMMAND_RSVD0                                       _MM_MAKEMASK(32,S_ANC_LINK_DEBUG_FIFO_COMMAND_RSVD0)
#define V_ANC_LINK_DEBUG_FIFO_COMMAND_RSVD0(V)                                    _MM_MAKEVALUE((V),S_ANC_LINK_DEBUG_FIFO_COMMAND_RSVD0)
#define G_ANC_LINK_DEBUG_FIFO_COMMAND_RSVD0(V)                                    _MM_GETVALUE((V),S_ANC_LINK_DEBUG_FIFO_COMMAND_RSVD0,M_ANC_LINK_DEBUG_FIFO_COMMAND_RSVD0)

          
#define S_ANC_LINK_ECC_STATUS_FIFO_DA_WORD_RSVD0                                  0  //Access: read-as-zero
#define M_ANC_LINK_ECC_STATUS_FIFO_DA_WORD_RSVD0                                  _MM_MAKEMASK(64,S_ANC_LINK_ECC_STATUS_FIFO_DA_WORD_RSVD0)
#define V_ANC_LINK_ECC_STATUS_FIFO_DA_WORD_RSVD0(V)                               _MM_MAKEVALUE((V),S_ANC_LINK_ECC_STATUS_FIFO_DA_WORD_RSVD0)
#define G_ANC_LINK_ECC_STATUS_FIFO_DA_WORD_RSVD0(V)                               _MM_GETVALUE((V),S_ANC_LINK_ECC_STATUS_FIFO_DA_WORD_RSVD0,M_ANC_LINK_ECC_STATUS_FIFO_DA_WORD_RSVD0)

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          

/******************************************************************************/
/* Register Fields typedef structs */
/******************************************************************************/
#ifndef __ASSEMBLY__     
    
typedef union anc_chan_version_t {                                    //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t minor_release:8;                                     //Access: read-only     
        uint32_t major_release:8;                                     //Access: read-only     
        uint32_t version:8;                                           //Access: read-only     
        uint32_t rsvd0:8;                                             //Access: read-as-zero  
        }f;
} anc_chan_version_t;
    
typedef union anc_chan_config_t {                                     //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t auto_clk_gating_en:1;                                //Access: read-write     
        uint32_t rsvd0:31;                                            //Access: read-as-zero  
        }f;
} anc_chan_config_t;
    
typedef union anc_chan_int_status_t {                                 //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t dma_cmd_flag:1;                                      //Access: write-once-clear     
        uint32_t link_cmd_flag:1;                                     //Access: write-once-clear     
        uint32_t channel_stopped:1;                                   //Access: write-once-clear     
        uint32_t dma_cmdq_fifo_low:1;                                 //Access: read-only     
        uint32_t link_cmdq_fifo_low:1;                                //Access: read-only     
        uint32_t link_bypass_fifo_low:1;                              //Access: read-only     
        uint32_t link_pio_read_fifo_high:1;                           //Access: read-only     
        uint32_t dma_cmdq_fifo_overflow:1;                            //Access: write-once-clear     
        uint32_t link_cmdq_fifo_overflow:1;                           //Access: write-once-clear     
        uint32_t link_bypass_fifo_overflow:1;                         //Access: write-once-clear     
        uint32_t link_pio_read_fifo_underflow:1;                      //Access: write-once-clear     
        uint32_t crc_err:1;                                           //Access: write-once-clear     
        uint32_t read_status_err_response:1;                          //Access: write-once-clear     
        uint32_t invalid_dma_command:1;                               //Access: write-once-clear     
        uint32_t invalid_link_command:1;                              //Access: write-once-clear     
        uint32_t axi_response_not_okay:1;                             //Access: write-once-clear     
        uint32_t aes_err:1;                                           //Access: write-once-clear     
        uint32_t dma_cmd_timeout:1;                                   //Access: write-once-clear     
        uint32_t link_cmd_timeout:1;                                  //Access: write-once-clear     
        uint32_t link_ecc_status_fifo_high:1;                         //Access: read-only     
        uint32_t link_ecc_status_fifo_underflow:1;                    //Access: write-once-clear     
        uint32_t rsvd0:11;                                            //Access: read-as-zero  
        }f;
} anc_chan_int_status_t;
    
typedef union anc_chan_int_enable_t {                                 //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t dma_cmd_flag:1;                                      //Access: read-write     
        uint32_t link_cmd_flag:1;                                     //Access: read-write     
        uint32_t channel_stopped:1;                                   //Access: read-write     
        uint32_t dma_cmdq_fifo_low:1;                                 //Access: read-write     
        uint32_t link_cmdq_fifo_low:1;                                //Access: read-write     
        uint32_t link_bypass_fifo_low:1;                              //Access: read-write     
        uint32_t link_pio_read_fifo_high:1;                           //Access: read-write     
        uint32_t dma_cmdq_fifo_overflow:1;                            //Access: read-write     
        uint32_t link_cmdq_fifo_overflow:1;                           //Access: read-write     
        uint32_t link_bypass_fifo_overflow:1;                         //Access: read-write     
        uint32_t link_pio_read_fifo_underflow:1;                      //Access: read-write     
        uint32_t crc_err:1;                                           //Access: read-write     
        uint32_t read_status_err_response:1;                          //Access: read-write     
        uint32_t invalid_dma_command:1;                               //Access: read-write     
        uint32_t invalid_link_command:1;                              //Access: read-write     
        uint32_t axi_response_not_okay:1;                             //Access: read-write     
        uint32_t aes_err:1;                                           //Access: read-write     
        uint32_t dma_cmd_timeout:1;                                   //Access: read-write     
        uint32_t link_cmd_timeout:1;                                  //Access: read-write     
        uint32_t link_ecc_status_fifo_high:1;                         //Access: read-write     
        uint32_t link_ecc_status_fifo_underflow:1;                    //Access: read-write     
        uint32_t rsvd0:11;                                            //Access: read-as-zero  
        }f;
} anc_chan_int_enable_t;
    
typedef union anc_chan_dma_watermarks_t {                             //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t dma_cmdq_fifo_low:7;                                 //Access: read-write     
        uint32_t rsvd0:25;                                            //Access: read-as-zero  
        }f;
} anc_chan_dma_watermarks_t;
    
typedef union anc_chan_link_watermarks_t {                            //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t link_cmdq_fifo_low:8;                                //Access: read-write     
        uint32_t rsvd0:8;                                             //Access: read-as-zero     
        uint32_t link_bypass_fifo_low:4;                              //Access: read-write     
        uint32_t rsvd1:4;                                             //Access: read-as-zero     
        uint32_t link_pio_read_fifo_high:7;                           //Access: read-write     
        uint32_t rsvd2:1;                                             //Access: read-as-zero  
        }f;
} anc_chan_link_watermarks_t;
    
typedef union anc_chan_status_watermarks_t {                          //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t link_ecc_status_fifo_high:7;                         //Access: read-write     
        uint32_t rsvd0:25;                                            //Access: read-as-zero  
        }f;
} anc_chan_status_watermarks_t;
    
typedef union anc_chan_dma_cmdq_fifo_status_t {                       //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t low:1;                                               //Access: read-only     
        uint32_t full:1;                                              //Access: read-only     
        uint32_t overflow:1;                                          //Access: read-only     
        uint32_t rsvd0:5;                                             //Access: read-as-zero     
        uint32_t level:7;                                             //Access: read-only     
        uint32_t rsvd1:1;                                             //Access: read-as-zero     
        uint32_t read_pointer:6;                                      //Access: read-only     
        uint32_t rsvd2:2;                                             //Access: read-as-zero     
        uint32_t write_pointer:6;                                     //Access: read-only     
        uint32_t rsvd3:2;                                             //Access: read-as-zero  
        }f;
} anc_chan_dma_cmdq_fifo_status_t;
    
typedef union anc_chan_link_cmdq_fifo_status_t {                      //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t low:1;                                               //Access: read-only     
        uint32_t full:1;                                              //Access: read-only     
        uint32_t overflow:1;                                          //Access: read-only     
        uint32_t rsvd0:5;                                             //Access: read-as-zero     
        uint32_t level:8;                                             //Access: read-only     
        uint32_t read_pointer:7;                                      //Access: read-only     
        uint32_t rsvd1:1;                                             //Access: read-as-zero     
        uint32_t write_pointer:7;                                     //Access: read-only     
        uint32_t rsvd2:1;                                             //Access: read-as-zero  
        }f;
} anc_chan_link_cmdq_fifo_status_t;
    
typedef union anc_chan_link_bypass_fifo_status_t {                    //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t low:1;                                               //Access: read-only     
        uint32_t full:1;                                              //Access: read-only     
        uint32_t overflow:1;                                          //Access: read-only     
        uint32_t rsvd0:5;                                             //Access: read-as-zero     
        uint32_t level:4;                                             //Access: read-only     
        uint32_t rsvd1:4;                                             //Access: read-as-zero     
        uint32_t read_pointer:3;                                      //Access: read-only     
        uint32_t rsvd2:5;                                             //Access: read-as-zero     
        uint32_t write_pointer:3;                                     //Access: read-only     
        uint32_t rsvd3:5;                                             //Access: read-as-zero  
        }f;
} anc_chan_link_bypass_fifo_status_t;
    
typedef union anc_chan_link_pio_read_fifo_status_t {                  //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t high:1;                                              //Access: read-only     
        uint32_t empty:1;                                             //Access: read-only     
        uint32_t underflow:1;                                         //Access: read-only     
        uint32_t rsvd0:5;                                             //Access: read-as-zero     
        uint32_t level:7;                                             //Access: read-only     
        uint32_t rsvd1:1;                                             //Access: read-as-zero     
        uint32_t read_pointer:6;                                      //Access: read-only     
        uint32_t rsvd2:2;                                             //Access: read-as-zero     
        uint32_t write_pointer:6;                                     //Access: read-only     
        uint32_t rsvd3:2;                                             //Access: read-as-zero  
        }f;
} anc_chan_link_pio_read_fifo_status_t;
    
typedef union anc_chan_dma_debug_fifo_status_t {                      //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_pointer:4;                                     //Access: read-only     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} anc_chan_dma_debug_fifo_status_t;
    
typedef union anc_chan_link_debug_fifo_status_t {                     //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_pointer:5;                                     //Access: read-only     
        uint32_t rsvd0:27;                                            //Access: read-as-zero  
        }f;
} anc_chan_link_debug_fifo_status_t;
    
typedef union anc_chan_link_ecc_status_fifo_status_t {                //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t high:1;                                              //Access: read-only     
        uint32_t empty:1;                                             //Access: read-only     
        uint32_t underflow:1;                                         //Access: read-only     
        uint32_t rsvd0:5;                                             //Access: read-as-zero     
        uint32_t level:7;                                             //Access: read-only     
        uint32_t rsvd1:1;                                             //Access: read-as-zero     
        uint32_t read_pointer:6;                                      //Access: read-only     
        uint32_t rsvd2:2;                                             //Access: read-as-zero     
        uint32_t write_pointer:6;                                     //Access: read-only     
        uint32_t rsvd3:2;                                             //Access: read-as-zero  
        }f;
} anc_chan_link_ecc_status_fifo_status_t;
    
typedef union anc_chan_dma_cmdq_fifo_t {                              //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t push_word:32;                                        //Access: write-only  
        }f;
} anc_chan_dma_cmdq_fifo_t;
    
typedef union anc_chan_link_cmdq_fifo_t {                             //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t push_word:32;                                        //Access: write-only  
        }f;
} anc_chan_link_cmdq_fifo_t;
    
typedef union anc_chan_link_bypass_fifo_t {                           //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t push_word:32;                                        //Access: write-only  
        }f;
} anc_chan_link_bypass_fifo_t;
    
typedef union anc_chan_link_pio_read_fifo_t {                         //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t pop_word:32;                                         //Access: read-only  
        }f;
} anc_chan_link_pio_read_fifo_t;
    
typedef union anc_chan_dma_debug_fifo_t {                             //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t pop_word:32;                                         //Access: read-only  
        }f;
} anc_chan_dma_debug_fifo_t;
    
typedef union anc_chan_link_debug_fifo_t {                            //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t pop_word:32;                                         //Access: read-only  
        }f;
} anc_chan_link_debug_fifo_t;
    
typedef union anc_chan_link_ecc_status_fifo_t {                       //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t pop_word:32;                                         //Access: read-only  
        }f;
} anc_chan_link_ecc_status_fifo_t;
     
    
typedef union anc_dma_config_t {                                      //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t axi_axqos:4;                                         //Access: read-write     
        uint32_t rsvd0:4;                                             //Access: read-as-zero     
        uint32_t burst_size:2;                                        //Access: read-write     
        uint32_t rsvd1:22;                                            //Access: read-as-zero  
        }f;
} anc_dma_config_t;
    
typedef union anc_dma_control_t {                                     //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t start:1;                                             //Access: write-auto-clear     
        uint32_t stop:1;                                              //Access: write-auto-clear     
        uint32_t reset:1;                                             //Access: write-auto-clear     
        uint32_t rsvd0:29;                                            //Access: read-as-zero  
        }f;
} anc_dma_control_t;
    
typedef union anc_dma_status_t {                                      //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t dma_active:1;                                        //Access: read-only     
        uint32_t axi_active:1;                                        //Access: read-only     
        uint32_t aes_active:1;                                        //Access: read-only     
        uint32_t cmdq_enabled:1;                                      //Access: read-only     
        uint32_t aes_enabled:1;                                       //Access: read-only     
        uint32_t dma_direction:1;                                     //Access: read-only     
        uint32_t rsvd0:26;                                            //Access: read-as-zero  
        }f;
} anc_dma_status_t;
    
typedef union anc_dma_aes_status_t {                                  //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t rsvd0:1;                                             //Access: read-as-zero     
        uint32_t key_select:1;                                        //Access: read-only     
        uint32_t key_length:2;                                        //Access: read-only     
        uint32_t rsvd1:1;                                             //Access: read-as-zero     
        uint32_t key_in_ctx:1;                                        //Access: read-only     
        uint32_t iv_in_ctx:1;                                         //Access: read-only     
        uint32_t txt_in_key_ctx:1;                                    //Access: read-only     
        uint32_t txt_in_iv_ctx:1;                                     //Access: read-only     
        uint32_t aes_err:1;                                           //Access: read-only     
        uint32_t rsvd2:22;                                            //Access: read-as-zero  
        }f;
} anc_dma_aes_status_t;
    
typedef union anc_dma_cmdq_count_t {                                  //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t total:32;                                            //Access: read-only  
        }f;
} anc_dma_cmdq_count_t;
    
typedef union anc_dma_cmd_timeout_t {                                 //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t value:31;                                            //Access: read-write     
        uint32_t enable:1;                                            //Access: read-write  
        }f;
} anc_dma_cmd_timeout_t;
    
typedef union anc_dma_cmdq_int_code_t {                               //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t code:16;                                             //Access: read-only     
        uint32_t rsvd0:16;                                            //Access: read-as-zero  
        }f;
} anc_dma_cmdq_int_code_t;
    
typedef union anc_dma_axi_t {                                         //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t address_requests_outstanding:5;                      //Access: read-only     
        uint32_t address_requests_words_remaining:9;                  //Access: read-only     
        uint32_t response:2;                                          //Access: read-only     
        uint32_t write_data_available:7;                              //Access: read-only     
        uint32_t read_space_available:7;                              //Access: read-only     
        uint32_t rsvd0:2;                                             //Access: read-as-zero  
        }f;
} anc_dma_axi_t;
    
typedef union anc_dma_axi_next_t {                                    //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t request_length:4;                                    //Access: read-only     
        uint32_t rsvd0:20;                                            //Access: read-as-zero     
        uint32_t address_high_byte:8;                                 //Access: read-only  
        }f;
} anc_dma_axi_next_t;
    
typedef union anc_dma_axi_next_address_t {                            //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t lower_four_bytes:32;                                 //Access: read-only  
        }f;
} anc_dma_axi_next_address_t;
    
typedef union anc_dma_axi_dma_count_t {                               //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_words:32;                                      //Access: read-only  
        }f;
} anc_dma_axi_dma_count_t;
    
typedef union anc_dma_axi_count_t {                                   //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t read_words:32;                                       //Access: read-only  
        }f;
} anc_dma_axi_count_t;
    
typedef union anc_dma_dlfifo_count_t {                                //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_words:32;                                      //Access: read-only  
        }f;
} anc_dma_dlfifo_count_t;
    
typedef union anc_dma_dlfifo_dma_count_t {                            //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t read_words:32;                                       //Access: read-only  
        }f;
} anc_dma_dlfifo_dma_count_t;
    
typedef union anc_dma_axi_dma_last_count_t {                          //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_words:32;                                      //Access: read-only  
        }f;
} anc_dma_axi_dma_last_count_t;
    
typedef union anc_dma_axi_last_count_t {                              //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t read_words:32;                                       //Access: read-only  
        }f;
} anc_dma_axi_last_count_t;
    
typedef union anc_dma_dlfifo_last_count_t {                           //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_words:32;                                      //Access: read-only  
        }f;
} anc_dma_dlfifo_last_count_t;
    
typedef union anc_dma_dlfifo_dma_last_count_t {                       //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t read_words:32;                                       //Access: read-only  
        }f;
} anc_dma_dlfifo_dma_last_count_t;
     
    
typedef union anc_link_config_t {                                     //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t ddr_mode:1;                                          //Access: read-write     
        uint32_t enable_crc:1;                                        //Access: read-write     
        uint32_t autopad_crc:1;                                       //Access: read-write     
        uint32_t enable_scrambler:1;                                  //Access: read-write     
        uint32_t rsvd0:28;                                            //Access: read-as-zero  
        }f;
} anc_link_config_t;
    
typedef union anc_link_control_t {                                    //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t start:1;                                             //Access: write-auto-clear     
        uint32_t stop:1;                                              //Access: write-auto-clear     
        uint32_t yield:1;                                             //Access: write-auto-clear     
        uint32_t abort:1;                                             //Access: write-auto-clear     
        uint32_t reset:1;                                             //Access: write-auto-clear     
        uint32_t start_cmdq:1;                                        //Access: write-auto-clear     
        uint32_t stop_cmdq:1;                                         //Access: write-auto-clear     
        uint32_t reset_cmdq:1;                                        //Access: write-auto-clear     
        uint32_t start_bypass:1;                                      //Access: write-auto-clear     
        uint32_t stop_bypass:1;                                       //Access: write-auto-clear     
        uint32_t reset_bypass:1;                                      //Access: write-auto-clear     
        uint32_t start_timer:1;                                       //Access: write-auto-clear     
        uint32_t stop_timer:1;                                        //Access: write-auto-clear     
        uint32_t reset_timer:1;                                       //Access: write-auto-clear     
        uint32_t reset_pio_read:1;                                    //Access: write-auto-clear     
        uint32_t rsvd0:17;                                            //Access: read-as-zero  
        }f;
} anc_link_control_t;
    
typedef union anc_link_status_t {                                     //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t cmdq_active:1;                                       //Access: read-only     
        uint32_t fsm_active:1;                                        //Access: read-only     
        uint32_t read_hold_active:1;                                  //Access: read-only     
        uint32_t rsvd0:5;                                             //Access: read-as-zero     
        uint32_t cmdq_state:4;                                        //Access: read-only     
        uint32_t rsvd1:4;                                             //Access: read-as-zero     
        uint32_t fsm_state:6;                                         //Access: read-only     
        uint32_t rsvd2:10;                                            //Access: read-as-zero  
        }f;
} anc_link_status_t;
    
typedef union anc_link_chip_enable_t {                                //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t ce:8;                                                //Access: read-write     
        uint32_t rsvd0:24;                                            //Access: read-as-zero  
        }f;
} anc_link_chip_enable_t;
    
typedef union anc_link_read_status_config_t {                         //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t position:8;                                          //Access: read-write     
        uint32_t polarity:8;                                          //Access: read-write     
        uint32_t status_capture_delay:4;                              //Access: read-write     
        uint32_t ready_sample_delay:4;                                //Access: read-write     
        uint32_t rsvd0:8;                                             //Access: read-as-zero  
        }f;
} anc_link_read_status_config_t;
    
typedef union anc_link_command_address_pulse_timing_t {               //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t ca_setup_time:5;                                     //Access: read-write     
        uint32_t rsvd0:3;                                             //Access: read-as-zero     
        uint32_t ca_hold_time:5;                                      //Access: read-write     
        uint32_t rsvd1:19;                                            //Access: read-as-zero  
        }f;
} anc_link_command_address_pulse_timing_t;
    
typedef union anc_link_sdr_timing_t {                                 //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t cle_ale_setup_time:5;                                //Access: read-write     
        uint32_t rsvd0:3;                                             //Access: read-as-zero     
        uint32_t cle_ale_hold_time:5;                                 //Access: read-write     
        uint32_t rsvd1:3;                                             //Access: read-as-zero     
        uint32_t data_capture_delay:5;                                //Access: read-write     
        uint32_t rsvd2:11;                                            //Access: read-as-zero  
        }f;
} anc_link_sdr_timing_t;
    
typedef union anc_link_sdr_data_timing_t {                            //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t ren_setup_time:5;                                    //Access: read-write     
        uint32_t rsvd0:3;                                             //Access: read-as-zero     
        uint32_t ren_hold_time:5;                                     //Access: read-write     
        uint32_t rsvd1:3;                                             //Access: read-as-zero     
        uint32_t wen_setup_time:5;                                    //Access: read-write     
        uint32_t rsvd2:3;                                             //Access: read-as-zero     
        uint32_t wen_hold_time:5;                                     //Access: read-write     
        uint32_t rsvd3:3;                                             //Access: read-as-zero  
        }f;
} anc_link_sdr_data_timing_t;
    
typedef union anc_link_ddr_data_timing_t {                            //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t ddr_pulse_width:5;                                   //Access: read-write     
        uint32_t rsvd0:27;                                            //Access: read-as-zero  
        }f;
} anc_link_ddr_data_timing_t;
    
typedef union anc_link_ddr_read_timing_t {                            //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t read_preamble:8;                                     //Access: read-write     
        uint32_t read_postamble:8;                                    //Access: read-write     
        uint32_t read_postamble_hold:8;                               //Access: read-write     
        uint32_t read_turnaround:8;                                   //Access: read-write  
        }f;
} anc_link_ddr_read_timing_t;
    
typedef union anc_link_ddr_write_timing_t {                           //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_preamble:8;                                    //Access: read-write     
        uint32_t rsvd0:8;                                             //Access: read-as-zero     
        uint32_t write_postamble:8;                                   //Access: read-write     
        uint32_t rsvd1:8;                                             //Access: read-as-zero  
        }f;
} anc_link_ddr_write_timing_t;
    
typedef union anc_link_dqs_timing_t {                                 //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_dqs_delay:6;                                   //Access: read-write     
        uint32_t rsvd0:26;                                            //Access: read-as-zero  
        }f;
} anc_link_dqs_timing_t;
    
typedef union anc_link_cmdq_count_t {                                 //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t total:32;                                            //Access: read-only  
        }f;
} anc_link_cmdq_count_t;
    
typedef union anc_link_bypass_count_t {                               //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t total:32;                                            //Access: read-only  
        }f;
} anc_link_bypass_count_t;
    
typedef union anc_link_cmd_timeout_t {                                //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t value:31;                                            //Access: read-write     
        uint32_t enable:1;                                            //Access: read-write  
        }f;
} anc_link_cmd_timeout_t;
    
typedef union anc_link_cmdq_int_code_t {                              //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t code:16;                                             //Access: read-only     
        uint32_t rsvd0:16;                                            //Access: read-as-zero  
        }f;
} anc_link_cmdq_int_code_t;
    
typedef union anc_link_timer_t {                                      //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t count:32;                                            //Access: read-only  
        }f;
} anc_link_timer_t;
    
typedef union anc_link_crc_t {                                        //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t parity_data:32;                                      //Access: read-only  
        }f;
} anc_link_crc_t;
    
typedef union anc_link_npl_interface_t {                              //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t cle:1;                                               //Access: read-only     
        uint32_t ale:1;                                               //Access: read-only     
        uint32_t ren:1;                                               //Access: read-only     
        uint32_t wen:1;                                               //Access: read-only     
        uint32_t data_out:8;                                          //Access: read-only     
        uint32_t data_out_enable:1;                                   //Access: read-only     
        uint32_t write_dqs:1;                                         //Access: read-only     
        uint32_t write_dqs_enable:1;                                  //Access: read-only     
        uint32_t pulldown:1;                                          //Access: read-only     
        uint32_t data_in:8;                                           //Access: read-only     
        uint32_t rsvd0:8;                                             //Access: read-as-zero  
        }f;
} anc_link_npl_interface_t;
    
typedef union anc_link_macro_status_t {                               //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t address:8;                                           //Access: read-only     
        uint32_t word_count:8;                                        //Access: read-only     
        uint32_t loop_count:8;                                        //Access: read-only     
        uint32_t active:1;                                            //Access: read-only     
        uint32_t rsvd0:7;                                             //Access: read-as-zero  
        }f;
} anc_link_macro_status_t;
    
typedef union anc_link_bypass_macro_status_t {                        //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t address:8;                                           //Access: read-only     
        uint32_t word_count:8;                                        //Access: read-only     
        uint32_t loop_count:8;                                        //Access: read-only     
        uint32_t active:1;                                            //Access: read-only     
        uint32_t rsvd0:7;                                             //Access: read-as-zero  
        }f;
} anc_link_bypass_macro_status_t;
    
typedef union anc_link_nand_status_t {                                //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t value:8;                                             //Access: read-only     
        uint32_t rsvd0:24;                                            //Access: read-as-zero  
        }f;
} anc_link_nand_status_t;
    
typedef union anc_link_dlfifo_link_count_t {                          //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_bytes:32;                                      //Access: read-only  
        }f;
} anc_link_dlfifo_link_count_t;
    
typedef union anc_link_dlfifo_count_t {                               //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t read_bytes:32;                                       //Access: read-only  
        }f;
} anc_link_dlfifo_count_t;
    
typedef union anc_link_pio_link_count_t {                             //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_bytes:32;                                      //Access: read-only  
        }f;
} anc_link_pio_link_count_t;
    
typedef union anc_link_pio_count_t {                                  //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t read_bytes:32;                                       //Access: read-only  
        }f;
} anc_link_pio_count_t;
    
typedef union anc_link_phy_count_t {                                  //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t write_bytes:32;                                      //Access: read-only  
        }f;
} anc_link_phy_count_t;
    
typedef union anc_link_phy_link_count_t {                             //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t read_bytes:32;                                       //Access: read-only  
        }f;
} anc_link_phy_link_count_t;
    
typedef union anc_link_fsm_counts_t {                                 //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t data_count:16;                                       //Access: read-only     
        uint32_t data_length:16;                                      //Access: read-only  
        }f;
} anc_link_fsm_counts_t;
    
typedef union anc_link_ecc_watermark_t {                              //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t watermark:8;                                         //Access: read-write     
        uint32_t watermark_enable:1;                                  //Access: read-write     
        uint32_t rsvd0:23;                                            //Access: read-as-zero  
        }f;
} anc_link_ecc_watermark_t;
    
typedef union anc_link_ecc_config_t {                                 //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t max_bit_flips:8;                                     //Access: read-write     
        uint32_t allowed_stuck_bits_00:8;                             //Access: read-write     
        uint32_t allowed_stuck_bits_ff:8;                             //Access: read-write     
        uint32_t rsvd0:8;                                             //Access: read-as-zero  
        }f;
} anc_link_ecc_config_t;
     
    
typedef union anc_ecc_p_chien_clock_config_t {                        //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t div_4_chin_clk:7;                                    //Access: write-only     
        uint32_t rsvd0:1;                                             //Access: read-as-zero     
        uint32_t div_3_chin_clk:7;                                    //Access: write-only     
        uint32_t rsvd1:1;                                             //Access: read-as-zero     
        uint32_t div_2_chin_clk:7;                                    //Access: write-only     
        uint32_t rsvd2:1;                                             //Access: read-as-zero     
        uint32_t dont_start_chien_config:7;                           //Access: write-only     
        uint32_t rsvd3:1;                                             //Access: read-as-zero  
        }f;
} anc_ecc_p_chien_clock_config_t;
    
typedef union anc_ecc_p_config_t {                                    //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t rsvd0:1;                                             //Access: read-as-zero     
        uint32_t stop_if_all_error_found:1;                           //Access: write-only     
        uint32_t early_termination_mask:1;                            //Access: write-only     
        uint32_t rsvd1:5;                                             //Access: read-as-zero     
        uint32_t chien_bma_abort:1;                                   //Access: write-only     
        uint32_t rsvd2:7;                                             //Access: read-as-zero     
        uint32_t synd_abort:1;                                        //Access: write-only     
        uint32_t rsvd3:7;                                             //Access: read-as-zero     
        uint32_t mask_int_en:1;                                       //Access: write-only     
        uint32_t rsvd4:7;                                             //Access: read-as-zero  
        }f;
} anc_ecc_p_config_t;
    
typedef union anc_ecc_p_offset1_t {                                   //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t start_add_offset:13;                                 //Access: write-only     
        uint32_t rsvd0:19;                                            //Access: read-as-zero  
        }f;
} anc_ecc_p_offset1_t;
    
typedef union anc_ecc_p_bch_direction_cnfg_t {                        //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t bch_dir_add_th:13;                                   //Access: write-only     
        uint32_t rsvd0:3;                                             //Access: read-as-zero     
        uint32_t bch_dir_cpl1_en:3;                                   //Access: write-only     
        uint32_t rsvd1:13;                                            //Access: read-as-zero  
        }f;
} anc_ecc_p_bch_direction_cnfg_t;
    
typedef union anc_ecc_p_page_idx_t {                                  //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t p_bch_dec_page:5;                                    //Access: write-only     
        uint32_t rsvd0:11;                                            //Access: read-as-zero     
        uint32_t p_bch_dir_rd_page1:5;                                //Access: write-only     
        uint32_t rsvd1:3;                                             //Access: read-as-zero     
        uint32_t p_bch_dir_rd_page2:5;                                //Access: write-only     
        uint32_t rsvd2:3;                                             //Access: read-as-zero  
        }f;
} anc_ecc_p_page_idx_t;
    
typedef union anc_ecc_p_page_structure_t {                            //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t t_code:8;                                            //Access: write-only     
        uint32_t rsvd0:8;                                             //Access: read-as-zero     
        uint32_t n_code:13;                                           //Access: write-only     
        uint32_t rsvd1:3;                                             //Access: read-as-zero  
        }f;
} anc_ecc_p_page_structure_t;
    
typedef union anc_ecc_p_ecc_clk_cnfg_t {                              //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t p_synd_force_clk_en:1;                               //Access: write-only     
        uint32_t rsvd0:7;                                             //Access: read-as-zero     
        uint32_t p_bma_force_clk_en:1;                                //Access: write-only     
        uint32_t rsvd1:7;                                             //Access: read-as-zero     
        uint32_t p_chien_force_clk_en:1;                              //Access: write-only     
        uint32_t rsvd2:15;                                            //Access: read-as-zero  
        }f;
} anc_ecc_p_ecc_clk_cnfg_t;
    
typedef union anc_ecc_p_offset3_t {                                   //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t p_offset3:13;                                        //Access: write-only     
        uint32_t rsvd0:19;                                            //Access: read-as-zero  
        }f;
} anc_ecc_p_offset3_t;
    
typedef union anc_ecc_p_chien_start_offset_t {                        //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t chien_start_offset:16;                               //Access: write-only     
        uint32_t rsvd0:16;                                            //Access: read-as-zero  
        }f;
} anc_ecc_p_chien_start_offset_t;
    
typedef union anc_ecc_p_chien_early_interrupt_t {                     //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t chien_early_int_timing:13;                           //Access: write-only     
        uint32_t rsvd0:19;                                            //Access: read-as-zero  
        }f;
} anc_ecc_p_chien_early_interrupt_t;
    
typedef union anc_ecc_p_offset4_t {                                   //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t chien_last_fix_byte:13;                              //Access: write-only     
        uint32_t rsvd0:19;                                            //Access: read-as-zero  
        }f;
} anc_ecc_p_offset4_t;
    
typedef union anc_ecc_p_status_t {                                    //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t status:1;                                            //Access: read-only     
        uint32_t rsvd0:7;                                             //Access: read-as-zero     
        uint32_t number_of_found_error:7;                             //Access: read-only     
        uint32_t rsvd1:1;                                             //Access: read-as-zero     
        uint32_t bch_ce:2;                                            //Access: read-only     
        uint32_t rsvd2:14;                                            //Access: read-as-zero  
        }f;
} anc_ecc_p_status_t;
    
typedef union anc_ecc_p_bch_dir_cntrs_t {                             //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t zero_to_one_cntr:7;                                  //Access: read-only     
        uint32_t rsvd0:9;                                             //Access: read-as-zero     
        uint32_t one_to_zero_cntr:7;                                  //Access: read-only     
        uint32_t rsvd1:9;                                             //Access: read-as-zero  
        }f;
} anc_ecc_p_bch_dir_cntrs_t;
    
typedef union anc_ecc_p_bma_status_t {                                //RRV: 
    uint32_t all;                                                     //RRM:
    struct {                                                              
        uint32_t bma_degree:7;                                        //Access: read-only     
        uint32_t rsvd0:1;                                             //Access: read-as-zero     
        uint32_t bma_early_termination_indication:1;                  //Access: read-only     
        uint32_t rsvd1:23;                                            //Access: read-as-zero  
        }f;
} anc_ecc_p_bma_status_t;
                                  

/******************************************************************************/
/* Registers typedef structs */
/******************************************************************************/
/* errors in perl template and spds required these to be commented out
     
  
typedef union anc_chan_t {
        uint32_t all[192];
        struct {      
           anc_chan_version_t                                VERSION                        ; // 0x00000000       
           anc_chan_config_t                                 CONFIG                         ; // 0x00000004       
           anc_chan_int_status_t                             INT_STATUS                     ; // 0x00000008       
           anc_chan_int_enable_t                             INT_ENABLE                     ; // 0x0000000c       
           anc_chan_dma_watermarks_t                         DMA_WATERMARKS                 ; // 0x00000010       
           anc_chan_link_watermarks_t                        LINK_WATERMARKS                ; // 0x00000014       
           anc_chan_status_watermarks_t                      STATUS_WATERMARKS              ; // 0x00000018       
           anc_chan_dma_cmdq_fifo_status_t                   DMA_CMDQ_FIFO_STATUS           ; // 0x0000001c       
           anc_chan_link_cmdq_fifo_status_t                  LINK_CMDQ_FIFO_STATUS          ; // 0x00000020       
           anc_chan_link_bypass_fifo_status_t                LINK_BYPASS_FIFO_STATUS        ; // 0x00000024       
           anc_chan_link_pio_read_fifo_status_t              LINK_PIO_READ_FIFO_STATUS      ; // 0x00000028       
           anc_chan_dma_debug_fifo_status_t                  DMA_DEBUG_FIFO_STATUS          ; // 0x0000002c       
           anc_chan_link_debug_fifo_status_t                 LINK_DEBUG_FIFO_STATUS         ; // 0x00000030       
           anc_chan_link_ecc_status_fifo_status_t            LINK_ECC_STATUS_FIFO_STATUS    ; // 0x00000034     
           uint32_t rsvd0[50];     
           anc_chan_dma_cmdq_fifo_t                          DMA_CMDQ_FIFO                  ; // 0x00000100     
           uint32_t rsvd1[15];     
           anc_chan_link_cmdq_fifo_t                         LINK_CMDQ_FIFO                 ; // 0x00000140     
           uint32_t rsvd2[15];     
           anc_chan_link_bypass_fifo_t                       LINK_BYPASS_FIFO               ; // 0x00000180     
           uint32_t rsvd3[15];     
           anc_chan_link_pio_read_fifo_t                     LINK_PIO_READ_FIFO             ; // 0x000001c0     
           uint32_t rsvd4[15];     
           anc_chan_dma_debug_fifo_t                         DMA_DEBUG_FIFO                 ; // 0x00000200     
           uint32_t rsvd5[15];     
           anc_chan_link_debug_fifo_t                        LINK_DEBUG_FIFO                ; // 0x00000240     
           uint32_t rsvd6[15];     
           anc_chan_link_ecc_status_fifo_t                   LINK_ECC_STATUS_FIFO           ; // 0x00000280  
        }r;
} anc_chan_t;
      
  
typedef union anc_dma_t {
        uint32_t all[192];
        struct {      
           anc_dma_config_t                                  CONFIG                         ; // 0x00001000       
           anc_dma_control_t                                 CONTROL                        ; // 0x00001004       
           anc_dma_status_t                                  STATUS                         ; // 0x00001008       
           anc_dma_aes_status_t                              AES_STATUS                     ; // 0x0000100c       
           anc_dma_cmdq_count_t                              CMDQ_COUNT                     ; // 0x00001010       
           anc_dma_cmd_timeout_t                             CMD_TIMEOUT                    ; // 0x00001014       
           anc_dma_cmdq_int_code_t                           CMDQ_INT_CODE                  ; // 0x00001018       
           anc_dma_axi_t                                     AXI                            ; // 0x0000101c       
           anc_dma_axi_next_t                                AXI_NEXT                       ; // 0x00001020       
           anc_dma_axi_next_address_t                        AXI_NEXT_ADDRESS               ; // 0x00001024       
           anc_dma_axi_dma_count_t                           AXI_DMA_COUNT                  ; // 0x00001028       
           anc_dma_axi_count_t                               DMA_AXI_COUNT                  ; // 0x0000102c       
           anc_dma_dlfifo_count_t                            DMA_DLFIFO_COUNT               ; // 0x00001030       
           anc_dma_dlfifo_dma_count_t                        DLFIFO_DMA_COUNT               ; // 0x00001034       
           anc_dma_axi_dma_last_count_t                      AXI_DMA_LAST_COUNT             ; // 0x00001038       
           anc_dma_axi_last_count_t                          DMA_AXI_LAST_COUNT             ; // 0x0000103c       
           anc_dma_dlfifo_last_count_t                       DMA_DLFIFO_LAST_COUNT          ; // 0x00001040       
           anc_dma_dlfifo_dma_last_count_t                   DLFIFO_DMA_LAST_COUNT          ; // 0x00001044  
        }r;
} anc_dma_t;
      
  
typedef union anc_link_t {
        uint32_t all[64];
        struct {      
           anc_link_config_t                                 CONFIG                         ; // 0x00002000       
           anc_link_control_t                                CONTROL                        ; // 0x00002004       
           anc_link_status_t                                 STATUS                         ; // 0x00002008       
           anc_link_chip_enable_t                            CHIP_ENABLE                    ; // 0x0000200c       
           anc_link_read_status_config_t                     READ_STATUS_CONFIG             ; // 0x00002010       
           anc_link_command_address_pulse_timing_t           COMMAND_ADDRESS_PULSE_TIMING   ; // 0x00002014       
           anc_link_sdr_timing_t                             SDR_TIMING                     ; // 0x00002018       
           anc_link_sdr_data_timing_t                        SDR_DATA_TIMING                ; // 0x0000201c       
           anc_link_ddr_data_timing_t                        DDR_DATA_TIMING                ; // 0x00002020       
           anc_link_ddr_read_timing_t                        DDR_READ_TIMING                ; // 0x00002024       
           anc_link_ddr_write_timing_t                       DDR_WRITE_TIMING               ; // 0x00002028       
           anc_link_dqs_timing_t                             DQS_TIMING                     ; // 0x0000202c       
           anc_link_cmdq_count_t                             CMDQ_COUNT                     ; // 0x00002030       
           anc_link_bypass_count_t                           BYPASS_COUNT                   ; // 0x00002034       
           anc_link_cmd_timeout_t                            CMD_TIMEOUT                    ; // 0x00002038       
           anc_link_cmdq_int_code_t                          CMDQ_INT_CODE                  ; // 0x0000203c       
           anc_link_timer_t                                  TIMER                          ; // 0x00002040       
           anc_link_crc_t                                    CRC                            ; // 0x00002044       
           anc_link_npl_interface_t                          NPL_INTERFACE                  ; // 0x00002048       
           anc_link_macro_status_t                           MACRO_STATUS                   ; // 0x0000204c       
           anc_link_bypass_macro_status_t                    BYPASS_MACRO_STATUS            ; // 0x00002050       
           anc_link_nand_status_t                            NAND_STATUS                    ; // 0x00002054       
           anc_link_dlfifo_link_count_t                      DLFIFO_LINK_COUNT              ; // 0x00002058       
           anc_link_dlfifo_count_t                           LINK_DLFIFO_COUNT              ; // 0x0000205c       
           anc_link_pio_link_count_t                         PIO_LINK_COUNT                 ; // 0x00002060       
           anc_link_pio_count_t                              LINK_PIO_COUNT                 ; // 0x00002064       
           anc_link_phy_count_t                              LINK_PHY_COUNT                 ; // 0x00002068       
           anc_link_phy_link_count_t                         PHY_LINK_COUNT                 ; // 0x0000206c       
           anc_link_fsm_counts_t                             FSM_COUNTS                     ; // 0x00002070       
           anc_link_ecc_watermark_t                          ECC_WATERMARK                  ; // 0x00002074       
           anc_link_ecc_config_t                             ECC_CONFIG                     ; // 0x00002078  
        }r;
} anc_link_t;
      
  
typedef union anc_ecc_t {
        uint32_t all[64];
        struct {      
           anc_ecc_p_chien_clock_config_t                    P_CHIEN_CLOCK_CONFIG           ; // 0x00002200     
           uint32_t rsvd0[2];     
           anc_ecc_p_config_t                                P_CONFIG                       ; // 0x0000220c       
           anc_ecc_p_offset1_t                               P_OFFSET1                      ; // 0x00002210     
           uint32_t rsvd1[1];     
           anc_ecc_p_bch_direction_cnfg_t                    P_BCH_DIRECTION_CNFG           ; // 0x00002218       
           anc_ecc_p_page_idx_t                              P_PAGE_IDX                     ; // 0x0000221c       
           anc_ecc_p_page_structure_t                        P_PAGE_STRUCTURE               ; // 0x00002220       
           anc_ecc_p_ecc_clk_cnfg_t                          P_ECC_CLK_CNFG                 ; // 0x00002224       
           anc_ecc_p_offset3_t                               P_OFFSET3                      ; // 0x00002228       
           anc_ecc_p_chien_start_offset_t                    P_CHIEN_START_OFFSET           ; // 0x0000222c       
           anc_ecc_p_chien_early_interrupt_t                 P_CHIEN_EARLY_INTERRUPT        ; // 0x00002230     
           uint32_t rsvd2[2];     
           anc_ecc_p_offset4_t                               P_OFFSET4                      ; // 0x0000223c     
           uint32_t rsvd3[24];     
           anc_ecc_p_status_t                                P_STATUS                       ; // 0x000022a0       
           anc_ecc_p_bch_dir_cntrs_t                         P_BCH_DIR_CNTRS                ; // 0x000022a4     
           uint32_t rsvd4[3];     
           anc_ecc_p_bma_status_t                            P_BMA_STATUS                   ; // 0x000022b4  
        }r;
} anc_ecc_t;
                          

typedef union anc_regs_s{
        struct {           
           anc_chan_t                    CHAN;                         // 0x00000000        
           uint32_t rsvd0[832];       
           anc_dma_t                     DMA;                          // 0x00001000        
           uint32_t rsvd1[832];       
           anc_link_t                    LINK;                         // 0x00002000        
           uint32_t rsvd2[64];       
           anc_ecc_t                     ECC;                          // 0x00002200                           
        }ab;
        uint32_t all[2240];
    };
}anc_regs_t;
*/

/******************************************************************************/
/* Registers enum */
/******************************************************************************/
/* errors in perl template and spds required these to be commented out
    
    
typedef enum anc_chan_regenum {     
           version = 0,       
           config = 1,       
           int_status = 2,       
           int_enable = 3,       
           dma_watermarks = 4,       
           link_watermarks = 5,       
           status_watermarks = 6,       
           dma_cmdq_fifo_status = 7,       
           link_cmdq_fifo_status = 8,       
           link_bypass_fifo_status = 9,       
           link_pio_read_fifo_status = 10,       
           dma_debug_fifo_status = 11,       
           link_debug_fifo_status = 12,       
           link_ecc_status_fifo_status = 13,      
           rsvd_anc_chan_0_50 = 63,     
           dma_cmdq_fifo = 64,      
           rsvd_anc_chan_1_15 = 79,     
           link_cmdq_fifo = 80,      
           rsvd_anc_chan_2_15 = 95,     
           link_bypass_fifo = 96,      
           rsvd_anc_chan_3_15 = 111,     
           link_pio_read_fifo = 112,      
           rsvd_anc_chan_4_15 = 127,     
           dma_debug_fifo = 128,      
           rsvd_anc_chan_5_15 = 143,     
           link_debug_fifo = 144,      
           rsvd_anc_chan_6_15 = 159,     
           link_ecc_status_fifo = 160,   
}anc_chan_regenum;
     
    
typedef enum anc_dma_regenum {     
           config = 0,       
           control = 1,       
           status = 2,       
           aes_status = 3,       
           cmdq_count = 4,       
           cmd_timeout = 5,       
           cmdq_int_code = 6,       
           axi = 7,       
           axi_next = 8,       
           axi_next_address = 9,       
           axi_dma_count = 10,       
           dma_axi_count = 11,       
           dma_dlfifo_count = 12,       
           dlfifo_dma_count = 13,       
           axi_dma_last_count = 14,       
           dma_axi_last_count = 15,       
           dma_dlfifo_last_count = 16,       
           dlfifo_dma_last_count = 17,   
}anc_dma_regenum;
     
    
typedef enum anc_link_regenum {     
           config = 0,       
           control = 1,       
           status = 2,       
           chip_enable = 3,       
           read_status_config = 4,       
           command_address_pulse_timing = 5,       
           sdr_timing = 6,       
           sdr_data_timing = 7,       
           ddr_data_timing = 8,       
           ddr_read_timing = 9,       
           ddr_write_timing = 10,       
           dqs_timing = 11,       
           cmdq_count = 12,       
           bypass_count = 13,       
           cmd_timeout = 14,       
           cmdq_int_code = 15,       
           timer = 16,       
           crc = 17,       
           npl_interface = 18,       
           macro_status = 19,       
           bypass_macro_status = 20,       
           nand_status = 21,       
           dlfifo_link_count = 22,       
           link_dlfifo_count = 23,       
           pio_link_count = 24,       
           link_pio_count = 25,       
           link_phy_count = 26,       
           phy_link_count = 27,       
           fsm_counts = 28,       
           ecc_watermark = 29,       
           ecc_config = 30,   
}anc_link_regenum;
     
    
typedef enum anc_ecc_regenum {     
           p_chien_clock_config = 0,      
           rsvd_anc_ecc_0_2 = 2,     
           p_config = 3,       
           p_offset1 = 4,      
           rsvd_anc_ecc_1_1 = 5,     
           p_bch_direction_cnfg = 6,       
           p_page_idx = 7,       
           p_page_structure = 8,       
           p_ecc_clk_cnfg = 9,       
           p_offset3 = 10,       
           p_chien_start_offset = 11,       
           p_chien_early_interrupt = 12,      
           rsvd_anc_ecc_2_2 = 14,     
           p_offset4 = 15,      
           rsvd_anc_ecc_3_24 = 39,     
           p_status = 40,       
           p_bch_dir_cntrs = 41,      
           rsvd_anc_ecc_4_3 = 44,     
           p_bma_status = 45,   
}anc_ecc_regenum;
     
        
        
        
        
        
        
        
     

*/
#endif
#endif
