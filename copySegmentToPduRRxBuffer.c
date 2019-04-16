/*
 * copySegmentToPduRRxBuffer.c
 *
 *  Created on: Apr 15, 2019
 *      Author: Karim Sherif Aref
 */

#include "CanTp_Types.h"
#include "CanTp.h"


/*
 *
 * 		>>> Add these Macros to your code to check and report for errors if needed <<<
 *
 *
 *
 *
If DET is ON, report errors.   If DET is OFF, do nothing
#if  ( CANTP_DEV_ERROR_DETECT == STD_ON )

#define VALIDATE(_exp,_api,_err ) \
		if( !(_exp) ) { \
			Det_ReportError(MODULE_ID_CANTP, 0, _api, _err); \
			return E_NOT_OK; \
		}

#define VALIDATE_NO_RV(_exp,_api,_err ) \
		if( !(_exp) ) { \
			Det_ReportError(MODULE_ID_CANTP, 0, _api, _err); \
			return; \
		}
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif
*/


/*
  				check for errors and report if found any
				VALIDATE( rxRuntime->pdurBuffer->SduDataPtr != NULL, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_RX_BUFFER );

				add this line in the if return_value - bufq_ok condition to check for errors

*/

/*End of Macros*/










/*
 * This function copies the segment to PduR Receiving Buffer, requests a new buffer from the SDUR if needed,
 * and reports the error to the Development Error Tracer (DET).
 *
 * Function Name: copySegmentToPduRRxBuffer
 * Parameters:
 * 				Inputs: 5
 *
 * 					1.	Name: *rxConfig
 * 						Type: CanTp_RxNSduType
 * 						Description: pointer to a structure contains the configurations of RX.
 *
 * 					2.	Name: *rxRuntime
 * 						Type: CanTp_ChannelPrivateType
 * 						Description: pointer to a structure contains the runtime parameters of RX.
 *
 * 					3.	Name: *segment
 * 						Type: uint8
 * 						Description: pointer to the segment or the data to be copied.
 *
 * 					4.	Name: segmentSize
 * 						Type: PduLengthType
 * 						Description: length of the Pdu.
 *
 * 					5.	Name: *bytesWrittenSuccessfully
 * 						Type: PduLengthType
 * 						Description: length of bytes written successfully.
 *
 *
 * 				Returns:
 *						BufReq_ReturnType:  BUFREQ_OK         0
 *	  								   		BUFREQ_NOT_OK     1
 *	  								   		BUFREQ_BUSY       2
 *	  								   		BUFREQ_OVFL       3
 *
 */

static BufReq_ReturnType copySegmentToPduRRxBuffer(   const CanTp_RxNSduType *rxConfig,
													CanTp_ChannelPrivateType *rxRuntime,
																		uint8 *segment,
																PduLengthType segmentSize,
																PduLengthType *BytesWrittenSuccessfully)
{

	BufReq_ReturnType return_value = BUFREQ_NOT_OK;
	bool endLoop = FALSE;
	*BytesWrittenSuccessfully = 0;

	while ((*BytesWrittenSuccessfully < segmentSize) && (!endLoop))
	{
		/*copy the data in the buffer as long as loop is not ended and there`s a room for copying*/

		if (rxRuntime->pdurBuffer != NULL)
		{
			/*no new buffer is needed*/
			while ((*BytesWrittenSuccessfully < segmentSize ) && (rxRuntime->pdurBuffer->SduLength > rxRuntime->pdurBufferCount))
			{
				rxRuntime->pdurBuffer->SduDataPtr[rxRuntime->pdurBufferCount++] = segment[(*BytesWrittenSuccessfully)++];
			}
		}

		if (*BytesWrittenSuccessfully < segmentSize )
		{
			/*request a new buffer from the SDUR.*/
			return_value = PduR_CanTpProvideRxBuffer(rxConfig->PduR_PduId, rxRuntime->transferTotal, &rxRuntime->pdurBuffer);

			/* just for protection, timeout should be added here but not necessary, the program should run well without it*/

			if (return_value == BUFREQ_OK)
			{
				/*new buffer request is successfully completed*/
				rxRuntime->pdurBufferCount = 0;   /*Empty the buffer.*/
			}

			else if (return_value == BUFREQ_BUSY)
			{
				rxRuntime->transferCount += *BytesWrittenSuccessfully;
				endLoop = TRUE;
			}

			else
			{
				endLoop = TRUE; /*###### This error must be handled while calling the function ######*/
			}

		}

		else
		{
			rxRuntime->transferCount += segmentSize; /*== BytesWrittenSuccessfully*/
			return_value = BUFREQ_OK;
			endLoop = TRUE;
		}
	}
	return return_value;
}
