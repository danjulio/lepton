/*******************************************************************************
**
**   File NAME: LeptonSDKEmb32OEM.cpp
**
**   CREATED: 6/12/2018
**
**   DESCRIPTION: Lepton SDK Module ported to Arduino
**
**   HISTORY: 6/12/2018 DJD - Initial Draft
**
**   Based on FLIR Systems Inc source code.  Their license is copied below
**   and forms the license for this code.
**
**      Copyright 2011,2012,2013,2014 FLIR Systems - Commercial
**      Vision Systems.  All rights reserved.
**
**      Proprietary - PROPRIETARY - FLIR Systems Inc.
**
**      This document is controlled to FLIR Technology Level 2.
**      The information contained in this document pertains to a
**      dual use product Controlled for export by the Export
**      Administration Regulations (EAR). Diversion contrary to
**      US law is prohibited.  US Department of Commerce
**      authorization is not required prior to export or
**      transfer to foreign persons or parties unless otherwise
**      prohibited.
**
**      Redistribution and use in source and binary forms, with
**      or without modification, are permitted provided that the
**      following conditions are met:
**
**      Redistributions of source code must retain the above
**      copyright notice, this list of conditions and the
**      following disclaimer.
**
**      Redistributions in binary form must reproduce the above
**      copyright notice, this list of conditions and the
**      following disclaimer in the documentation and/or other
**      materials provided with the distribution.
**
**      Neither the name of the FLIR Systems Corporation nor the
**      names of its contributors may be used to endorse or
**      promote products derived from this software without
**      specific prior written permission.
**
**      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
**      CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
**      WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**      WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
**      PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
**      COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
**      DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
**      CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
**      PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
**      USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
**      CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
**      CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
**      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
**      USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
**      OF SUCH DAMAGE.
**
*******************************************************************************/
#include "LeptonSDKEmb32OEM.h"
#include <i2c_t3.h>

#if (USE_BORESIGHT_MEASUREMENT_FUNCTIONS == 1)
#include "math.h"
#endif

LeptonSDKEmb32OEM::LeptonSDKEmb32OEM()
{
}


/**************************************/
/*                AGC                 */
/**************************************/

/**
 * Retrieves the current AGC Enable state from the Camera. The 
 * AGC enable state turns AGC On (enabled) or OFF (disabled).   
 * 
 * @param agcEnableStatePtr
 *               Pointer to variable to update with the camera's
 *               current state.
 *               Range:
 *                  LEP_AGC_DISABLE=0
 *                  LEP_AGC_ENABLE =1
 * 
 * @return LEP_OK if all goes well, otherwise an error code.
 */
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_AGC_ENABLE_E_PTR agcEnableStatePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* attribute is an enum, so 32-bit value */

   /* Validate Parameter(s)
   */
   if( agcEnableStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Read the Camera's AGC Enable State
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )agcEnableStatePtr,
                              attributeWordLength );
   return( result );
}

/**
 * Sets the current AGC Enable state on the Camera.  The 
 * AGC enable state turns AGC On (enabled) or OFF (disabled).
 * 
 * @param agcEnableState 
 *             Specifies the enable state to set the camera' AGC
 *             Range:
 *                LEP_AGC_DISABLE=0
 *                LEP_AGC_ENABLE =1
 * 
 * @return LEP_OK if all goes well, otherwise an error code.
 */
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_AGC_ENABLE_E agcEnableState )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* 2 16-bit value */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) agcEnableState;

   /* Validate Parameter(s)
   */
   if( agcEnableState >= LEP_END_AGC_ENABLE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's AGC Enable State
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}




LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcPolicy( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_AGC_POLICY_E_PTR agcPolicyPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* 2 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcPolicyPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Policy
   */
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_POLICY,
                              ( LEP_ATTRIBUTE_T_PTR )agcPolicyPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcPolicy( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_AGC_POLICY_E agcPolicy )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* 2 16-bit value */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) agcPolicy;

   /* Bounds Check
   */
   if( agcPolicy >= LEP_END_AGC_POLICY )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's AGC Policy
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_POLICY,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}



LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcROI( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                          LEP_AGC_ROI_T_PTR agcROIPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* four 16-bit values */

   /* Validate Parameter(s)
   */
   if( agcROIPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC ROI
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_ROI,
                              ( LEP_ATTRIBUTE_T_PTR )agcROIPtr,
                              attributeWordLength );
   //return(sizeof(*agcROIPtr));
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcROI( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                          LEP_AGC_ROI_T agcROI )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* four 16-bit values */

   _LEP_data[3] = agcROI.endRow;
   _LEP_data[2] = agcROI.endCol;
   _LEP_data[1] = agcROI.startRow;
   _LEP_data[0] = agcROI.startCol;

   /* Perform Command
   ** Writing the Camera's AGC ROI
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_ROI,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );


   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcLinearHistogramClipPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_UINT16 *agcLinearHistogramClipPercentPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcLinearHistogramClipPercentPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Histogram Clip Percent
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HISTOGRAM_CLIP_PERCENT,
                              ( LEP_ATTRIBUTE_T_PTR )agcLinearHistogramClipPercentPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcLinearHistogramClipPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_UINT16 agcLinearHistogramClipPercent )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC Linear Histogram Clip Percent
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HISTOGRAM_CLIP_PERCENT,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcLinearHistogramClipPercent,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcLinearHistogramTailSize( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 *agcLinearHistogramTailSizePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcLinearHistogramTailSizePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Histogram Tail Size
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HISTOGRAM_TAIL_SIZE,
                              ( LEP_ATTRIBUTE_T_PTR )agcLinearHistogramTailSizePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcLinearHistogramTailSize( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_UINT16 agcLinearHistogramTailSize )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC Linear Histogram Clip Percent
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HISTOGRAM_TAIL_SIZE,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcLinearHistogramTailSize,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHistogramStatistics( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_AGC_HISTOGRAM_STATISTICS_T_PTR *agcHistogramStatisticsPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* four 16-bit values */

   /* Validate Parameter(s)
   */
   if( agcHistogramStatisticsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Histogram Statistics
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_STATISTICS,
                              ( LEP_ATTRIBUTE_T_PTR )agcHistogramStatisticsPtr,
                              attributeWordLength );
   return( result );
}



/* Linear Policy Controls
*/

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcLinearMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_UINT16 *agcLinearMaxGainPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcLinearMaxGainPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Max Gain
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_MAX_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR )agcLinearMaxGainPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcLinearMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_UINT16 agcLinearMaxGain )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC Linear Max Gain
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_MAX_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcLinearMaxGain,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcLinearMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_UINT16 *agcLinearMidPointPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcLinearMidPointPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Midpoint
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_MIDPOINT,
                              ( LEP_ATTRIBUTE_T_PTR )agcLinearMidPointPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcLinearMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_UINT16 agcLinearMidPoint )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC Linear Midpoint
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_MIDPOINT,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcLinearMidPoint,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcLinearDampeningFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_UINT16 *agcLinearDampeningFactorPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcLinearDampeningFactorPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC Linear Dampening Factor
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_DAMPENING_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )agcLinearDampeningFactorPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcLinearDampeningFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_UINT16 agcLinearDampeningFactor )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC Linear Histogram Dampening Factor
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_LINEAR_DAMPENING_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcLinearDampeningFactor,
                              attributeWordLength
                              );
   return( result );
}



/* Heq Policy Controls
*/

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHeqDampingFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 *agcHeqDampingFactorPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqDampingFactorPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Dampening Factor
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_DAMPENING_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqDampingFactorPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcHeqDampingFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 agcHeqDampingFactor )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Histogram Dampening Factor
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_DAMPENING_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqDampingFactor,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHeqMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_UINT16 *agcHeqMaxGainPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqMaxGainPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Max Gain
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_MAX_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqMaxGainPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcHeqMaxGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_UINT16 agcHeqMaxGain )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Max Gain
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_MAX_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqMaxGain,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHeqClipLimitHigh( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 *agcHeqClipLimitHighPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqClipLimitHighPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Clip Limit High
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_CLIP_LIMIT_HIGH,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqClipLimitHighPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcHeqClipLimitHigh( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 agcHeqClipLimitHigh )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Clip Limit High
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_CLIP_LIMIT_HIGH,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqClipLimitHigh,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHeqClipLimitLow( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_UINT16 *agcHeqClipLimitLowPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqClipLimitLowPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Clip Limit Low
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_CLIP_LIMIT_LOW,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqClipLimitLowPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcHeqClipLimitLow( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_UINT16 agcHeqClipLimitLow )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Clip Limit Low
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_CLIP_LIMIT_LOW,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqClipLimitLow,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHeqBinExtension( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_UINT16 *agcHeqBinExtensionPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqBinExtensionPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Bin Extension
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_BIN_EXTENSION,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqBinExtensionPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcHeqBinExtension( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_UINT16 agcHeqBinExtension )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Bin Extension
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_BIN_EXTENSION,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqBinExtension,
                              attributeWordLength
                              );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHeqMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_UINT16 *agcHeqMidPointPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   /* Validate Parameter(s)
   */
   if( agcHeqMidPointPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's AGC HEQ Midpoint
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_MIDPOINT,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqMidPointPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcHeqMidPoint( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_UINT16 agcHeqMidPoint )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */


   /* Perform Command
   ** Writing the Camera's AGC HEQ Midpoint
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_MIDPOINT,
                              ( LEP_ATTRIBUTE_T_PTR ) & agcHeqMidPoint,
                              attributeWordLength
                              );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHeqEmptyCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_AGC_HEQ_EMPTY_COUNT_T_PTR emptyCountPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_EMPTY_COUNTS,
                              ( LEP_ATTRIBUTE_T_PTR )emptyCountPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcHeqEmptyCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_AGC_HEQ_EMPTY_COUNT_T emptyCount )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_EMPTY_COUNTS,
                              ( LEP_ATTRIBUTE_T_PTR ) & emptyCount,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHeqNormalizationFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_AGC_HEQ_NORMALIZATION_FACTOR_T_PTR normalizationFactorPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_NORMALIZATION_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )normalizationFactorPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcHeqNormalizationFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_AGC_HEQ_NORMALIZATION_FACTOR_T normalizationFactor )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_NORMALIZATION_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR ) & normalizationFactor,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHeqScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_AGC_HEQ_SCALE_FACTOR_E_PTR scaleFactorPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_SCALE_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )scaleFactorPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcHeqScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_AGC_HEQ_SCALE_FACTOR_E scaleFactor )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) scaleFactor;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_SCALE_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcCalcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_AGC_ENABLE_E_PTR agcCalculationEnableStatePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if( agcCalculationEnableStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_CALC_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )agcCalculationEnableStatePtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcCalcEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_AGC_ENABLE_E agcCalculationEnableState )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) agcCalculationEnableState;

   if( agcCalculationEnableState >= LEP_END_AGC_ENABLE )
   {
      return( LEP_RANGE_ERROR );
   }

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_CALC_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAgcHeqLinearPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 *agcHeqLinearPercentPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   if( agcHeqLinearPercentPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_LINEAR_PERCENT,
                              ( LEP_ATTRIBUTE_T_PTR )agcHeqLinearPercentPtr,
                              attributeWordLength );

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAgcHeqLinearPercent( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 agcHeqLinearPercent)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_AGC_HEQ_LINEAR_PERCENT,
                              ( LEP_ATTRIBUTE_T_PTR )&agcHeqLinearPercent,
                              attributeWordLength );

   return(result);
}


/**************************************/
/*           I2C_Protocol             */
/**************************************/
LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_SelectDevice(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr, 
                                LEP_PROTOCOL_DEVICE_E device)
{
    LEP_RESULT result;

    result = LEP_I2C_MasterSelectDevice( portDescPtr, device );

    return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_OpenPort(LEP_UINT16 portID,
                            LEP_UINT16 *baudRateInkHz,
                            LEP_UINT8* deviceAddress)
{
   LEP_RESULT result;
   LEP_UINT16 statusReg;

   result = LEP_I2C_MasterOpen( portID, baudRateInkHz );

   if(result != LEP_OK)
   {
      return(LEP_COMM_INVALID_PORT_ERROR);
   }

   *deviceAddress = 0x2a;
   result = LEP_I2C_MasterReadData( portID,
                                *deviceAddress,
                                LEP_I2C_STATUS_REG,
                                &statusReg,
                                1 );
   
   if(result != LEP_OK)
   {
      /*
       *    Try 0x00 as the device address if 0x2a didn't work. In this case, we are in Virgin Boot Mode.
       *
       */
      *deviceAddress = 0x00;
      result = LEP_I2C_MasterReadData( portID,
                                   *deviceAddress,
                                   LEP_I2C_STATUS_REG,
                                   &statusReg,
                                   1 );
      if(result != LEP_OK)
      {
         return(LEP_COMM_NO_DEV);
      }
   }

    return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_ClosePort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result;

    result =LEP_I2C_MasterClose( portDescPtr );

    return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_ResetPort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result;

    result = LEP_I2C_MasterReset( portDescPtr );

    return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_GetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_COMMAND_ID commandID, 
                                LEP_ATTRIBUTE_T_PTR attributePtr,
                                LEP_UINT16 attributeWordLength)
{
    LEP_RESULT result;
    LEP_UINT16 statusReg;
    LEP_INT16 statusCode;
    LEP_UINT32 done;
    LEP_UINT16 crcExpected, crcActual;

    /* Implement the Lepton TWI READ Protocol
    */
    /* First wait until the Camera is ready to receive a new
    ** command by polling the STATUS REGISTER BUSY Bit until it
    ** reports NOT BUSY.
    */ 

    do
    {
        /* Read the Status REGISTER and peek at the BUSY Bit
        */ 
        result = LEP_I2C_MasterReadData( portDescPtr->portID,
                                         portDescPtr->deviceAddress,
                                         LEP_I2C_STATUS_REG,
                                         &statusReg,
                                         1 );
        if(result != LEP_OK)
        {
           return(result);
        }
        done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;

    }while( !done );

    /* Set the Lepton's DATA LENGTH REGISTER first to inform the
    ** Lepton Camera how many 16-bit DATA words we want to read.
    */ 
    result = LEP_I2C_MasterWriteData( portDescPtr->portID,
                                      portDescPtr->deviceAddress,
                                      LEP_I2C_DATA_LENGTH_REG, 
                                      &attributeWordLength, 
                                      1);
    if(result != LEP_OK)
    {
       return(result);
    }
    /* Now issue the GET Attribute Command
    */ 
    result = LEP_I2C_MasterWriteData( portDescPtr->portID,
                                      portDescPtr->deviceAddress,
                                      LEP_I2C_COMMAND_REG, 
                                      &commandID, 
                                      1);

    if(result != LEP_OK)
    {
       return(result);
    }

    /* Now wait until the Camera has completed this command by
    ** polling the statusReg REGISTER BUSY Bit until it reports NOT
    ** BUSY.
    */ 
    do
    {
        /* Read the statusReg REGISTER and peek at the BUSY Bit
        */ 
        result = LEP_I2C_MasterReadData( portDescPtr->portID,
                                         portDescPtr->deviceAddress,
                                         LEP_I2C_STATUS_REG,
                                         &statusReg,
                                         1 );

        if(result != LEP_OK)
        {
           return(result);
        }
        done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;

    }while( !done );
    

    /* Check statusReg word for Errors?
    */ 
    statusCode = (statusReg >> 8) ? ((statusReg >> 8) | 0xFF00) : 0;
    if(statusCode)
    {
      return((LEP_RESULT)statusCode);
    }

    /* If NO Errors then READ the DATA from the DATA REGISTER(s)
    */ 
    if( attributeWordLength <= 16 )
    {
        /* Read from the DATA Registers - always start from DATA 0
        ** Little Endean
        */ 
        result = LEP_I2C_MasterReadData(portDescPtr->portID,
                                        portDescPtr->deviceAddress,
                                        LEP_I2C_DATA_0_REG,
                                        attributePtr,
                                        attributeWordLength );
    }
    else if( attributeWordLength <= 1024 )
    {
        /* Read from the DATA Block Buffer
        */ 
      result = LEP_I2C_MasterReadData(portDescPtr->portID,
                                      portDescPtr->deviceAddress,
                                      LEP_I2C_DATA_BUFFER_0,
                                      attributePtr,
                                      attributeWordLength );
    }

//    if(result == LEP_OK && attributeWordLength > 0)
//    {
//       /* Check CRC */
//       result = LEP_I2C_MasterReadData( portDescPtr->portID,
//                                        portDescPtr->deviceAddress,
//                                        LEP_I2C_DATA_CRC_REG,
//                                        &crcExpected,
//                                        1);
//       crcActual = (LEP_UINT16)CalcCRC16Words(attributeWordLength, (short*)attributePtr);
//
//       /* Check for 0 in the register in case the camera does not support CRC check
//       */
//       if(crcExpected != 0 && crcExpected != crcActual)
//       {
//          return(LEP_CHECKSUM_ERROR);
//       }
//       
//    }

    return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_SetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_COMMAND_ID commandID, 
                                LEP_ATTRIBUTE_T_PTR attributePtr,
                                LEP_UINT16 attributeWordLength)
{
    LEP_RESULT result;
    LEP_UINT16 statusReg;
    LEP_INT16 statusCode;
    LEP_UINT32 done;
    LEP_UINT16 timeoutCount = LEPTON_I2C_COMMAND_BUSY_WAIT_COUNT;

    /* Implement the Lepton TWI WRITE Protocol
    */
    /* First wait until the Camera is ready to receive a new
    ** command by polling the STATUS REGISTER BUSY Bit until it
    ** reports NOT BUSY.
    */ 
    do
    {
        /* Read the Status REGISTER and peek at the BUSY Bit
        */ 
        result = LEP_I2C_MasterReadData( portDescPtr->portID,
                                         portDescPtr->deviceAddress,
                                         LEP_I2C_STATUS_REG,
                                         &statusReg,
                                         1 );
        if(result != LEP_OK)
        {
           return(result);
        }
        done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;
        /* Add timout check */
        if( timeoutCount-- == 0 )
        {
            /* Timed out waiting for command busy to go away
            */ 
          return(LEP_TIMEOUT_ERROR);

        }
    }while( !done );

    if( result == LEP_OK )
    {
        /* Now WRITE the DATA to the DATA REGISTER(s)
        */ 
        if( attributeWordLength <= 16 )
        {
            /* WRITE to the DATA Registers - always start from DATA 0
            */ 
            result = LEP_I2C_MasterWriteData(portDescPtr->portID,
                                             portDescPtr->deviceAddress,
                                             LEP_I2C_DATA_0_REG,
                                             attributePtr,
                                             attributeWordLength );
        }
        else if( attributeWordLength <= 1024 )
        {
            /* WRITE to the DATA Block Buffer
            */     
            result = LEP_I2C_MasterWriteData(portDescPtr->portID,
                                             portDescPtr->deviceAddress,
                                             LEP_I2C_DATA_BUFFER_0,
                                             attributePtr,
                                             attributeWordLength );

        }
        else
            result = LEP_RANGE_ERROR;
    }

    if( result == LEP_OK )
    {
        /* Set the Lepton's DATA LENGTH REGISTER first to inform the
        ** Lepton Camera how many 16-bit DATA words we want to read.
        */ 
        result = LEP_I2C_MasterWriteData( portDescPtr->portID,
                                          portDescPtr->deviceAddress,
                                          LEP_I2C_DATA_LENGTH_REG, 
                                          &attributeWordLength, 
                                          1);

        if( result == LEP_OK )
        {
            /* Now issue the SET Attribute Command
            */ 
            result = LEP_I2C_MasterWriteData( portDescPtr->portID,
                                              portDescPtr->deviceAddress,
                                              LEP_I2C_COMMAND_REG, 
                                              &commandID, 
                                              1);
			
            if( result == LEP_OK )
            {
                /* Now wait until the Camera has completed this command by
                ** polling the statusReg REGISTER BUSY Bit until it reports NOT
                ** BUSY.
                */ 
                do
                {
                    /* Read the statusReg REGISTER and peek at the BUSY Bit
                    */ 
                    result = LEP_I2C_MasterReadData( portDescPtr->portID,
                                                     portDescPtr->deviceAddress,
                                                     LEP_I2C_STATUS_REG,
                                                     &statusReg,
                                                     1 );
                    if(result != LEP_OK)
                    {
                       return(result);
                    }
                    done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;

                }while( !done );

                    /* Check statusReg word for Errors?
                   */ 
                   statusCode = (statusReg >> 8) ? ((statusReg >> 8) | 0xFF00) : 0;
                   if(statusCode)
                   {
                     return((LEP_RESULT)statusCode);
                   }

            }
        }
    }

    /* Check statusReg word for Errors?
    */

    return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_RunCommand(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_COMMAND_ID commandID)
{
    LEP_RESULT result;
    LEP_UINT16 statusReg;
    LEP_INT16 statusCode;
    LEP_UINT32 done;
    LEP_UINT16 timeoutCount = LEPTON_I2C_COMMAND_BUSY_WAIT_COUNT;

    /* Implement the Lepton TWI WRITE Protocol
    */
    /* First wait until the Camera is ready to receive a new
    ** command by polling the STATUS REGISTER BUSY Bit until it
    ** reports NOT BUSY.
    */ 
    do
    {
        /* Read the Status REGISTER and peek at the BUSY Bit
        */ 
        result = LEP_I2C_MasterReadRegister( portDescPtr->portID,
                                             portDescPtr->deviceAddress,
                                             LEP_I2C_STATUS_REG,
                                             &statusReg);
        if(result != LEP_OK)
        {
            return(result);
        }
        done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;
        /* Add timout check */
        if( timeoutCount-- == 0 )
        {
            /* Timed out waiting for command busy to go away
            */ 

        }
    }while( !done );

    if( result == LEP_OK )
    {
        /* Set the Lepton's DATA LENGTH REGISTER first to inform the
        ** Lepton Camera no 16-bit DATA words being transferred.
        */ 
        result = LEP_I2C_MasterWriteRegister( portDescPtr->portID,
                                              portDescPtr->deviceAddress,
                                              LEP_I2C_DATA_LENGTH_REG, 
                                              (LEP_UINT16)0);

        if( result == LEP_OK )
        {
            /* Now issue the Run Command
            */ 
            result = LEP_I2C_MasterWriteRegister( portDescPtr->portID,
                                                  portDescPtr->deviceAddress,
                                                  LEP_I2C_COMMAND_REG, 
                                                  commandID);
            if( result == LEP_OK )
            {
                /* Now wait until the Camera has completed this command by
                ** polling the statusReg REGISTER BUSY Bit until it reports NOT
                ** BUSY.
                */ 
                do
                {
                    /* Read the statusReg REGISTER and peek at the BUSY Bit
                    */ 
                    result = LEP_I2C_MasterReadRegister( portDescPtr->portID,
                                                         portDescPtr->deviceAddress,
                                                         LEP_I2C_STATUS_REG,
                                                         &statusReg);
                    if(result != LEP_OK)
                    {
                        return(result);
                    }
                    done = (statusReg & LEP_I2C_STATUS_BUSY_BIT_MASK)? 0: 1;
                    /* Timeout? */

                }while( !done );

                statusCode = (statusReg >> 8) ? ((statusReg >> 8) | 0xFF00) : 0;
                if(statusCode)
                {
                  return((LEP_RESULT)statusCode);
                }
            }
        }
    }

    /* Check statusReg word for Errors?
    */

    
    return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_DirectReadRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_UINT16 regAddress,
                                      LEP_UINT16 *regValue)
{
   LEP_RESULT result = LEP_OK;

   result = LEP_I2C_MasterReadRegister( portDescPtr->portID,
                                        portDescPtr->deviceAddress,
                                        regAddress,
                                        regValue);

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_GetPortStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result = LEP_OK;

    return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_GetDeviceAddress(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_UINT8* deviceAddress)
{
   LEP_RESULT result = LEP_OK;

   if(deviceAddress == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }
   *deviceAddress = portDescPtr->deviceAddress;

   return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_DirectWriteBuffer(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_ATTRIBUTE_T_PTR attributePtr,
                                     LEP_UINT16 attributeWordLength)
{
   LEP_RESULT result = LEP_OK;

   /* WRITE to the DATA Block Buffer
   */     
   result = LEP_I2C_MasterWriteData(portDescPtr->portID,
                                    portDescPtr->deviceAddress,
                                    LEP_I2C_DATA_BUFFER_0,
                                    attributePtr,
                                    attributeWordLength );

  

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_DirectWriteRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_UINT16 regAddress,
                                       LEP_UINT16 regValue)
{
   LEP_RESULT result = LEP_OK;

   result = LEP_I2C_MasterWriteRegister(portDescPtr->portID,
                                        portDescPtr->deviceAddress,
                                        regAddress, 
                                        regValue);
   return(result);
}


/**************************************/
/*            I2C_Service             */
/**************************************/
LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_MasterSelectDevice(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr, 
                                      LEP_PROTOCOL_DEVICE_E device)
{
    LEP_RESULT result = LEP_OK;

    /* Do any device-specific calls to implement a close operation
    */ 
#ifdef IMPLEMENTS_SELECT_DEVICE
	result = DEV_I2C_MasterSelectDevice(device);
#else
    result = LEP_UNDEFINED_FUNCTION_ERROR;
#endif

    return(result);
}

/* Driver Open
*/ 
LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_MasterOpen(LEP_UINT16 portID, 
                              LEP_UINT16 *portBaudRate)
{
    LEP_RESULT result;

    /* Call the I2C Device-Specific Driver to open device as a
    ** Master
    */ 
    result = DEV_I2C_MasterInit( portID, portBaudRate );

    return(result);
}

/* Driver Close
*/ 
LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_MasterClose(LEP_CAMERA_PORT_DESC_T_PTR portDescriptorPtr)
{
    LEP_RESULT result = LEP_OK;

    /* Do any device-specific calls to implement a close operation
    */ 
	result = DEV_I2C_MasterClose();
    return(result);
}

/* Driver Reset
*/ 
LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_MasterReset(LEP_CAMERA_PORT_DESC_T_PTR portDescriptorPtr )
{
    LEP_RESULT result = LEP_OK;

    return(result);
}

/**
 * Driver Read
 *    Use Lepton I2C protocol for READ starting from current
 *      location
 * 
 * @param portID        User-defined parameter to identify one of multiple ports
 * 
 * @param deviceAddress This is the Lepton TWI/CCI (I2C) device address.
 * 
 * @param subAddress    Specifies the Lepton Register Address to write to
 * 
 * @param dataPtr       Pointer to the DATA buffer that is filled by this command
 * 
 * @param dataLength    Number of 16-bit words to read.
 * 
 * @return 
 */
LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_MasterReadData(LEP_UINT16 portID,
                                  LEP_UINT8  deviceAddress, 
                                  LEP_UINT16 subAddress, 
                                  LEP_UINT16 *dataPtr,
                                  LEP_UINT16 dataLength)
{
    LEP_RESULT result = LEP_OK;
    LEP_UINT16 transactionStatus;
    LEP_UINT16 numWordsRead;

    result = DEV_I2C_MasterReadData(portID,
                                    deviceAddress,
                                    subAddress,
                                    dataPtr,
                                    dataLength,
                                    &numWordsRead,
                                    &transactionStatus
                                   );

    return(result);
}

/**
 * Driver Write
 * 
 * @param portID     User-defined parameter to identify one of multiple ports
 * 
 * @param subAddress Specifies the Lepton Register Address to write to
 * 
 * @param dataPtr    Pointer to a DATA buffer to source the transfers
 * 
 * @param dataLength Specifies the number of 16-bit words to write from the bufffer
 * 
 * @return LEP_RESULT   LEP_OK if all goes well; otherwise a Lepton error code.
 */
LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_MasterWriteData(LEP_UINT16 portID,
                                   LEP_UINT8  deviceAddress, 
                                   LEP_UINT16 subAddress, 
                                   LEP_UINT16 *dataPtr,
                                   LEP_UINT16 dataLength)
{
    LEP_RESULT result = LEP_OK;
    LEP_UINT16 transactionStatus;
    LEP_UINT16 numWordsWritten;

    result = DEV_I2C_MasterWriteData(portID,
                                     deviceAddress,
                                     subAddress,
                                     dataPtr,
                                     dataLength,
                                     &numWordsWritten,
                                     &transactionStatus
                                    );
    return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_MasterReadRegister(LEP_UINT16 portID,
                                      LEP_UINT8  deviceAddress, 
                                      LEP_UINT16 regAddress,
                                      LEP_UINT16 *regValue)
{
    LEP_RESULT result = LEP_OK;
    LEP_UINT16 transactionStatus;

    result = DEV_I2C_MasterReadRegister(portID,
                                        deviceAddress,
                                        regAddress,
                                        regValue,
                                        &transactionStatus
                                       );
    return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_MasterWriteRegister(LEP_UINT16 portID,
                                       LEP_UINT8  deviceAddress, 
                                       LEP_UINT16 regAddress,
                                       LEP_UINT16 regValue)
{
    LEP_RESULT result = LEP_OK;
    LEP_UINT16 transactionStatus;

    result = DEV_I2C_MasterWriteRegister(portID,
                                         deviceAddress,
                                         regAddress,
                                         regValue,
                                         &transactionStatus
                                        );
    return(result);
}


/* Driver Status
*/ 
LEP_RESULT LeptonSDKEmb32OEM::LEP_I2C_MasterStatus(LEP_UINT16 portID,
                                LEP_UINT16 *portStatus )
{
    LEP_RESULT result = LEP_OK;

    return(result);
}


/**************************************/
/*                OEM                 */
/**************************************/
/******************************************************************************/
/**
 * Power Down the Camera by asserting the POWERDOWN condition.
 * 
 * @param portDescPtr
 * 
 * @return 
 */
LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemPowerDown( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_POWER_DOWN );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemPowerOn( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;

   result = LEP_DirectWriteRegister( portDescPtr, 0x0, 0x0 );

   return( result );
}

/**
 * Places the Camera into the Stand By condition.
 * 
 * @param portDescPtr
 * 
 * @return 
 */
LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemStandby( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;


   //result= LEP_RunCommand( portDescPtr, (LEP_COMMAND_ID)LEP_CID_OEM_STANDBY );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemReboot( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_REBOOT );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemLowPowerMode1( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_LOW_POWER_MODE_1 );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemLowPowerMode2( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_LOW_POWER_MODE_2 );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemBit( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_BIT_TEST );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemMaskRevision( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_OEM_MASK_REVISION_T_PTR oemMaskRevisionPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* one 16-bit value */

   /* Validate Parameter(s)
   */
   if( oemMaskRevisionPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Chip Mask Revision
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_MASK_REVISION,
                              ( LEP_ATTRIBUTE_T_PTR )oemMaskRevisionPtr,
                              attributeWordLength );
   return( result );
}
#if 0
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemMasterID( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_OEM_MASTER_ID_T_PTR oemMasterIDPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 48; /* 96 bytes  */

   /* Validate Parameter(s)
   */
   if( oemMasterIDPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Chip Master ID
   */
   result = LEP_GetAttribute( portDescPtr,
                             ( LEP_COMMAND_ID )LEP_CID_OEM_MASTER_ID,
                             ( LEP_ATTRIBUTE_T_PTR )oemMasterIDPtr,
                             attributeWordLength );
   return( result );
}
#endif

#if USE_DEPRECATED_PART_NUMBER_INTERFACE
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemFlirPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16; /* 32 bytes */

   /* Validate Parameter(s)
   */
   if( oemPartNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Part Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_FLIR_PART_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )oemPartNumberPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemCustPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;   /* 32 bytes */

   if( oemPartNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_CUST_PART_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )oemPartNumberPtr,
                              attributeWordLength );

   return( result );
}
#else
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemFlirPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16; /* 32 bytes */

   /* Validate Parameter(s)
   */
   if( oemPartNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Part Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_FLIR_PART_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )oemPartNumberPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemCustPartNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_OEM_PART_NUMBER_T_PTR oemPartNumberPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;   /* 32 bytes */

   if( oemPartNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_CUST_PART_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )oemPartNumberPtr,
                              attributeWordLength );


   return( result );
}
#endif

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemSoftwareVersion( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_OEM_SW_VERSION_T *oemSoftwareVersionPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* 4 16-bit words to contain 64-bits */

   /* Validate Parameter(s)
   */
   if( oemSoftwareVersionPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Software Version
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_SOFTWARE_VERSION,
                              ( LEP_ATTRIBUTE_T_PTR )oemSoftwareVersionPtr,
                              attributeWordLength );
   return( result );
}


#if 0
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemVendorID(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_OEM_VENDORID_T *oemVendorIDPtr)
{
   LEP_RESULT  result = LEP_OK;

   return( result );
}
#endif



LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemVideoOutputEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_ENABLE_E_PTR oemVideoOutputEnablePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputEnablePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video output enable state
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_ENABLE,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoOutputEnablePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemVideoOutputEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_ENABLE_E oemVideoOutputEnable )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) oemVideoOutputEnable;

   /* Validate Parameter(s)
   */
   if( oemVideoOutputEnable >= LEP_END_VIDEO_OUTPUT_ENABLE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_ENABLE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_FORMAT_E_PTR oemVideoOutputFormatPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputFormatPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video output format
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_FORMAT,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoOutputFormatPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_FORMAT_E oemVideoOutputFormat )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) oemVideoOutputFormat;

   /* Validate Parameter(s)
   */
   if( oemVideoOutputFormat >= LEP_END_VIDEO_OUTPUT_FORMAT )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output format
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_FORMAT,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemVideoOutputSource( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_SOURCE_E_PTR oemVideoOutputSourcePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputSourcePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video output source selection
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_SOURCE,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoOutputSourcePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemVideoOutputSource( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_VIDEO_OUTPUT_SOURCE_E oemVideoOutputSource )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) oemVideoOutputSource;

   /* Validate Parameter(s)
   */
   if( oemVideoOutputSource >= LEP_END_VIDEO_OUTPUT_SOURCE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output source selection
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_SOURCE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemVideoOutputSourceConstant( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_UINT16 oemVideoOutputSourceConstant )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* 1 16-bit values for constant value */


   /* Perform Command
   ** Reading the Camera's current video output source selection
   */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_CONSTANT,
                              ( LEP_ATTRIBUTE_T_PTR ) & oemVideoOutputSourceConstant,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemVideoOutputSourceConstant( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                LEP_UINT16 *oemVideoOutputSourceConstPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* 1 16-bit values for constant value */

   if( oemVideoOutputSourceConstPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output source selection
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_CONSTANT,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoOutputSourceConstPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemVideoOutputChannel( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_OEM_VIDEO_OUTPUT_CHANNEL_E_PTR oemVideoOutputChannelPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoOutputChannelPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video output channel selection
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_CHANNEL,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoOutputChannelPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemVideoOutputChannel( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_OEM_VIDEO_OUTPUT_CHANNEL_E oemVideoOutputChannel )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) oemVideoOutputChannel;

   /* Validate Parameter(s)
   */
   if( oemVideoOutputChannel >= LEP_END_VIDEO_OUTPUT_CHANNEL )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output channel selection
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_OUTPUT_CHANNEL,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemVideoGammaEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_VIDEO_GAMMA_ENABLE_E_PTR oemVideoGammaEnablePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( oemVideoGammaEnablePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video gamma correction enable
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_GAMMA_ENABLE,
                              ( LEP_ATTRIBUTE_T_PTR )oemVideoGammaEnablePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemVideoGammaEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_VIDEO_GAMMA_ENABLE_E oemVideoGammaEnable )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) oemVideoGammaEnable;

   /* Validate Parameter(s)
   */
   if( oemVideoGammaEnable >= LEP_END_VIDEO_GAMMA_ENABLE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output channel selection
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_VIDEO_GAMMA_ENABLE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemCalStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_OEM_STATUS_E_PTR calStatusPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if( calStatusPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_STATUS,
                              ( LEP_ATTRIBUTE_T_PTR )calStatusPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemFFCNormalizationTarget( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_OEM_FFC_NORMALIZATION_TARGET_T_PTR ffcTargetPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* FFC Target is a single 16-bit value */

   if( ffcTargetPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_FFC_NORMALIZATION_TARGET,
                              ( LEP_ATTRIBUTE_T_PTR )ffcTargetPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemFFCNormalizationTarget( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_OEM_FFC_NORMALIZATION_TARGET_T ffcTarget )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_FFC_NORMALIZATION_TARGET,
                              ( LEP_ATTRIBUTE_T_PTR ) & ffcTarget,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemFFCNormalization( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_OEM_FFC_NORMALIZATION_TARGET_T ffcTarget )
{
   LEP_RESULT result = LEP_OK;
   LEP_OEM_STATUS_E oemStatus = LEP_OEM_STATUS_BUSY;

   result = LEP_SetOemFFCNormalizationTarget( portDescPtr, ffcTarget );
   if( result == LEP_OK )
   {
      result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_FFC_NORMALIZATION_TARGET );
      while( oemStatus == LEP_OEM_STATUS_BUSY )
      {
         LEP_GetOemCalStatus( portDescPtr, &oemStatus );
      }
   }

   return( result );

}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_OEM_STATUS_E oemStatus = LEP_OEM_STATUS_BUSY;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_FFC_NORMALIZATION_TARGET );
   while( oemStatus == LEP_OEM_STATUS_BUSY )
   {
      LEP_GetOemCalStatus( portDescPtr, &oemStatus );
   }

   return( result );

}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemFrameMean( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_OEM_FRAME_AVERAGE_T_PTR frameAveragePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   if( frameAveragePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_SCENE_MEAN_VALUE,
                              ( LEP_ATTRIBUTE_T_PTR )frameAveragePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemPowerMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_OEM_POWER_STATE_E_PTR powerModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( powerModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_POWER_MODE,
                              ( LEP_ATTRIBUTE_T_PTR )powerModePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemPowerMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_OEM_POWER_STATE_E powerMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) powerMode;

   if( powerMode >= LEP_OEM_END_POWER_MODE )
   {
      return( LEP_RANGE_ERROR );
   }

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_POWER_MODE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemGpioMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_OEM_GPIO_MODE_E_PTR gpioModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( gpioModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_GPIO_MODE_SELECT,
                              ( LEP_ATTRIBUTE_T_PTR )gpioModePtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemGpioMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_OEM_GPIO_MODE_E gpioMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) gpioMode;

   if ( gpioMode >= LEP_OEM_END_GPIO_MODE )
   {
      return( LEP_RANGE_ERROR );
   }

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_GPIO_MODE_SELECT,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemGpioVsyncPhaseDelay( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_OEM_VSYNC_DELAY_E_PTR numHsyncLinesPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( numHsyncLinesPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_GPIO_VSYNC_PHASE_DELAY,
                              ( LEP_ATTRIBUTE_T_PTR )numHsyncLinesPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemGpioVsyncPhaseDelay( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_OEM_VSYNC_DELAY_E numHsyncLines )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_INT16) numHsyncLines;

   if( numHsyncLines >= LEP_END_OEM_VSYNC_DELAY )
   {
      return( LEP_RANGE_ERROR );
   }

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_GPIO_VSYNC_PHASE_DELAY,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemUserDefaultsState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_USER_PARAMS_STATE_E_PTR userParamsStatePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( userParamsStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_USER_DEFAULTS,
                              ( LEP_ATTRIBUTE_T_PTR )userParamsStatePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemUserDefaultsCopyToOtp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_USER_DEFAULTS );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunOemUserDefaultsRestore( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_OEM_USER_DEFAULTS_RESTORE );

   return( result );
}



LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemThermalShutdownEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T ThermalShutdownEnableState )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) ThermalShutdownEnableState.oemThermalShutdownEnable;

   if( ThermalShutdownEnableState.oemThermalShutdownEnable >= LEP_OEM_END_STATE )
   {
      return( LEP_RANGE_ERROR );
   }
   /* Perform Command
   ** Writing the Camera's current video freeze enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_THERMAL_SHUTDOWN_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemThermalShutdownEnable( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_THERMAL_SHUTDOWN_ENABLE_T_PTR ThermalShutdownEnableStatePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */

   if( ThermalShutdownEnableStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_THERMAL_SHUTDOWN_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )ThermalShutdownEnableStatePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemShutterProfileObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_SHUTTER_PROFILE_OBJ_T ShutterProfileObj )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* 2 words */

   _LEP_data[1] = ShutterProfileObj.openPeriodInFrames;
   _LEP_data[0] = ShutterProfileObj.closePeriodInFrames;

   /* Perform Command
   ** Writing the Camera's current video freeze enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_SHUTTER_PROFILE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemShutterProfileObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_OEM_SHUTTER_PROFILE_OBJ_T_PTR ShutterProfileObjPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* 2 words */

   if( ShutterProfileObjPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_SHUTTER_PROFILE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR )ShutterProfileObjPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemBadPixelReplaceControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T BadPixelReplaceControl )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two word enums */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) BadPixelReplaceControl.oemBadPixelReplaceEnable;

   /* Perform Command
   ** Writing the Camera's current video freeze enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_BAD_PIXEL_REPLACE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemBadPixelReplaceControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                             LEP_OEM_BAD_PIXEL_REPLACE_CONTROL_T_PTR BadPixelReplaceControlPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum */
   if( BadPixelReplaceControlPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_BAD_PIXEL_REPLACE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR )BadPixelReplaceControlPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemTemporalFilterControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_TEMPORAL_FILTER_CONTROL_T TemporalFilterControl )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) TemporalFilterControl.oemTemporalFilterEnable;;

   /* Perform Command
   ** Writing the Camera's current video freeze enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_TEMPORAL_FILTER_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemTemporalFilterControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_OEM_TEMPORAL_FILTER_CONTROL_T_PTR TemporalFilterControlPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */
   if( TemporalFilterControlPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_TEMPORAL_FILTER_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR )TemporalFilterControlPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemColumnNoiseEstimateControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T ColumnNoiseEstimateControl )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) ColumnNoiseEstimateControl.oemColumnNoiseEstimateEnable;;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_COLUMN_NOISE_ESTIMATE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemColumnNoiseEstimateControl( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                                 LEP_OEM_COLUMN_NOISE_ESTIMATE_CONTROL_T_PTR ColumnNoiseEstimateControlPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one enum = two words */
   if( ColumnNoiseEstimateControlPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_COLUMN_NOISE_ESTIMATE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR )ColumnNoiseEstimateControlPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetOemPixelNoiseSettings( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_OEM_PIXEL_NOISE_SETTINGS_T_PTR pixelNoiseSettingsPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* struct size 4 bytes */

   if( pixelNoiseSettingsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_PIXEL_NOISE_ESTIMATE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR )pixelNoiseSettingsPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetOemPixelNoiseSettings( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_OEM_PIXEL_NOISE_SETTINGS_T pixelNoiseSettings )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* struct size 4 bytes */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) pixelNoiseSettings.oemPixelNoiseEstimateEnable;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_OEM_PIXEL_NOISE_ESTIMATE_CONTROL,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


/**************************************/
/*                RAD                 */
/**************************************/
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTShutterMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_RAD_TS_MODE_E_PTR radTShutterModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( radTShutterModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current TShutter mode
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TSHUTTER_MODE,
                              ( LEP_ATTRIBUTE_T_PTR )radTShutterModePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTShutterMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_RAD_TS_MODE_E radTShutterMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) radTShutterMode;

   /* Validate Parameter(s)
   */
   if( radTShutterMode >= LEP_RAD_TS_END_TS_MODE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current TShutter mode
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TSHUTTER_MODE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTShutter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_KELVIN_T_PTR radTShutterPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radTShutterPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TSHUTTER,
                              ( LEP_ATTRIBUTE_T_PTR )radTShutterPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTShutter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_KELVIN_T radTShutter )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TSHUTTER,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTShutter,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_RunRadFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_RAD_STATUS_E radStatus = LEP_RAD_STATUS_BUSY;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_RAD_RUN_FFC );
   if( result == LEP_OK )
   {
      //TODO: Add timeout check
      while( radStatus == LEP_RAD_STATUS_BUSY )
      {
         LEP_GetRadRunStatus( portDescPtr, &radStatus );
      }
   }
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadRBFOInternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadRBFOInternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   _LEP_data[7] = (LEP_UINT16) radRBFOPtr->RBFO_O >> 16;
   _LEP_data[6] = (LEP_UINT16) radRBFOPtr->RBFO_O & 0xFFFF;
   _LEP_data[5] = (LEP_UINT16) radRBFOPtr->RBFO_F >> 16;
   _LEP_data[4] = (LEP_UINT16) radRBFOPtr->RBFO_F & 0xFFFF;
   _LEP_data[3] = (LEP_UINT16) radRBFOPtr->RBFO_B >> 16;
   _LEP_data[2] = (LEP_UINT16) radRBFOPtr->RBFO_B & 0xFFFF;
   _LEP_data[1] = (LEP_UINT16) radRBFOPtr->RBFO_R >> 16;
   _LEP_data[0] = (LEP_UINT16) radRBFOPtr->RBFO_R & 0xFFFF;

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadRBFOExternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadRBFOExternal0( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 4 16-bit words

   _LEP_data[7] = (LEP_UINT16) radRBFOPtr->RBFO_O >> 16;
   _LEP_data[6] = (LEP_UINT16) radRBFOPtr->RBFO_O & 0xFFFF;
   _LEP_data[5] = (LEP_UINT16) radRBFOPtr->RBFO_F >> 16;
   _LEP_data[4] = (LEP_UINT16) radRBFOPtr->RBFO_F & 0xFFFF;
   _LEP_data[3] = (LEP_UINT16) radRBFOPtr->RBFO_B >> 16;
   _LEP_data[2] = (LEP_UINT16) radRBFOPtr->RBFO_B & 0xFFFF;
   _LEP_data[1] = (LEP_UINT16) radRBFOPtr->RBFO_R >> 16;
   _LEP_data[0] = (LEP_UINT16) radRBFOPtr->RBFO_R & 0xFFFF;

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadInternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadInternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   _LEP_data[7] = (LEP_UINT16) radRBFOPtr->RBFO_O >> 16;
   _LEP_data[6] = (LEP_UINT16) radRBFOPtr->RBFO_O & 0xFFFF;
   _LEP_data[5] = (LEP_UINT16) radRBFOPtr->RBFO_F >> 16;
   _LEP_data[4] = (LEP_UINT16) radRBFOPtr->RBFO_F & 0xFFFF;
   _LEP_data[3] = (LEP_UINT16) radRBFOPtr->RBFO_B >> 16;
   _LEP_data[2] = (LEP_UINT16) radRBFOPtr->RBFO_B & 0xFFFF;
   _LEP_data[1] = (LEP_UINT16) radRBFOPtr->RBFO_R >> 16;
   _LEP_data[0] = (LEP_UINT16) radRBFOPtr->RBFO_R & 0xFFFF;

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadExternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadExternalRBFOHighGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   _LEP_data[7] = (LEP_UINT16) radRBFOPtr->RBFO_O >> 16;
   _LEP_data[6] = (LEP_UINT16) radRBFOPtr->RBFO_O & 0xFFFF;
   _LEP_data[5] = (LEP_UINT16) radRBFOPtr->RBFO_F >> 16;
   _LEP_data[4] = (LEP_UINT16) radRBFOPtr->RBFO_F & 0xFFFF;
   _LEP_data[3] = (LEP_UINT16) radRBFOPtr->RBFO_B >> 16;
   _LEP_data[2] = (LEP_UINT16) radRBFOPtr->RBFO_B & 0xFFFF;
   _LEP_data[1] = (LEP_UINT16) radRBFOPtr->RBFO_R >> 16;
   _LEP_data[0] = (LEP_UINT16) radRBFOPtr->RBFO_R & 0xFFFF;

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadInternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL_LG,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadInternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   _LEP_data[7] = (LEP_UINT16) radRBFOPtr->RBFO_O >> 16;
   _LEP_data[6] = (LEP_UINT16) radRBFOPtr->RBFO_O & 0xFFFF;
   _LEP_data[5] = (LEP_UINT16) radRBFOPtr->RBFO_F >> 16;
   _LEP_data[4] = (LEP_UINT16) radRBFOPtr->RBFO_F & 0xFFFF;
   _LEP_data[3] = (LEP_UINT16) radRBFOPtr->RBFO_B >> 16;
   _LEP_data[2] = (LEP_UINT16) radRBFOPtr->RBFO_B & 0xFFFF;
   _LEP_data[1] = (LEP_UINT16) radRBFOPtr->RBFO_R >> 16;
   _LEP_data[0] = (LEP_UINT16) radRBFOPtr->RBFO_R & 0xFFFF;

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_INTERNAL_LG,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadExternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 8 16-bit words

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL_LG,
                              ( LEP_ATTRIBUTE_T_PTR )radRBFOPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadExternalRBFOLowGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RBFO_T_PTR radRBFOPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;    // RBFO is 4 16-bit words

   _LEP_data[7] = (LEP_UINT16) radRBFOPtr->RBFO_O >> 16;
   _LEP_data[6] = (LEP_UINT16) radRBFOPtr->RBFO_O & 0xFFFF;
   _LEP_data[5] = (LEP_UINT16) radRBFOPtr->RBFO_F >> 16;
   _LEP_data[4] = (LEP_UINT16) radRBFOPtr->RBFO_F & 0xFFFF;
   _LEP_data[3] = (LEP_UINT16) radRBFOPtr->RBFO_B >> 16;
   _LEP_data[2] = (LEP_UINT16) radRBFOPtr->RBFO_B & 0xFFFF;
   _LEP_data[1] = (LEP_UINT16) radRBFOPtr->RBFO_R >> 16;
   _LEP_data[0] = (LEP_UINT16) radRBFOPtr->RBFO_R & 0xFFFF;

   if( radRBFOPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RBFO_EXTERNAL_LG,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadResponsivityShift( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_RS_T_PTR radResponsivityShiftPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radResponsivityShiftPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RESPONSIVITY_SHIFT,
                              ( LEP_ATTRIBUTE_T_PTR )radResponsivityShiftPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadResponsivityShift( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_RS_T radResponsivityShift )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RESPONSIVITY_SHIFT,
                              ( LEP_ATTRIBUTE_T_PTR ) & radResponsivityShift,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadFNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_FNUMBER_T_PTR radFNumberPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radFNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_F_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )radFNumberPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadFNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_FNUMBER_T radFNumber )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_F_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR ) & radFNumber,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTauLens( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TAULENS_T_PTR radTauLensPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radTauLensPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAU_LENS,
                              ( LEP_ATTRIBUTE_T_PTR )radTauLensPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTauLens( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TAULENS_T radTauLens )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAU_LENS,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTauLens,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadRadometryFilter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_RADIOMETRY_FILTER_T_PTR radRadiometryFilterPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radRadiometryFilterPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RADIOMETRY_FILTER,
                              ( LEP_ATTRIBUTE_T_PTR )radRadiometryFilterPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadRadometryFilter( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_RADIOMETRY_FILTER_T radRadiometryFilter )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RADIOMETRY_FILTER,
                              ( LEP_ATTRIBUTE_T_PTR ) & radRadiometryFilter,
                              attributeWordLength );

   return( result );
}

/* Deprecated: Use LEP_GetRadTFpaLut */
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTFpaCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_LUT256_T_PTR radTFpaCLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTFpaCLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaCLutPtr,
                              attributeWordLength );

   return( result );
}

/* Deprecated: Use LEP_SetRadTFpaLut */
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTFpaCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_LUT256_T_PTR radTFpaCLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTFpaCLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaCLutPtr,
                              attributeWordLength );

   return( result );
}

/* Deprecated: Use LEP_GetRadTAuxLut */
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTAuxCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_LUT256_T_PTR radTAuxCLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTAuxCLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxCLutPtr,
                              attributeWordLength );

   return( result );
}

/* Deprecated: Use LEP_SetRadTAuxLut */
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTAuxCLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_LUT256_T_PTR radTAuxCLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTAuxCLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxCLutPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTFpaLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LUT256_T_PTR radTFpaLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTFpaLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTFpaLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LUT256_T_PTR radTFpaLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTFpaLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTAuxLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LUT256_T_PTR radTAuxLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTAuxLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTAuxLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LUT256_T_PTR radTAuxLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 256;    // 256 16-bit word LUT

   if( radTAuxLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadResponsivityValueLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_LUT128_T_PTR radResponsivityValueLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 128 16-bit word LUT

   if( radResponsivityValueLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RESPONSIVITY_VALUE_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radResponsivityValueLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadResponsivityValueLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_LUT128_T_PTR radResponsivityValueLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 128 16-bit word LUT

   if( radResponsivityValueLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RESPONSIVITY_VALUE_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radResponsivityValueLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadDebugTemp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_RAD_KELVIN_T_PTR radDebugTempPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radDebugTempPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_DEBUG_TEMP,
                              ( LEP_ATTRIBUTE_T_PTR )radDebugTempPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadDebugTemp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_RAD_KELVIN_T radDebugTemp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_DEBUG_TEMP,
                              ( LEP_ATTRIBUTE_T_PTR ) & radDebugTemp,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadDebugFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_RAD_FLUX_T_PTR radDebugFluxPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words

   if( radDebugFluxPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_DEBUG_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR )radDebugFluxPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadDebugFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_RAD_FLUX_T radDebugFlux )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words

   _LEP_data[1] = radDebugFlux >> 16;
   _LEP_data[0] = radDebugFlux & 0xFFFF;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_DEBUG_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_ENABLE_E_PTR radEnableStatePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   if( radEnableStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )radEnableStatePtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_ENABLE_E radEnableState )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) radEnableState;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_GLOBAL_GAIN_T_PTR radGlobalGainPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radGlobalGainPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_GLOBAL_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR )radGlobalGainPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_GLOBAL_GAIN_T radGlobalGain )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_GLOBAL_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR ) & radGlobalGain,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_RAD_GLOBAL_OFFSET_T_PTR radGlobalOffsetPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radGlobalOffsetPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_GLOBAL_OFFSET,
                              ( LEP_ATTRIBUTE_T_PTR )radGlobalOffsetPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_RAD_GLOBAL_OFFSET_T radGlobalOffset )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* single 16-bit value */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_GLOBAL_OFFSET,
                              ( LEP_ATTRIBUTE_T_PTR ) & radGlobalOffset,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTFpaCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_TEMPERATURE_UPDATE_E_PTR radTFpaCtsModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   if( radTFpaCtsModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_CTS_MODE,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaCtsModePtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTFpaCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_TEMPERATURE_UPDATE_E radTFpaCtsMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) radTFpaCtsMode;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_CTS_MODE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTAuxCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_TEMPERATURE_UPDATE_E_PTR radTAuxCtsModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   if( radTAuxCtsModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_CTS_MODE,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxCtsModePtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTAuxCtsMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_TEMPERATURE_UPDATE_E radTAuxCtsMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // 2 16-bit words for an enum

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) radTAuxCtsMode;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_CTS_MODE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTFpaCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TEMPERATURE_COUNTS_T_PTR radTFpaCtsPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radTFpaCtsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_CTS,
                              ( LEP_ATTRIBUTE_T_PTR )radTFpaCtsPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTFpaCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TEMPERATURE_COUNTS_T radTFpaCts )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TFPA_CTS,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTFpaCts,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTAuxCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TEMPERATURE_COUNTS_T_PTR radTAuxCtsPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   if( radTAuxCtsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_CTS,
                              ( LEP_ATTRIBUTE_T_PTR )radTAuxCtsPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTAuxCts( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_TEMPERATURE_COUNTS_T radTAuxCts )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    // single 16-bit word

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TAUX_CTS,
                              ( LEP_ATTRIBUTE_T_PTR ) & radTAuxCts,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTEqShutterLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RAD_LUT128_T_PTR radTEqShutterLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 128 16-bit word LUT

   if( radTEqShutterLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TEQ_SHUTTER_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTEqShutterLutPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTEqShutterLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_RAD_LUT128_T_PTR radTEqShutterLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 128 16-bit word LUT

   if( radTEqShutterLutPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TEQ_SHUTTER_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radTEqShutterLutPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadRunStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_RAD_STATUS_E_PTR radStatusPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   if( radStatusPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_RUN_STATUS,
                              ( LEP_ATTRIBUTE_T_PTR )radStatusPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTEqShutterFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_FLUX_T_PTR radTEqShutterFluxPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // single 32-bit word

   if( radTEqShutterFluxPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TEQ_SHUTTER_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR )radTEqShutterFluxPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTEqShutterFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_FLUX_T radTEqShutterFlux )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // single 32-bit word

   _LEP_data[1] = radTEqShutterFlux >> 16;
   _LEP_data[0] = radTEqShutterFlux & 0xFFFF;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TEQ_SHUTTER_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadMffcFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_FLUX_T_PTR radRadMffcFluxPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // single 32-bit word

   if( radRadMffcFluxPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_MFFC_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR )radRadMffcFluxPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadMffcFlux( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_RAD_FLUX_T radRadMffcFlux )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    // single 32-bit word

   _LEP_data[1] = radRadMffcFlux >> 16;
   _LEP_data[0] = radRadMffcFlux & 0xFFFF;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_MFFC_FLUX,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadFrameMedianPixelValue( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_MEDIAN_VALUE_T_PTR frameMedianPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit word */

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_FRAME_MEDIAN_VALUE,
                              ( LEP_ATTRIBUTE_T_PTR )frameMedianPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadMLGLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_RAD_SIGNED_LUT128_T_PTR radMLGLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 256 16-bit word LUT

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_MLG_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radMLGLutPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadMLGLut( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_RAD_SIGNED_LUT128_T_PTR radMLGLutPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 128;    // 256 16-bit word LUT

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_MLG_LUT,
                              ( LEP_ATTRIBUTE_T_PTR )radMLGLutPtr,
                              attributeWordLength );

   return( result );
}
#if USE_DEPRECATED_HOUSING_TCP_INTERFACE
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR radHousingTcp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    // LEP_RAD_LINEAR_TEMP_CORRECTION_T is 4 16-bit words

   if( radHousingTcp == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_THOUSING_TCP,
                              ( LEP_ATTRIBUTE_T_PTR )radHousingTcp,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_LINEAR_TEMP_CORRECTION_T radHousingTcp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* single 16-bit value */

   _LEP_data[3] = radHousingTcp.offset;
   _LEP_data[2] = radHousingTcp.gainAux;
   _LEP_data[1] = radHousingTcp.gainShutter;
   _LEP_data[0] = radHousingTcp.pad;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_THOUSING_TCP,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}
#else
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR RadHousingTcpPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    // LEP_RAD_LINEAR_TEMP_CORRECTION_T is 4 16-bit words

   if( RadHousingTcpPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_HOUSING_TCP,
                              ( LEP_ATTRIBUTE_T_PTR )RadHousingTcpPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadHousingTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_LINEAR_TEMP_CORRECTION_T RadHousingTcp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* single 16-bit value */

   _LEP_data[3] = RadHousingTcp.offset;
   _LEP_data[2] = RadHousingTcp.gainAux;
   _LEP_data[1] = RadHousingTcp.gainShutter;
   _LEP_data[0] = RadHousingTcp.pad;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_HOUSING_TCP,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}
#endif




LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadShutterTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR RadShutterTcpPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    // LEP_RAD_LINEAR_TEMP_CORRECTION_T is 4 16-bit words

   if( RadShutterTcpPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_SHUTTER_TCP,
                              ( LEP_ATTRIBUTE_T_PTR )RadShutterTcpPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadShutterTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_LINEAR_TEMP_CORRECTION_T RadShutterTcp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* single 16-bit value */

   _LEP_data[3] = RadShutterTcp.offset;
   _LEP_data[2] = RadShutterTcp.gainAux;
   _LEP_data[1] = RadShutterTcp.gainShutter;
   _LEP_data[0] = RadShutterTcp.pad;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_SHUTTER_TCP,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}




LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadLensTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LINEAR_TEMP_CORRECTION_T_PTR RadLensTcpPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    // LEP_RAD_LINEAR_TEMP_CORRECTION_T is 4 16-bit words

   if( RadLensTcpPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }
   result = LEP_GetAttribute(  portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_LENS_TCP,
                              ( LEP_ATTRIBUTE_T_PTR )RadLensTcpPtr,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadLensTcp( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_RAD_LINEAR_TEMP_CORRECTION_T RadLensTcp )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* 4 16-bit value */

   _LEP_data[3] = RadLensTcp.offset;
   _LEP_data[2] = RadLensTcp.gainAux;
   _LEP_data[1] = RadLensTcp.gainShutter;
   _LEP_data[0] = RadLensTcp.pad;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_LENS_TCP,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadPreviousGlobalOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_GLOBAL_OFFSET_T_PTR globalOffsetPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_PREVIOUS_GLOBAL_OFFSET,
                              ( LEP_ATTRIBUTE_T_PTR )globalOffsetPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadPreviousGlobalGain( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                         LEP_RAD_GLOBAL_GAIN_T_PTR globalGainPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_PREVIOUS_GLOBAL_GAIN,
                              ( LEP_ATTRIBUTE_T_PTR )globalGainPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetGlobalGainFFC( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_RAD_GLOBAL_GAIN_T_PTR globalGainFfcPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_GLOBAL_GAIN_FFC,
                              ( LEP_ATTRIBUTE_T_PTR )globalGainFfcPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadCnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   if( scaleFactorPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_CNF_SCALE_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )scaleFactorPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   if( scaleFactorPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_TNF_SCALE_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )scaleFactorPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadSnfScaleFactor( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                     LEP_RAD_PARAMETER_SCALE_FACTOR_T_PTR scaleFactorPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   if( scaleFactorPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_SNF_SCALE_FACTOR,
                              ( LEP_ATTRIBUTE_T_PTR )scaleFactorPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadArbitraryOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_ARBITRARY_OFFSET_T_PTR arbitraryOffsetPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   if( arbitraryOffsetPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_ARBITRARY_OFFSET,
                              ( LEP_ATTRIBUTE_T_PTR )arbitraryOffsetPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadArbitraryOffset( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_ARBITRARY_OFFSET_T arbitraryOffset )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_RAD_ARBITRARY_OFFSET,
                              ( LEP_ATTRIBUTE_T_PTR ) & arbitraryOffset,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadFluxLinearParams(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_FLUX_LINEAR_PARAMS_T_PTR fluxParamsPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;

   if(fluxParamsPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_FLUX_LINEAR_PARAMS,
                             (LEP_ATTRIBUTE_T_PTR)fluxParamsPtr,
                             attributeWordLength);

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadFluxLinearParams(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_RAD_FLUX_LINEAR_PARAMS_T fluxParams)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 8;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_FLUX_LINEAR_PARAMS,
                             (LEP_ATTRIBUTE_T_PTR)&fluxParams,
                             attributeWordLength);

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTLinearEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_ENABLE_E_PTR enableStatePtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if(enableStatePtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) enableStatePtr,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTLinearEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_RAD_ENABLE_E enableState)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) enableState;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTLinearResolution(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_TLINEAR_RESOLUTION_E_PTR resolutionPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if(resolutionPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_RESOLUTION,
                             (LEP_ATTRIBUTE_T_PTR)resolutionPtr,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTLinearResolution(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_RAD_TLINEAR_RESOLUTION_E resolution)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) resolution;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_RESOLUTION,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadTLinearAutoResolution(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_ENABLE_E_PTR enableStatePtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if(enableStatePtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_AUTO_RESOLUTION,
                             (LEP_ATTRIBUTE_T_PTR)enableStatePtr,
                             attributeWordLength);
   return(result);
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadTLinearAutoResolution(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_RAD_ENABLE_E enableState)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) enableState;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_TLINEAR_AUTO_RESOLUTION,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadSpotmeterRoi(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_ROI_T_PTR spotmeterRoiPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;

   if(spotmeterRoiPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_SPOTMETER_ROI,
                             (LEP_ATTRIBUTE_T_PTR)spotmeterRoiPtr,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadSpotmeterRoi(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_RAD_ROI_T spotmeterRoi)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;

   _LEP_data[3] = spotmeterRoi.endRow;
   _LEP_data[2] = spotmeterRoi.endCol;
   _LEP_data[1] = spotmeterRoi.startRow;
   _LEP_data[0] = spotmeterRoi.startCol;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_SPOTMETER_ROI,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadSpotmeterObjInKelvinX100(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_RAD_SPOTMETER_OBJ_KELVIN_T_PTR kelvinPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;

   if(kelvinPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_SPOTMETER_OBJ_KELVIN,
                             (LEP_ATTRIBUTE_T_PTR)kelvinPtr,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadArbitraryOffsetMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_ARBITRARY_OFFSET_MODE_E_PTR arbitraryOffsetModePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if(arbitraryOffsetModePtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_ARBITRARY_OFFSET_MODE,
                             (LEP_ATTRIBUTE_T_PTR)arbitraryOffsetModePtr,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadArbitraryOffsetMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_RAD_ARBITRARY_OFFSET_MODE_E arbitraryOffsetMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_ARBITRARY_OFFSET_MODE,
                             (LEP_ATTRIBUTE_T_PTR)&arbitraryOffsetMode,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadArbitraryOffsetParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_ARBITRARY_OFFSET_PARAMS_T_PTR arbitraryOffsetParamsPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if(arbitraryOffsetParamsPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_ARBITRARY_OFFSET_PARAMS,
                             (LEP_ATTRIBUTE_T_PTR)arbitraryOffsetParamsPtr,
                             attributeWordLength);
   return(result);

}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadArbitraryOffsetParams( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_ARBITRARY_OFFSET_PARAMS_T arbitraryOffsetParams)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   result = LEP_SetAttribute(portDescPtr,
                             (LEP_COMMAND_ID)LEP_CID_RAD_ARBITRARY_OFFSET_PARAMS,
                             (LEP_ATTRIBUTE_T_PTR)&arbitraryOffsetParams,
                             attributeWordLength);
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetRadRadioCalValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_RADIO_CAL_VALUES_T_PTR radRadioCalValuesPtr)
{
    LEP_RESULT result = LEP_OK;
    LEP_UINT16 attributeWordLength = 4;

    if(radRadioCalValuesPtr == NULL)
    {
       return(LEP_BAD_ARG_POINTER_ERROR);
    }

    result = LEP_GetAttribute(portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_RAD_RADIO_CAL_VALUES,
                              (LEP_ATTRIBUTE_T_PTR)radRadioCalValuesPtr,
                              attributeWordLength);
    return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetRadRadioCalValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_RAD_RADIO_CAL_VALUES_T radRadioCalValues )
{
    LEP_RESULT result = LEP_OK;
    LEP_UINT16 attributeWordLength = 4;

    result = LEP_SetAttribute(portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_RAD_RADIO_CAL_VALUES,
                              (LEP_ATTRIBUTE_T_PTR)&radRadioCalValues,
                              attributeWordLength);
    return(result);
}


/**************************************/
/*                SDK                 */
/**************************************/
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                            LEP_COMMAND_ID commandID, 
                            LEP_ATTRIBUTE_T_PTR attributePtr,
                            LEP_UINT16 attributeWordLength)
{
    LEP_RESULT  result = LEP_OK;

    /* Validate the port descriptor
    */ 
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    /* Validate input pointer
    */
    if( attributePtr == NULL )
    {
        return(LEP_BAD_ARG_POINTER_ERROR);
    }

    /* Modify the passed-in command ID to add the Get type
    */
    commandID |= LEP_GET_TYPE;

    /* Perform Command using the active Port
    */
    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        /* Use the Lepton TWI/CCI Port
        */ 
        result = LEP_I2C_GetAttribute( portDescPtr, 
                                       commandID,
                                       attributePtr,
                                       attributeWordLength );
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {

        /* Use the Lepton SPI Port
        */ 

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    return(result);
}

/**
 * Sets the value of a camera attribute.
 * 
 * @return 
 */
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetAttribute(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                            LEP_COMMAND_ID commandID, 
                            LEP_ATTRIBUTE_T_PTR attributePtr,
                            LEP_UINT16 attributeWordLength)
{
    LEP_RESULT result = LEP_OK;

    /* Validate the port descriptor
    */ 
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    /* Modify the passed-in command ID to add the Get type
    */
    commandID |= LEP_SET_TYPE;

    /* Issue Command to the Active Port
    */
    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        /* Use the Lepton TWI/CCI Port
        */ 
        result = LEP_I2C_SetAttribute( portDescPtr, 
                                       commandID,
                                       attributePtr,
                                       attributeWordLength );
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {
        /* Use the Lepton SPI Port
        */ 

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunCommand(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                          LEP_COMMAND_ID commandID)
{
    LEP_RESULT  result = LEP_OK;

    /* Validate the port descriptor
    */ 
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    /* Modify the passed-in command ID to add the Run type
    */
    commandID |= LEP_RUN_TYPE;

    /* Perform Command
    */
    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        /* Use the Lepton TWI/CCI Port
        */ 
        result = LEP_I2C_RunCommand( portDescPtr, 
                                     commandID);
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {
        /* Use the Lepton SPI Port
        */ 

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SelectDevice(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr, 
                            LEP_PROTOCOL_DEVICE_E device)
{
    LEP_RESULT result = LEP_OK;

    /* Validate the port descriptor
    */ 
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    /* Select Device
    */
    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        result = LEP_I2C_SelectDevice(portDescPtr, device);
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    return(result);
}

/******************************************************************************/
/**
 * Opens a Lepton commnications port of the specified type and
 * is assigned the passed ID.
 * This function dynamically allocates a new descriptor from the
 * system heap.
 * 
 * @param portID      LEP_UINT 16  User defined port ID.  This 
 *                    value is not used by the Lepton SDK but
 *                    provides the ability for
 *                    application-specific use.
 * 
 * @param portType    LEP_CAMERA_PORT_E  Specifies the Lepton 
 *                    Communications Port type.
 * 
 * @param pDescriptor LEP_CAMERA_PORT_DESC_T_PTR  Lepton Port
 *                    descriptor. This is a handle to a valid
 *                    Lepton port is fusseccful, NULL otherwise.
 * 
 * @return LEP_RESULT  Lepton Error Code.  LEP_OK if all goes well,
 *         otherise and Lepton error code is retunred.
 */
LEP_RESULT LeptonSDKEmb32OEM::LEP_OpenPort(LEP_UINT16 portID,
                        LEP_CAMERA_PORT_E portType,
                        LEP_UINT16   portBaudRate,
                        LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result = LEP_COMM_INVALID_PORT_ERROR;
    LEP_UINT8 deviceAddress;

    /* Attempt to acquire memory
    **   Dynamic creation using malloc() or static allocation
    **   Our reference will us dynamic creation
    */ 
#ifdef LEP_USE_DYNAMIC_ALLOCATION
    /* Allocate from the heap
    */ 
    portDescPtr = (LEP_CAMERA_PORT_DESC_T_PTR)malloc( sizeof(LEP_CAMERA_PORT_DESC_T));
#else    
    /* Allocate from static memory
    */ 
#endif        

    /* Validate the port descriptor
    */ 
    if( portDescPtr != NULL )
    {
        /* Open Port driver
        */
        switch( portType )
        {
            case LEP_CCI_TWI:
                result = LEP_I2C_OpenPort(portID, &portBaudRate, &deviceAddress);
                if( result == LEP_OK )
                {
                    portDescPtr->portBaudRate = portBaudRate;
                    portDescPtr->portID = portID;
                    portDescPtr->portType = portType;
                    portDescPtr->deviceAddress = deviceAddress;
                }
                
#ifdef LEP_USE_DYNAMIC_ALLOCATION
				else
					free( portDescPtr);
#endif
                    

                break;

            case LEP_CCI_SPI:
                break;

            default:
                break;
        }
    }
    else
        result = LEP_ERROR_CREATING_COMM;


    return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_ClosePort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result = LEP_OK;
	
    /* Validate the port descriptor
    */ 
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    /* Close Port driver
    */
    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        result = LEP_I2C_ClosePort(portDescPtr);
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    

#ifdef LEP_USE_DYNAMIC_ALLOCATION
    free( portDescPtr );
#endif        

    return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_ResetPort(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr)
{
    LEP_RESULT result;

    /* Validate the port descriptor
    */ 
    if( portDescPtr == NULL )
    {
        return(LEP_COMM_PORT_NOT_OPEN);
    }

    if( portDescPtr->portType == LEP_CCI_TWI )
    {
        LEP_I2C_ResetPort( portDescPtr );
    }
    else if( portDescPtr->portType == LEP_CCI_SPI )
    {

    }
    else
        result = LEP_COMM_INVALID_PORT_ERROR;

    return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetPortStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr, 
                             LEP_UINT16 *status)
{
    LEP_RESULT result = LEP_OK;


    return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_DirectReadRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_UINT16 registerAddress,
                                  LEP_UINT16* regValue)
{
   LEP_RESULT result = LEP_OK;


   if( portDescPtr->portType == LEP_CCI_TWI )
   {
     /* Use the Lepton TWI/CCI Port
     */ 
      result = LEP_I2C_DirectReadRegister(portDescPtr, registerAddress, regValue);
   }
   else if( portDescPtr->portType == LEP_CCI_SPI )
   {
     /* Use the Lepton SPI Port
     */ 

   }
   else
     result = LEP_COMM_INVALID_PORT_ERROR;

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetDeviceAddress(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_UINT8* deviceAddress)
{
   LEP_RESULT result = LEP_OK;

   if(portDescPtr->portType == LEP_CCI_TWI)
   {
      result = LEP_I2C_GetDeviceAddress(portDescPtr, deviceAddress);
   }

   return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_DirectWriteRegister(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_UINT16 registerAddress,
                                   LEP_UINT16 regValue)
{
   LEP_RESULT result = LEP_OK;
   /* Validate the port descriptor
   */ 
   if( portDescPtr == NULL )
   {
     return(LEP_COMM_PORT_NOT_OPEN);
   }

   /* Issue Command to the Active Port
   */
   if( portDescPtr->portType == LEP_CCI_TWI )
   {
     /* Use the Lepton TWI/CCI Port
     */ 
      result = LEP_I2C_DirectWriteRegister(portDescPtr, registerAddress, regValue);
     
   }
   else if( portDescPtr->portType == LEP_CCI_SPI )
   {
     /* Use the Lepton SPI Port
     */ 

   }
   else
     result = LEP_COMM_INVALID_PORT_ERROR;
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_DirectWriteBuffer(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_ATTRIBUTE_T_PTR attributePtr,
                                 LEP_UINT16 attributeWordLength)
{
   LEP_RESULT result = LEP_OK;

   /* Validate the port descriptor
   */ 
   if( portDescPtr == NULL )
   {
     return(LEP_COMM_PORT_NOT_OPEN);
   }

   /* Issue Command to the Active Port
   */
   if( portDescPtr->portType == LEP_CCI_TWI )
   {
     /* Use the Lepton TWI/CCI Port
     */ 
     result = LEP_I2C_DirectWriteBuffer(portDescPtr,
                                        attributePtr,
                                        attributeWordLength );
   }
   else if( portDescPtr->portType == LEP_CCI_SPI )
   {
     /* Use the Lepton SPI Port
     */ 

   }
   else
     result = LEP_COMM_INVALID_PORT_ERROR;
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSDKVersion(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_SDK_VERSION_T_PTR sdkVersionPtr)
{
   LEP_RESULT result = LEP_OK;

   if(sdkVersionPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }
   sdkVersionPtr->major = LEP_SDK_VERSION_MAJOR;
   sdkVersionPtr->minor = LEP_SDK_VERSION_MINOR;
   sdkVersionPtr->build = LEP_SDK_VERSION_BUILD;

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetCameraBootStatus(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_SDK_BOOT_STATUS_E_PTR bootStatusPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 regValue;

   result = LEP_DirectReadRegister(portDescPtr, 0x2, &regValue);

   if(result == LEP_OK && regValue & 4)
   {
      *bootStatusPtr = LEP_BOOT_STATUS_BOOTED;
   }
   else
   {
      *bootStatusPtr = LEP_BOOT_STATUS_NOT_BOOTED;
   }

   return(result);
}

/******************************************************************************/
/** PRIVATE MODULE FUNCTIONS                                                 **/
/******************************************************************************/

LEP_RESULT LeptonSDKEmb32OEM::_LEP_DelayCounts(LEP_UINT32 counts)
{
    LEP_UINT32 a = 0;
    while( counts-- )
    {
        a=counts;        
    }
    if( a )
    {
        return(LEP_TIMEOUT_ERROR) ;
    }
    return(LEP_OK);
}


/**************************************/
/*                SYS                 */
/**************************************/
LEP_RESULT LeptonSDKEmb32OEM::LEP_RunSysPing( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result = LEP_OK;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_SYS_PING );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_STATUS_T_PTR sysStatusPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* 4 16-bit values */

   /* Validate Parameter(s)
   */
   if( sysStatusPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Status
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_CAM_STATUS,
                              ( LEP_ATTRIBUTE_T_PTR )sysStatusPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysFlirSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_SYS_FLIR_SERIAL_NUMBER_T_PTR sysSerialNumberBufPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* 8 bytes values */

   /* Validate Parameter(s)
   */
   if( sysSerialNumberBufPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Serial Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FLIR_SERIAL_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )sysSerialNumberBufPtr,
                              attributeWordLength );
   return( result );
}
#if USE_DEPRECATED_SERIAL_NUMBER_INTERFACE
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysCustSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_SYS_CUST_SERIAL_NUMBER_T_PTR sysSerialNumberPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;   /* 32 byte string */

   if( sysSerialNumberPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_CUST_SERIAL_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )sysSerialNumberPtr,
                              attributeWordLength );

   return( result );
}
#else

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysCustSerialNumber( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_SYS_CUST_SERIAL_NUMBER_T_PTR sysCustSNPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;      /*32 byte string */

   if( sysCustSNPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_CUST_SERIAL_NUMBER,
                              ( LEP_ATTRIBUTE_T_PTR )sysCustSNPtr,
                              attributeWordLength );

   return( result );
}
#endif
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysCameraUpTime( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                   LEP_SYS_UPTIME_NUMBER_T_PTR sysCameraUpTimePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values */

   /* Validate Parameter(s)
   */
   if( sysCameraUpTimePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Serial Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_CAM_UPTIME,
                              ( LEP_ATTRIBUTE_T_PTR )sysCameraUpTimePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysAuxTemperatureCelcius( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_SYS_AUX_TEMPERATURE_CELCIUS_T_PTR auxTemperaturePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_SYS_AUX_TEMPERATURE_KELVIN_T unitsKelvin;

   /* Validate Parameter(s)
   */
   if( auxTemperaturePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetSysAuxTemperatureKelvin( portDescPtr, &unitsKelvin );
   *auxTemperaturePtr = ( LEP_SYS_AUX_TEMPERATURE_CELCIUS_T )( ( ( unitsKelvin / 100 ) + ( ( unitsKelvin % 100 ) * .01 ) ) - 273.15 );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysFpaTemperatureCelcius( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                            LEP_SYS_FPA_TEMPERATURE_CELCIUS_T_PTR fpaTemperaturePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_SYS_FPA_TEMPERATURE_KELVIN_T unitsKelvin;

   /* Validate Parameter(s)
   */
   if( fpaTemperaturePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetSysFpaTemperatureKelvin( portDescPtr, &unitsKelvin );
   *fpaTemperaturePtr = ( LEP_SYS_FPA_TEMPERATURE_CELCIUS_T )( ( ( unitsKelvin / 100 ) + ( ( unitsKelvin % 100 ) * .01 ) ) - 273.15 );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysAuxTemperatureKelvin( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_AUX_TEMPERATURE_KELVIN_T_PTR auxTemperaturePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* one 16-bit values */

   /* Validate Parameter(s)
   */
   if( auxTemperaturePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Serial Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_AUX_TEMPERATURE_KELVIN,
                              ( LEP_ATTRIBUTE_T_PTR )auxTemperaturePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysFpaTemperatureKelvin( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_FPA_TEMPERATURE_KELVIN_T_PTR fpaTemperaturePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1; /* one 16-bit values */

   /* Validate Parameter(s)
   */
   if( fpaTemperaturePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's Serial Number
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FPA_TEMPERATURE_KELVIN,
                              ( LEP_ATTRIBUTE_T_PTR )fpaTemperaturePtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysTelemetryEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_TELEMETRY_ENABLE_STATE_E_PTR enableStatePtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bits */

   if( enableStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** 
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_TELEMETRY_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )enableStatePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetSysTelemetryEnableState( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_TELEMETRY_ENABLE_STATE_E enableState )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bits */

   if( enableState >= LEP_END_TELEMETRY_ENABLE_STATE )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** 
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_TELEMETRY_ENABLE_STATE,
                              ( LEP_ATTRIBUTE_T_PTR ) & enableState,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysTelemetryLocation( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_SYS_TELEMETRY_LOCATION_E_PTR telemetryLocationPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bits */

   if( telemetryLocationPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** 
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_TELEMETRY_LOCATION,
                              ( LEP_ATTRIBUTE_T_PTR )telemetryLocationPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetSysTelemetryLocation( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_SYS_TELEMETRY_LOCATION_E telemetryLocation )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bits */

   if( telemetryLocation >= LEP_END_TELEMETRY_LOCATION )
   {
      return( LEP_RANGE_ERROR );
   }
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_TELEMETRY_LOCATION,
                              ( LEP_ATTRIBUTE_T_PTR ) & telemetryLocation,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunFrameAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT  result;

   /* Run the frame averaging command
   */
   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )LEP_CID_SYS_EXECTUE_FRAME_AVERAGE );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunSysAverageFrames( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_SYS_FRAME_AVERAGE_DIVISOR_E numFrameToAverage )
{
   LEP_RESULT  result = LEP_OK;

   /* Set the requested frames to average
   */
   result = LEP_SetSysFramesToAverage( portDescPtr, numFrameToAverage );

   if( result == LEP_OK )
   {
      /* Run the frame averaging command
      */
      result = LEP_RunFrameAverage( portDescPtr );
   }

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysFramesToAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_FRAME_AVERAGE_DIVISOR_E_PTR numFrameToAveragePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* one 32-bit value enum */

   /* Validate Parameter(s) 
   */
   if( numFrameToAveragePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current number of frames to average
   ** step
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_NUM_FRAMES_TO_AVERAGE,
                              ( LEP_ATTRIBUTE_T_PTR )numFrameToAveragePtr,
                              attributeWordLength );


   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetSysFramesToAverage( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_FRAME_AVERAGE_DIVISOR_E numFrameToAverage )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bit */

   /* Validate Parameter(s) 
   */
   if( numFrameToAverage >= LEP_SYS_END_FA_DIV )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current number of frames to average
   ** step
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_NUM_FRAMES_TO_AVERAGE,
                              ( LEP_ATTRIBUTE_T_PTR ) & numFrameToAverage,
                              attributeWordLength );


   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysSceneStatistics( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_SCENE_STATISTICS_T_PTR sceneStatisticsPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    /* struct contains 4 16-bit values */

   if( sceneStatisticsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_SCENE_STATISTICS,
                              ( LEP_ATTRIBUTE_T_PTR )sceneStatisticsPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysSceneRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_SYS_VIDEO_ROI_T_PTR sceneRoiPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    /* roi consists of 4 16-bit values */

   if( sceneRoiPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_SCENE_ROI,
                              ( LEP_ATTRIBUTE_T_PTR )sceneRoiPtr,
                              attributeWordLength );

   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_SetSysSceneRoi( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_SYS_VIDEO_ROI_T sceneRoi )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4;    /* roi consists of 4 16-bit values */

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_SCENE_ROI,
                              ( LEP_ATTRIBUTE_T_PTR ) & sceneRoi,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysThermalShutdownCount( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                           LEP_SYS_THERMAL_SHUTDOWN_COUNTS_T_PTR thermalCountsPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 1;    /* 1 16-bit value */

   if( thermalCountsPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_THERMAL_SHUTDOWN_COUNT,
                              ( LEP_ATTRIBUTE_T_PTR )thermalCountsPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysShutterPosition( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_SHUTTER_POSITION_E_PTR shutterPositionPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;    /* enums are 32-bit */

   if( shutterPositionPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_SHUTTER_POSITION,
                              ( LEP_ATTRIBUTE_T_PTR )shutterPositionPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetSysShutterPosition( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_SHUTTER_POSITION_E shutterPosition )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;     /* enums are 32-bit */

   if( shutterPosition >= LEP_SYS_SHUTTER_POSITION_END )
   {
      return( LEP_RANGE_ERROR );
   }

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_SHUTTER_POSITION,
                              ( LEP_ATTRIBUTE_T_PTR ) & shutterPosition,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysFfcShutterModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_SYS_FFC_SHUTTER_MODE_OBJ_T_PTR shutterModeObjPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;

   if( shutterModeObjPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FFC_SHUTTER_MODE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR )shutterModeObjPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetSysFfcShutterModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_SYS_FFC_SHUTTER_MODE_OBJ_T shutterModeObj )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FFC_SHUTTER_MODE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR ) & shutterModeObj,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_RunSysFFCNormalization( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_SYS_STATUS_E sysStatus = LEP_SYS_STATUS_BUSY;

   result = LEP_RunCommand( portDescPtr, ( LEP_COMMAND_ID )FLR_CID_SYS_RUN_FFC );
   while( sysStatus == LEP_SYS_STATUS_BUSY )
   {
      LEP_GetSysFFCStatus( portDescPtr, &sysStatus );
   }

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysFFCStatus( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_SYS_STATUS_E_PTR ffcStatusPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if( ffcStatusPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FFC_STATUS,
                              ( LEP_ATTRIBUTE_T_PTR )ffcStatusPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysGainMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_SYS_GAIN_MODE_E_PTR gainModePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* enums are 32-bit */

   if( gainModePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_GAIN_MODE,
                              ( LEP_ATTRIBUTE_T_PTR )gainModePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetSysGainMode( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_SYS_GAIN_MODE_E gainMode )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_GAIN_MODE,
                              ( LEP_ATTRIBUTE_T_PTR ) & gainMode,
                              attributeWordLength );

   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysGainModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_SYS_GAIN_MODE_OBJ_T_PTR gainModeObjPtr )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 14;

   if( gainModeObjPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_GAIN_MODE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR )gainModeObjPtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetSysGainModeObj( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                  LEP_SYS_GAIN_MODE_OBJ_T gainModeObj )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 14;

   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_GAIN_MODE_OBJ,
                              ( LEP_ATTRIBUTE_T_PTR ) & gainModeObj,
                              attributeWordLength );

   return( result );
}



LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysFFCStates( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                LEP_SYS_FFC_STATES_E_PTR ffcStatePtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2;

   if( ffcStatePtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_FFC_STATE,
                              ( LEP_ATTRIBUTE_T_PTR )ffcStatePtr,
                              attributeWordLength );

   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetSysBoresightValues( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_SYS_BORESIGHT_VALUES_T_PTR boresightValuesPtr)
{
    LEP_RESULT  result = LEP_OK;
    LEP_UINT16 attributeWordLength = 6;

    if( boresightValuesPtr == NULL )
    {
       return( LEP_BAD_ARG_POINTER_ERROR );
    }

    result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_SYS_BORESIGHT_VALUES,
                              ( LEP_ATTRIBUTE_T_PTR )boresightValuesPtr,
                              attributeWordLength );

    return( result );
}


/**************************************/
/*                VID                 */
/**************************************/
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidPolarity(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_POLARITY_E_PTR vidPolarityPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPolarityPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's Video Polarity
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_POLARITY_SELECT, 
                              (LEP_ATTRIBUTE_T_PTR)vidPolarityPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidPolarity(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                              LEP_POLARITY_E vidPolarity)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPolarity >= LEP_VID_END_POLARITY )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's Video Polarity
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_POLARITY_SELECT, 
                              (LEP_ATTRIBUTE_T_PTR)&vidPolarity,
                              attributeWordLength);
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_PCOLOR_LUT_E_PTR vidPcolorLutPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPcolorLutPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current Video LUT selection
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LUT_SELECT, 
                              (LEP_ATTRIBUTE_T_PTR)vidPcolorLutPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                               LEP_PCOLOR_LUT_E vidPcolorLut)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) vidPcolorLut;

   /* Validate Parameter(s)
   */
   if( vidPcolorLut >= LEP_VID_END_PCOLOR_LUT )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's current Video LUT selection
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LUT_SELECT, 
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength);
   return( result );
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidLowGainPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_PCOLOR_LUT_E_PTR vidPcolorLutPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPcolorLutPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current Video LUT selection
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LOW_GAIN_COLOR_LUT, 
                              (LEP_ATTRIBUTE_T_PTR)vidPcolorLutPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidLowGainPcolorLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_PCOLOR_LUT_E vidPcolorLut)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidPcolorLut >= LEP_VID_END_PCOLOR_LUT )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's current Video LUT selection
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LOW_GAIN_COLOR_LUT, 
                              (LEP_ATTRIBUTE_T_PTR)&vidPcolorLut,
                              attributeWordLength);
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidUserLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_VID_LUT_BUFFER_T_PTR vidUserLutBufPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 512; /* 512 16-bit values for 1024 byte LUT*/

   /* Validate Parameter(s)
   */
   if( vidUserLutBufPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's User Video LUT 
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LUT_TRANSFER, 
                              (LEP_ATTRIBUTE_T_PTR)vidUserLutBufPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidUserLut(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                             LEP_VID_LUT_BUFFER_T_PTR vidUserLutBufPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 512; /* 512 16-bit values for 1024 byte LUT*/

   /* Validate Parameter(s)
   */
   if( vidUserLutBufPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's User Video LUT
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_LUT_TRANSFER, 
                              (LEP_ATTRIBUTE_T_PTR)vidUserLutBufPtr,
                              attributeWordLength);
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidFocusCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_VID_FOCUS_CALC_ENABLE_E_PTR vidEnableFocusCalcStatePtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidEnableFocusCalcStatePtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's Video Focus Metric calculation enable 
   ** state
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_CALC_ENABLE, 
                              (LEP_ATTRIBUTE_T_PTR)vidEnableFocusCalcStatePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidFocusCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_VID_FOCUS_CALC_ENABLE_E vidFocusCalcEnableState)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) vidFocusCalcEnableState;

   /* Validate Parameter(s)
   */
   if( vidFocusCalcEnableState >= LEP_VID_END_FOCUS_CALC_ENABLE )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's Video Focus Metric Calculation enable
   ** state
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_CALC_ENABLE, 
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength);
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidBoresightCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E_PTR boresightCalcEnableStatePtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* Enums are 32-bit */

   if(boresightCalcEnableStatePtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute( portDescPtr,
                           (LEP_COMMAND_ID)LEP_CID_VID_BORESIGHT_CALC_ENABLE, 
                           (LEP_ATTRIBUTE_T_PTR)boresightCalcEnableStatePtr,
                           attributeWordLength );


   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidBoresightCalcEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                              LEP_VID_BORESIGHT_CALC_ENABLE_STATE_E boresightCalcEnableState)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* Enums are 32-bit */

   if(boresightCalcEnableState >= LEP_VID_END_BORESIGHT_CALC_ENABLE_STATE)
   {
      return(LEP_RANGE_ERROR);
   }

   result = LEP_SetAttribute( portDescPtr,
                           (LEP_COMMAND_ID)LEP_CID_VID_BORESIGHT_CALC_ENABLE, 
                           (LEP_ATTRIBUTE_T_PTR)&boresightCalcEnableState,
                           attributeWordLength);

   return(result);
}
LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidBoresightCoordinates(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_VID_BORESIGHT_COORDINATES_T_PTR boresightCoordinatesPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 attributeWordLength = 16;

   if( boresightCoordinatesPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   result = LEP_GetAttribute( portDescPtr,
                        (LEP_COMMAND_ID)LEP_CID_VID_BORESIGHT_COORDINATES, 
                        (LEP_ATTRIBUTE_T_PTR)boresightCoordinatesPtr,
                        attributeWordLength );

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidTargetPosition(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                    LEP_VID_TARGET_POSITION_T_PTR targetPositionPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_VID_BORESIGHT_COORDINATES_T boresightCoordinates;

   if(targetPositionPtr == NULL)
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }
#if (USE_BORESIGHT_MEASUREMENT_FUNCTIONS == 1)
   result = LEP_GetVidBoresightCoordinates(portDescPtr, &boresightCoordinates);
   if(result == LEP_OK)
   {
      LEP_CalcVidBoresightAlignment(boresightCoordinates, targetPositionPtr);
   }
#else
   return(LEP_FUNCTION_NOT_SUPPORTED);
#endif

   return(result);
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidROI(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                         LEP_VID_FOCUS_ROI_T_PTR vidROIPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* four 16-bit values */

   /* Validate Parameter(s)
   */
   if( vidROIPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's AGC ROI
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_ROI, 
                              (LEP_ATTRIBUTE_T_PTR)vidROIPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidROI(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                         LEP_VID_FOCUS_ROI_T vidROI)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 4; /* four 16-bit values */

   /* Perform Command
   ** Writing the Camera's AGC ROI
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_ROI, 
                              (LEP_ATTRIBUTE_T_PTR)&vidROI,
                              attributeWordLength);
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidFocusMetric(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                 LEP_VID_FOCUS_METRIC_T_PTR vidFocusMetricPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values */

   /* Validate Parameter(s)
   */
   if( vidFocusMetricPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current video Focus Metric calculation
   ** result
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_METRIC, 
                              (LEP_ATTRIBUTE_T_PTR)vidFocusMetricPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidFocusMetricThreshold(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_VID_FOCUS_METRIC_THRESHOLD_T_PTR vidFocusMetricThresholdPtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values */

   /* Validate Parameter(s)
   */
   if( vidFocusMetricThresholdPtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current video Focus Metric threshold
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_THRESHOLD, 
                              (LEP_ATTRIBUTE_T_PTR)vidFocusMetricThresholdPtr,
                              attributeWordLength );
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidFocusMetricThreshold(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                          LEP_VID_FOCUS_METRIC_THRESHOLD_T vidFocusMetricThreshold)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit value*/

   /* Validate Parameter(s)
   */

   /* Perform Command
   ** Writing the Camera's current video Focus Metric threshold
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FOCUS_THRESHOLD, 
                              (LEP_ATTRIBUTE_T_PTR)&vidFocusMetricThreshold,
                              attributeWordLength);
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidSbNucEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_VID_SBNUC_ENABLE_E_PTR vidSbNucEnableStatePtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidSbNucEnableStatePtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current Scene-Based NUC enable state
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_SBNUC_ENABLE, 
                              (LEP_ATTRIBUTE_T_PTR)vidSbNucEnableStatePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidSbNucEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                      LEP_VID_SBNUC_ENABLE_E vidSbNucEnableState)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidSbNucEnableState >= LEP_VID_END_SBNUC_ENABLE )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's current Scene-Based NUC enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_SBNUC_ENABLE, 
                              (LEP_ATTRIBUTE_T_PTR)&vidSbNucEnableState,
                              attributeWordLength);
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidFreezeEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_VID_FREEZE_ENABLE_E_PTR vidFreezeEnableStatePtr)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidFreezeEnableStatePtr == NULL )
   {
      return(LEP_BAD_ARG_POINTER_ERROR);
   }

   /* Perform Command
   ** Reading the Camera's current video freeze enable state
   */
   result = LEP_GetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FREEZE_ENABLE, 
                              (LEP_ATTRIBUTE_T_PTR)vidFreezeEnableStatePtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidFreezeEnableState(LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                       LEP_VID_FREEZE_ENABLE_E vidFreezeEnableState)
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidFreezeEnableState >= LEP_VID_END_FREEZE_ENABLE )
   {
      return(LEP_RANGE_ERROR);
   }

   /* Perform Command
   ** Writing the Camera's current video freeze enable state
   */
   result = LEP_SetAttribute( portDescPtr,
                              (LEP_COMMAND_ID)LEP_CID_VID_FREEZE_ENABLE, 
                              (LEP_ATTRIBUTE_T_PTR)&vidFreezeEnableState,
                              attributeWordLength);
   return( result );
}

LEP_RESULT LeptonSDKEmb32OEM::LEP_GetVidVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_VID_VIDEO_OUTPUT_FORMAT_E_PTR vidVideoOutputFormatPtr )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   /* Validate Parameter(s)
   */
   if( vidVideoOutputFormatPtr == NULL )
   {
      return( LEP_BAD_ARG_POINTER_ERROR );
   }

   /* Perform Command
   ** Reading the Camera's current video output format
   */
   result = LEP_GetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_VID_VIDEO_OUTPUT_FORMAT,
                              ( LEP_ATTRIBUTE_T_PTR )vidVideoOutputFormatPtr,
                              attributeWordLength );
   return( result );
}


LEP_RESULT LeptonSDKEmb32OEM::LEP_SetVidVideoOutputFormat( LEP_CAMERA_PORT_DESC_T_PTR portDescPtr,
                                        LEP_VID_VIDEO_OUTPUT_FORMAT_E vidVideoOutputFormat )
{
   LEP_RESULT  result = LEP_OK;
   LEP_UINT16 attributeWordLength = 2; /* two 16-bit values for 32-bit enum */

   _LEP_data[1] = 0;
   _LEP_data[0] = (LEP_UINT16) vidVideoOutputFormat;

   /* Validate Parameter(s)
   */
   if( vidVideoOutputFormat >= LEP_END_VID_VIDEO_OUTPUT_FORMAT )
   {
      return( LEP_RANGE_ERROR );
   }

   /* Perform Command
   ** Writing the Camera's current video output format
   */
   result = LEP_SetAttribute( portDescPtr,
                              ( LEP_COMMAND_ID )LEP_CID_VID_VIDEO_OUTPUT_FORMAT,
                              ( LEP_ATTRIBUTE_T_PTR ) & _LEP_data,
                              attributeWordLength );
   return( result );
}

#if (USE_BORESIGHT_MEASUREMENT_FUNCTIONS == 1)

LEP_RESULT LeptonSDKEmb32OEM::LEP_CalcVidBoresightAlignment(LEP_VID_BORESIGHT_COORDINATES_T boresightCoordinates, LEP_VID_TARGET_POSITION_T_PTR targetPositionPtr)
{
   LEP_RESULT result = LEP_OK;
   LEP_FLOAT32 top, bot, left, right, rows, cols;
   
   top = (boresightCoordinates.top_0.col + boresightCoordinates.top_1.col) / 2.0;
   bot = (boresightCoordinates.bottom_0.col + boresightCoordinates.bottom_1.col) / 2.0;
   left = (boresightCoordinates.left_0.row + boresightCoordinates.left_1.row) / 2.0;
   right = (boresightCoordinates.right_0.row + boresightCoordinates.right_1.row) / 2.0;

   rows = boresightCoordinates.bottom_1.row - boresightCoordinates.top_0.row + 1.0;
   cols = boresightCoordinates.right_1.col - boresightCoordinates.left_0.col + 1.0;

   /* Both lines are perfect horizontal/vertical */
   if((left == right) && (top == bot))
   {
      targetPositionPtr->col = top;
      targetPositionPtr->row = left;
      targetPositionPtr->rotation = 0.0;
   }
   /* Line 1 is perfect horizontal and Line 2 has slope */
   else if ((left == right) && (top != bot)) 
   {
      targetPositionPtr->row = left;
      targetPositionPtr->col = (LEP_FLOAT32)(bot + ((top - bot) / rows) * left);
      targetPositionPtr->rotation = (LEP_FLOAT32)((57.2957802 * atan((top - bot) / rows)) / 2.0);
   }
   /* Line 1 has slope and Line 2 is perfect vertical */ 
   else if ((left != right) && (top == bot)) 
   {
      targetPositionPtr->col = top;
      targetPositionPtr->row = (LEP_FLOAT32)(right + (right - left) / cols * top / cols);
      targetPositionPtr->rotation = (LEP_FLOAT32)((57.2957802 * atan((right - left) / cols)) / 2.0);
   } 
   /* Both lines have slope */
   else 
   {
      targetPositionPtr->col = (LEP_FLOAT32)(((rows - rows * top / (top - bot)) - left) / ((right - left) / cols - rows / (top - bot)));
      targetPositionPtr->row = (LEP_FLOAT32)(left + targetPositionPtr->col * (right - left) / cols);
      targetPositionPtr->rotation = (LEP_FLOAT32)((57.2957802 * (atan((right - left) / cols) + atan((top - bot) / rows))) / 2.0);
   }

   return(result);
}
#endif


/**************************************/
/*            Teensy I2C              */
/**************************************/
LEP_RESULT LeptonSDKEmb32OEM::DEV_I2C_MasterSelectDevice(LEP_PROTOCOL_DEVICE_E device)
{
    LEP_RESULT result = LEP_OK;

    return(result);
}


/******************************************************************************/
/**
 * Performs I2C Master Initialization
 * 
 * @param portID     LEP_UINT16  User specified port ID tag.  Can be used to
 *                   select between multiple cameras
 * 
 * @param BaudRate   Clock speed in kHz. Typically this is 400.
 *                   The Device Specific Driver will try to match the desired
 *                   speed.  This parameter is updated to the actual speed the
 *                   driver can use.
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise
 */
LEP_RESULT LeptonSDKEmb32OEM::DEV_I2C_MasterInit(LEP_UINT16 portID, 
                              LEP_UINT16 *BaudRate)
{
    LEP_RESULT result = LEP_OK;

    Wire.begin();
    Wire.setClock(*BaudRate*1000);
    *BaudRate = Wire.getClock()/1000;

    return(result);
}

/**
 * Closes the I2C driver connection.
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT LeptonSDKEmb32OEM::DEV_I2C_MasterClose()
{
    LEP_RESULT result = LEP_OK;

    return(result);
}

/**
 * Resets the I2C driver back to the READY state.
 * 
 * @return LEP_RESULT  0 if all goes well, errno otherwise.
 */
LEP_RESULT LeptonSDKEmb32OEM::DEV_I2C_MasterReset(void )
{
   LEP_RESULT result = LEP_OK;

  Wire.resetBus();

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::DEV_I2C_MasterReadData(LEP_UINT16  portID,               // User-defined port ID
                                  LEP_UINT8   deviceAddress,        // Lepton Camera I2C Device Address
                                  LEP_UINT16  regAddress,           // Lepton Register Address
                                  LEP_UINT16 *readDataPtr,          // Read DATA buffer pointer
                                  LEP_UINT16  wordsToRead,          // Number of 16-bit words to Read
                                  LEP_UINT16 *numWordsRead,         // Number of 16-bit words actually Read
                                  LEP_UINT16 *status                // Transaction Status
                                 )
{
   LEP_RESULT result = LEP_OK;

    /* Place Device-Specific Interface here
    */ 
   
   LEP_UINT32 bytesToRead = wordsToRead << 1;
   LEP_UINT32 bytesActuallyRead = 0;
   LEP_UINT16 wordsActuallyRead = 0;
   LEP_UINT16 *writePtr;


   /*
     Write the address, which is 2 bytes
   */
   Wire.beginTransmission(deviceAddress);
   Wire.write(regAddress >> 8);
   Wire.write(regAddress & 0xFF);
   *status = Wire.endTransmission();
//Serial.printf("For reg %d trying %d, ", regAddress, bytesToRead);
   if (*status != 0) {
      result = LEP_ERROR;
   } else {
       /*
             Read back the data at the address written above
       */
       bytesActuallyRead = Wire.requestFrom(deviceAddress, bytesToRead, I2C_STOP);
       if (bytesActuallyRead == 0) {
          i2c_status st;
          st = Wire.status();
          Serial.printf("DEV_I2C_MasterReadData requestFrom %d of %d failed with %d - ", regAddress, bytesToRead, st);
          switch (st) {
              case I2C_WAITING:
                  Serial.println("WAITING");
                  break;
              case I2C_TIMEOUT:
                  Serial.println("TIMEOUT");
                  break;
              case I2C_ADDR_NAK:
                  Serial.println("ADDR_NAK");
                  break;
              case I2C_DATA_NAK:
                  Serial.println("DATA_NAK");
                  break;
              case I2C_ARB_LOST:
                  Serial.println("ARB_LOST");
                  break;
              case I2C_BUF_OVF:
                  Serial.println("BUF_OVF");
                  break;
              case I2C_NOT_ACQ:
                  Serial.println("NOT_ACQ");
                  break;
              case I2C_DMA_ERR:
                  Serial.println("DMA_ERR");
                  break;
              case I2C_SENDING:
                  Serial.println("SENDING");
                  break;
              case I2C_SEND_ADDR:
                  Serial.println("SEND_ADDR");
                  break;
              case I2C_RECEIVING:
                  Serial.println("RECEIVING");
                  break;
              case I2C_SLAVE_TX:
                  Serial.println("SLAVE_TX");
                  break;
              case I2C_SLAVE_RX:
                  Serial.println("SLAVE_RX");
                  break;
          }
//       } else {
//          Serial.printf("read %d\n", bytesActuallyRead);
       }
/*
       if (bytesActuallyRead != bytesToRead)
       {
          result = LEP_ERROR;
       }
*/

   }

   wordsActuallyRead = (LEP_UINT16)((bytesActuallyRead >> 1) + (bytesActuallyRead & 0x01));
   *numWordsRead = wordsActuallyRead;

   if(result == LEP_OK)
   {
       writePtr = readDataPtr;
       while(wordsActuallyRead--)
       {
          *writePtr  = Wire.readByte() << 8;
          *writePtr |= Wire.readByte();
//Serial.println(*writePtr, HEX);
          writePtr++;
       }
   }

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::DEV_I2C_MasterWriteData(LEP_UINT16  portID,              // User-defined port ID
                                   LEP_UINT8   deviceAddress,       // Lepton Camera I2C Device Address
                                   LEP_UINT16  regAddress,          // Lepton Register Address
                                   LEP_UINT16 *writeDataPtr,        // Write DATA buffer pointer
                                   LEP_UINT16  wordsToWrite,        // Number of 16-bit words to Write
                                   LEP_UINT16 *numWordsWritten,     // Number of 16-bit words actually written
                                   LEP_UINT16 *status)              // Transaction Status
{
   LEP_RESULT result = LEP_OK;

   *numWordsWritten = wordsToWrite;
   
   Wire.beginTransmission(deviceAddress);
   Wire.write(regAddress >> 8);
   Wire.write(regAddress & 0xFF);
//Serial.printf("Writing to %d: ", regAddress);
   while (wordsToWrite--) {
       Wire.write(*writeDataPtr >> 8);
       Wire.write(*writeDataPtr & 0xFF);
//Serial.printf("0x%4x ", *writeDataPtr);
       writeDataPtr++;
   } 
//Serial.printf("\n");
   *status = Wire.endTransmission();

   if (*status != 0) {
      result = LEP_ERROR;
   }
   
   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::DEV_I2C_MasterReadRegister( LEP_UINT16 portID,
                                       LEP_UINT8  deviceAddress, 
                                       LEP_UINT16 regAddress,
                                       LEP_UINT16 *regValue,     // Number of 16-bit words actually written
                                       LEP_UINT16 *status
                                     )
{
    LEP_RESULT result = LEP_OK;

   LEP_UINT16 wordsActuallyRead;
    /* Place Device-Specific Interface here
    */ 
   result = DEV_I2C_MasterReadData(portID, deviceAddress, regAddress, regValue, 1 /*1 word*/, &wordsActuallyRead, status);

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::DEV_I2C_MasterWriteRegister( LEP_UINT16 portID,
                                        LEP_UINT8  deviceAddress, 
                                        LEP_UINT16 regAddress,
                                        LEP_UINT16 regValue,     // Number of 16-bit words actually written
                                        LEP_UINT16 *status
                                      )
{
   LEP_RESULT result = LEP_OK;
   LEP_UINT16 wordsActuallyWritten;
    /* Place Device-Specific Interface here
    */ 
   result = DEV_I2C_MasterWriteData(portID, deviceAddress, regAddress, &regValue, 1, &wordsActuallyWritten, status);

   return(result);
}

LEP_RESULT LeptonSDKEmb32OEM::DEV_I2C_MasterStatus(void )
{
    LEP_RESULT result = LEP_OK;

    /* Place Device-Specific Interface here
    */ 


    return(result);
}


/**************************************/
/*             crc16fast              */
/**************************************/
/*
 *  ===== ByteCRC16 =====
 *      Calculate (update) the CRC16 for a single 8-bit byte
 */
int LeptonSDKEmb32OEM::ByteCRC16(int value, int crcin)
{
	return (unsigned short)((crcin << 8) ^  ccitt_16Table[((crcin >> 8) ^ (value)) & 255]);
}

/*

 *  ===== CalcCRC16Words =====
 *      Calculate the CRC for a buffer of 16-bit words.  Supports both
 *  Little and Big Endian formats using conditional compilation.
 *      Note: minimum count is 1 (0 case not handled)
 */
CRC16 LeptonSDKEmb32OEM::CalcCRC16Words(unsigned int count, short *buffer) {

    int crc = 0;

    do {

        int value = *buffer++;
#ifdef _BIG_ENDIAN
        crc = ByteCRC16(value >> 8, crc);
        crc = ByteCRC16(value, crc);
#else
        crc = ByteCRC16(value, crc);
        crc = ByteCRC16(value >> 8, crc);
#endif
    }
	while (--count);
    return (CRC16) crc;
}

/*
 *  ===== CalcCRC16Bytes =====
 *      Calculate the CRC for a buffer of 8-bit words.
 *      Note: minimum count is 1 (0 case not handled)
 */
CRC16 LeptonSDKEmb32OEM::CalcCRC16Bytes(unsigned int count, char *buffer) {

    int crc = 0;

    do {

        int value = *buffer++;
        crc = ByteCRC16(value, crc);
    }
	while (--count);
    return crc;
}

