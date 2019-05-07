#include "CanTpRuntime.h"
#include <stdio.h>
#include <stdlib.h>
#include "CanTp_Types.h"
#include "CanTp_Cfg.h"
#include "CanTp.h"
#include "Helper_Functions.h"
#include "CanTp_Cbk.h"
#include "SchM_CanTp.h"
#include "MemMap.h"
#include "Det.h"

extern CanTp_RunTimeDataType CanTpRunTimeData;
extern CanTp_ConfigType CanTp_Config;

/*************************************************** Helper Functions ************************************************************/

Std_ReturnType canReceivePaddingHelper(const CanTp_RxNSduType *rxConfig, CanTp_ChannelPrivateType *rxRuntime, PduInfoType *PduInfoPtr)
{
	if (rxConfig->CanTpRxPaddingActivation == CANTP_ON)
	{
		uint8 i = 0;
		for ( i = PduInfoPtr->SduLength; i < MAX_SEGMENT_DATA_SIZE; i++)
		{
			PduInfoPtr->SduDataPtr[i] = 0x0;
		}
		PduInfoPtr->SduLength = MAX_SEGMENT_DATA_SIZE;
	}
	rxRuntime->iso15765.NasNarTimeoutCount = (rxConfig->CanTpNar);  // Value in seconds of the N_Ar timeout. N_Ar is the time for transmission of a CAN frame (any N_PDU) on the receiver side.
	rxRuntime->iso15765.NasNarPending = TRUE;

//	return CanIf_Transmit(rxConfig->CanIf_FcPduId, PduInfoPtr);
	return E_OK;
}

/*
 * This function copies the segment to PduR Receiving Buffer, requests a new buffer from the SDUR if needed,
 * and reports the error to the Development Error Tracer (DET).
 *-3
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

//ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime, data, pduLength, &bytesWrittenToSduRBuffer);

BufReq_ReturnType copySegmentToPduRRxBuffer(   const CanTp_RxNSduType *rxConfig,
													CanTp_ChannelPrivateType *rxRuntime,
																PduInfoType *info,         // { Data , length }
																PduLengthType segmentSize )	   // pduLength
{

	BufReq_ReturnType return_value = BUFREQ_NOT_OK;



		/* copy the data in the buffer as long as loop there`s a room for copying */


			return_value = PduR_CanTpStartOfReception(rxConfig->CanTpRxNPdu.CanTpRxNPduId,info,rxRuntime->transferTotal,&rxRuntime->Buffersize);


		/* return form  PduR_CanTpStartOfReception */

		if (return_value == BUFREQ_OK)
		{

			return_value = PduR_CanTpCopyRxData(rxConfig->CanTpRxNPdu.CanTpRxNPduId,info,&rxRuntime->Buffersize);

		}

			else if (return_value == BUFREQ_BUSY)
			{

			}

			else	 /* in case of BUFREQ_NOT_OK or BUFREQ_OVFL */
			{

			}


	return return_value;
}

/* this function coping data and length to Rx_runtime.canFrameBufferData if segementsize < MAX_SEGMENT_DATA_SIZE and return True if copying Done */
boolean copySegmentToLocalRxBuffer(CanTp_ChannelPrivateType *rxRuntime, uint8 *segment,PduLengthType segmentSize)
{
	boolean ret = FALSE;

	if ( segmentSize < MAX_SEGMENT_DATA_SIZE )
	{
		uint8 i;
		/*This for loop copying the Data to local Buffer*/
		for (i=0; i < segmentSize; i++)
		{
			rxRuntime->canFrameBuffer.data[i] = segment[i];              // segment is data
		}

		rxRuntime->canFrameBuffer.byteCount = segmentSize;              // copy Length To The local Buffer
		ret = TRUE;
	}
	return ret;			/* return True if copying is Done as the size is small than MAX_SEGMENT_DATA_SIZE */
}

/* This Function get The Frame Type { SF,FF,CF,CTS_FC,WAIT_FC,OVERFLOW_FC } */

ISO15765FrameType getFrameType(const CanTp_AddressingFormatType *formatType,const PduInfoType *CanTpRxPduPtr)
{
	ISO15765FrameType res = INVALID_FRAME;
	uint8 tpci = 0;

	switch (*formatType)
	{
	case CANTP_STANDARD:
		tpci = CanTpRxPduPtr->SduDataPtr[0];
		break;

	case CANTP_EXTENDED:
		tpci = CanTpRxPduPtr->SduDataPtr[1];
		break;

	default:
		break;
	}

	switch (tpci & ISO15765_TPCI_MASK)
	{
	case ISO15765_TPCI_SF:
		res = SINGLE_FRAME;
		break;
	case ISO15765_TPCI_FF:
		res = FIRST_FRAME;
		break;
	case ISO15765_TPCI_CF:
		res = CONSECUTIVE_FRAME;
		break;

	case ISO15765_TPCI_FC: 						/* check which kind of flow control. */
		switch (tpci & ISO15765_TPCI_FS_MASK)
		{
		case ISO15765_FLOW_CONTROL_STATUS_CTS:
			res = FLOW_CONTROL_CTS_FRAME;
			break;
		case ISO15765_FLOW_CONTROL_STATUS_WAIT:
			res = FLOW_CONTROL_WAIT_FRAME;
			break;
		case ISO15765_FLOW_CONTROL_STATUS_OVFLW:
			res = FLOW_CONTROL_OVERFLOW_FRAME;
			break;
		}
	}
	return res;
}

/* This Fuction is used to get the length of the PDU from N_PCI */
PduLengthType getPduLength(const CanTp_AddressingFormatType *formatType,const ISO15765FrameType iso15765Frame, const PduInfoType *CanTpRxPduPtr)
{
	PduLengthType res = 0;
	uint8 tpci_offset = 0;


	 /* to check the format type */
	switch (*formatType)
	{
	case CANTP_STANDARD:
		tpci_offset = 0;
		break;
	case CANTP_EXTENDED:
		tpci_offset = 1;
		break;
	default:
		return 0;
	}

	switch (iso15765Frame)
	{
	case SINGLE_FRAME:
		// Parse the data length from the single frame header.
		res = CanTpRxPduPtr->SduDataPtr[tpci_offset] & ISO15765_TPCI_DL;
		break;
	case FIRST_FRAME:
		// Parse the data length form the first frame.
		res = CanTpRxPduPtr->SduDataPtr[tpci_offset + 1] + ((PduLengthType)((CanTpRxPduPtr->SduDataPtr[tpci_offset]) & 0xf) << 8);
		break;
	default:
		res = 0;
		break;
	}
	return res;
}


/* This function returns either it SF or FF */
ISO15765FrameType calcRequiredProtocolFrameType(const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime)
{

	ISO15765FrameType ret = INVALID_FRAME;

	if (txConfig->CanTpTxAddressingFormat == CANTP_EXTENDED)
	{
		if (txRuntime->transferTotal <= MAX_PAYLOAD_CF_EXT_ADDR)
		{
			ret = SINGLE_FRAME;
		}
		else
		{
			if (txConfig->CanTpTxTaType == CANTP_PHYSICAL)
			{
				ret = FIRST_FRAME;
			}
			else
			{
			}
		}
	}

	else 					// in case of  CANTP_STANDARD
	{
		if (txRuntime->transferTotal <= MAX_PAYLOAD_CF_STD_ADDR)
		{
			ret = SINGLE_FRAME;
		}
		else
		{
			if (txConfig->CanTpTxTaType == CANTP_PHYSICAL)
			{
				ret = FIRST_FRAME;
			}
			else
			{
			}
		}
	}

	return ret;
}

/*************************************************** Handling All Frames ************************************************************/

void handleSingleFrame(const CanTp_RxNSduType *rxConfig,CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData)
{
	BufReq_ReturnType ret;
	PduLengthType pduLength;
	PduInfoType *info = NULL;
	uint8 * data = rxPduData;



	if (rxRuntime->iso15765.state != IDLE)
	{
		PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_NOT_OK);  // Abort current reception, we need to tell the current receiver it has been aborted.
	}

	initRx15765RuntimeData(rxRuntime);           // i think this line is useless
	pduLength = getPduLength(&rxConfig->CanTpRxAddressingFormat, SINGLE_FRAME, rxPduData);



	if (rxConfig->CanTpRxAddressingFormat == CANTP_STANDARD)
	{
		info->SduDataPtr = &rxPduData->SduDataPtr[1];
	}
	else								/* in case of Extended Addressing format */
	{
		info->SduDataPtr = &rxPduData->SduDataPtr[2];
	}

	rxRuntime->transferTotal = pduLength;                /* copying length to runtime */
	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNbr);

	ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime, info, pduLength);

	if (ret == BUFREQ_OK)
	{
		PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_OK);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;

	}
	else if (ret == BUFREQ_BUSY)
	{
		if (rxConfig->CanTpRxAddressingFormat == CANTP_STANDARD)
		{
			data = &rxPduData->SduDataPtr[1];
		}
		else
		{
			data = &rxPduData->SduDataPtr[2];
		}
		(void)copySegmentToLocalRxBuffer(rxRuntime, data, pduLength ); 			   /* copy data to local buffer in case of BUFREQ_BUSY */

		rxRuntime->iso15765.state = RX_WAIT_SDU_BUFFER;
		rxRuntime->mode = CANTP_RX_PROCESSING;
	}

	else      /* in case of BUFREQ_NOT_OK or BUFREQ_OVFL */
	{
		PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_NO_BUFFER);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}


void handleFirstFrame(const CanTp_RxNSduType *rxConfig,CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData)
{
	BufReq_ReturnType ret;
	PduLengthType pduLength;
	PduInfoType *info = rxPduData;


	if (rxRuntime->iso15765.state != IDLE)
	{
		PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_NOT_OK); // Abort current reception, we need to tell the current receiver it has been aborted.
	}

	(void) initRx15765RuntimeData(rxRuntime);                         /* I think this line is useless */
	pduLength = getPduLength(&rxConfig->CanTpRxAddressingFormat, FIRST_FRAME,rxPduData);
	rxRuntime->transferTotal = pduLength;




	// Validate that that there is a reason for using the segmented transfers and
	// if not simply skip (single frame should have been used).
	if (rxConfig->CanTpRxAddressingFormat == CANTP_STANDARD)
	{
		if (pduLength <= MAX_PAYLOAD_SF_STD_ADDR)
		{
			return;
		}
	}
	else
	{
		if (pduLength <= MAX_PAYLOAD_SF_EXT_ADDR)
		{
			return;
		}
	}

	// Validate that the SDU is full length in this first frame.
	if (rxPduData->SduLength < MAX_SEGMENT_DATA_SIZE)
	{
		return;
	}

	/* point the data to correct byte  */

	if (rxConfig->CanTpRxAddressingFormat == CANTP_STANDARD)
	{
		info->SduDataPtr = &rxPduData->SduDataPtr[2];
	}
	else								/* in case of Extended Addressing format */
	{
		info->SduDataPtr = &rxPduData->SduDataPtr[3];
	}



	rxRuntime->iso15765.framesHandledCount = 1;			 // Segment count begins with 1 (FirstFrame has the 0).
	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNbr);


	ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime,info,MAX_PAYLOAD_FF_STD_ADDR);



	if (ret == BUFREQ_OK)
	{
		rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNcr); // Value in seconds of the N_Cr timeout. N_Cr is the time until reception of the next Consecutive Frame N_PDU.
		rxRuntime->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret);
	}
	else if (ret == BUFREQ_BUSY)
	{
		if (rxConfig->CanTpRxAddressingFormat == CANTP_STANDARD)
		{
			(void)copySegmentToLocalRxBuffer(rxRuntime, &rxPduData->SduDataPtr[2], MAX_PAYLOAD_FF_STD_ADDR );
		}
		else
		{
			(void)copySegmentToLocalRxBuffer(rxRuntime, &rxPduData->SduDataPtr[3], MAX_PAYLOAD_FF_EXT_ADDR );
		}
		rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNbr);
		rxRuntime->iso15765.state = RX_WAIT_SDU_BUFFER;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret);
	}
	else if (ret == BUFREQ_OVFL)
	{
		sendFlowControlFrame(rxConfig, rxRuntime, ret);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}



void handleFlowControlFrame(const CanTp_TxNSduType *txConfig,CanTp_ChannelPrivateType *txRuntime, const PduInfoType *txPduData)
{

	uint8 indexCount = 0;             // Farag change this from int to uint8
	uint8 extendedAddress = 0;

	if ( txRuntime->iso15765.state == TX_WAIT_FLOW_CONTROL )
	{
		if (txConfig->CanTpTxAddressingFormat == CANTP_EXTENDED)
		{
			extendedAddress = txPduData->SduDataPtr[indexCount++];                     // code we7esh we hanshelha 3ashan mabyesta5demhash
		}

		// txPduData->SduDataPtr[indexCount++] = 1st byte (frame type + flow control flag)
		switch (txPduData->SduDataPtr[indexCount++] & ISO15765_TPCI_FS_MASK) // checking the flow control flag (Clear To Send=0 , Wait=1, Overflow/abort =2)
		{
		case ISO15765_FLOW_CONTROL_STATUS_CTS:
#if 1
		{	// This construction is added to make the hcs12 compiler happy.
			const uint8 bs = txPduData->SduDataPtr[indexCount++];
			txRuntime->iso15765.BS = bs;
			txRuntime->iso15765.nextFlowControlCount = bs;
		}
		txRuntime->iso15765.STmin = txPduData->SduDataPtr[indexCount++];
#else
		txRuntime->iso15765.BS = txPduData->SduDataPtr[indexCount++];
		txRuntime->iso15765.nextFlowControlCount = txRuntime->iso15765.BS;
		txRuntime->iso15765.STmin = txPduData->SduDataPtr[indexCount++];
#endif
		// change state and setup timout
		txRuntime->iso15765.stateTimeoutCount = (txConfig->CanTpNcs);
		txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
		break;

		case ISO15765_FLOW_CONTROL_STATUS_WAIT:
			txRuntime->iso15765.stateTimeoutCount = (txConfig->CanTpNbs);  /*CanTp: 264*/
			txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
			break;

		case ISO15765_FLOW_CONTROL_STATUS_OVFLW:
			PduR_CanTpRxIndication(txConfig->CanTpRxFcNPdu.CanTpRxFcNPduId, NTFRSLT_E_NOT_OK);       // 27na benehbed ,,,, PduR_PduID
			txRuntime->iso15765.state = IDLE;
			txRuntime->mode = CANTP_TX_WAIT;
			break;
		}
	}
}


void handleConsecutiveFrame(const CanTp_RxNSduType *rxConfig,CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData)
{
	uint8 indexCount = 0;
	uint8 segmentNumber = 0;
	uint8 extendedAddress = 0;
	PduLengthType bytesLeftToCopy = 0;
	PduLengthType bytesLeftToTransfer = 0;
	PduLengthType currentSegmentSize = 0;
	PduLengthType currentSegmentMaxSize = 0;
	PduLengthType bytesCopiedToPdurRxBuffer = 0;
	BufReq_ReturnType ret = BUFREQ_NOT_OK;

	PduInfoType *info = rxPduData;


	if (rxRuntime->iso15765.state == RX_WAIT_CONSECUTIVE_FRAME)
	{
		if (rxConfig->CanTpRxAddressingFormat == CANTP_EXTENDED)
		{
			extendedAddress = rxPduData->SduDataPtr[indexCount++];
		}

		// getting consecutive frame number
		segmentNumber = rxPduData->SduDataPtr[indexCount++] & SEGMENT_NUMBER_MASK;      //segment number equals consecutive frame index (3..0 bits in first byte)

		// checking if this consecutive frame is handled
		if (segmentNumber != (rxRuntime->iso15765.framesHandledCount & SEGMENT_NUMBER_MASK))
		{
			PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_WRONG_SN);
			rxRuntime->iso15765.state = IDLE;
			rxRuntime->mode = CANTP_RX_WAIT;
		}
		else
		{
			currentSegmentMaxSize = MAX_SEGMENT_DATA_SIZE - indexCount;                  // 6 bytes in extended add. & 7 bytes in standard add.
			bytesLeftToCopy = rxRuntime->transferTotal - rxRuntime->transferCount;

			// determining segment size
			if (bytesLeftToCopy < currentSegmentMaxSize)
			{
				currentSegmentSize = bytesLeftToCopy; // 1-5.
			}
			else
			{
				currentSegmentSize = currentSegmentMaxSize; 		// 6 or 7, depends on addressing format used.
			}

			// Copy received data to buffer provided by SDUR.
			info->SduDataPtr = &rxPduData->SduDataPtr[indexCount];


			ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime,info,currentSegmentSize);

			if (ret == BUFREQ_NOT_OK) {
				PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_NO_BUFFER);
				rxRuntime->iso15765.state = IDLE;
				rxRuntime->mode = CANTP_RX_WAIT;
			}
			else if (ret == BUFREQ_BUSY)
			{
				boolean dataCopyFailure = FALSE;
				PduLengthType bytesNotCopiedToPdurRxBuffer = currentSegmentSize - bytesCopiedToPdurRxBuffer;

				// checking copy failure
				if (rxConfig->CanTpRxAddressingFormat == CANTP_STANDARD)
				{
					if ( copySegmentToLocalRxBuffer(rxRuntime,	&rxPduData->SduDataPtr[1 + bytesCopiedToPdurRxBuffer],bytesNotCopiedToPdurRxBuffer ) != TRUE )
					{
						rxRuntime->iso15765.state = IDLE;
						rxRuntime->mode = CANTP_RX_WAIT;
						dataCopyFailure = TRUE;
					}

				}
				else    // in extended add.
				{
					if ( copySegmentToLocalRxBuffer(rxRuntime,&rxPduData->SduDataPtr[2 + bytesCopiedToPdurRxBuffer],bytesNotCopiedToPdurRxBuffer) != TRUE )
					{
						rxRuntime->iso15765.state = IDLE;
						rxRuntime->mode = CANTP_RX_WAIT;
						dataCopyFailure = TRUE;
					}
				}

				if ( !dataCopyFailure )             // Data is copied correctly
				{
					rxRuntime->iso15765.framesHandledCount++;
					rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNbr);
					rxRuntime->iso15765.state = RX_WAIT_SDU_BUFFER;
					rxRuntime->mode = CANTP_RX_PROCESSING;
					sendFlowControlFrame(rxConfig, rxRuntime, ret);
				}
			}
			else if (ret == BUFREQ_OK)
			{
				bytesLeftToTransfer = rxRuntime->transferTotal - rxRuntime->transferCount;
				if (bytesLeftToTransfer > 0)
				{
					rxRuntime->iso15765.framesHandledCount++;                  // hanzawed 3adad el frames elly 2tna2alet
					COUNT_DECREMENT(rxRuntime->iso15765.nextFlowControlCount); // han2alel 3adad el frames elly fadla 3ala  el next flow control

					if (rxRuntime->iso15765.nextFlowControlCount == 0  && rxRuntime->iso15765.BS > 0)
					{
						sendFlowControlFrame(rxConfig, rxRuntime, BUFREQ_OK);
					}
					else
					{
						rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNcr);  // Value in seconds of the N_Cr timeout. N_Cr is the time until reception of the next Consecutive Frame N_PDU.
					}
				}
				else
				{
					rxRuntime->iso15765.state = IDLE;
					rxRuntime->mode = CANTP_RX_WAIT;
					PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_OK);
				}
			}
		}
	}
} 					// 438, 550 PC-lint: extendedAdress not accessed. Extended adress needs to be implemented. Ticket #136


/****************************************************** sending  Frames **************************************************************/

void sendFlowControlFrame(const CanTp_RxNSduType *rxConfig, CanTp_ChannelPrivateType *rxRuntime, BufReq_ReturnType flowStatus)
{
	uint8 indexCount = 0;
	Std_ReturnType ret = E_NOT_OK;
	PduInfoType pduInfo;
	uint8 sduData[8];				 // Note that buffer is declared on the stack.
	uint16 spaceFreePduRBuffer = 0;
	uint16 computedBs = 0;

	pduInfo.SduDataPtr = &sduData[0];         // make pointer points to array of data


	if (rxConfig->CanTpRxAddressingFormat == CANTP_EXTENDED)
	{
		sduData[indexCount++] = rxRuntime->iso15765.extendedAddress;
	}

	switch (flowStatus)
	{
	case BUFREQ_OK:
	{
		sduData[indexCount++] = ISO15765_TPCI_FC | ISO15765_FLOW_CONTROL_STATUS_CTS;        	// change the value of control flag (clear to send)

		spaceFreePduRBuffer = rxRuntime->pdurBuffer->SduLength - rxRuntime->pdurBufferCount;  /* TODO: I think this would be bufferSize directly */



		if (rxConfig->CanTpRxAddressingFormat == CANTP_EXTENDED)                         // for Extended
		{
			computedBs = (spaceFreePduRBuffer / MAX_PAYLOAD_SF_EXT_ADDR) + 1; 			 // + 1 is for local buffer.
		}
		else																			// for standard
		{
			computedBs = (spaceFreePduRBuffer / MAX_PAYLOAD_SF_STD_ADDR) + 1;  			// + 1 is for local buffer.
		}

		if (computedBs > rxConfig->CanTpBs)
		{
			computedBs = rxConfig->CanTpBs;
		}

		sduData[indexCount++] = computedBs;
		sduData[indexCount++] = (uint8) rxConfig->CanTpSTmin;
		rxRuntime->iso15765.nextFlowControlCount = computedBs;
		pduInfo.SduLength = indexCount;                           	  // indexcount here = 3
		break;
	}

	case BUFREQ_NOT_OK:
		break;

	case BUFREQ_BUSY:
		sduData[indexCount++] = ISO15765_TPCI_FC | ISO15765_FLOW_CONTROL_STATUS_WAIT;         // change the value of control flag (wait)
		indexCount +=2;
		pduInfo.SduLength = indexCount;								  // indexcount here = 3
		break;

	case BUFREQ_OVFL:
		sduData[indexCount++] = ISO15765_TPCI_FC | ISO15765_FLOW_CONTROL_STATUS_OVFLW;		  // change the value of control flag (overflow)
		indexCount +=2;
		pduInfo.SduLength = indexCount;								 // indexcount here = 3
		break;

	default:
		break;
	}


	ret = canReceivePaddingHelper(rxConfig, rxRuntime, &pduInfo);       // pduinfo contains pointer to data and data length

	if (ret != E_OK)
	{
		PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_NOT_OK);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}


/*This fuction copies Data from pdurBuffer to CanFrameBuffer */
BufReq_ReturnType sendNextTxFrame(const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime)
{
	BufReq_ReturnType ret = BUFREQ_OK;

	// copy data to temp buffer
	for(; txRuntime->canFrameBuffer.byteCount < MAX_SEGMENT_DATA_SIZE && ret == BUFREQ_OK ;)
	{
			/* al if de bthandle lw fe mshakel */
		if(txRuntime->pdurBuffer == NULL || txRuntime->pdurBufferCount == txRuntime->pdurBuffer->SduLength)   /* lw msh byshawer 3la data aw pdurBufferCount == SduLength */
		{
			// data empty, request new data
//			ret = PduR_CanTpProvideTxBuffer(txConfig->PduR_PduId, &txRuntime->pdurBuffer, 0);
			txRuntime->pdurBufferCount = 0;
			if(ret == BUFREQ_OK)
			{
				// new data received
//				VALIDATE( txRuntime->pdurBuffer->SduDataPtr != NULL,
//						SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TX_BUFFER );
			}
			else
			{
				// failed to receive new data
				txRuntime->pdurBuffer = NULL;
				break;
			}
		}

		 /* copying Data from pdurBuffer to canFrameBuffer and all in runtime */
		txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
				txRuntime->pdurBuffer->SduDataPtr[txRuntime->pdurBufferCount++];
		txRuntime->transferCount++;            /* increase the transfer count */

		if(txRuntime->transferCount == txRuntime->transferTotal)     /* check that we copy all Bytes in the PDU */
		 {
			// all bytes, send
			break;
		}
	}

/* here company handle Tx_Confirmation and we don't need this right Now
 *	if(ret == BUFREQ_OK)
 *	{
 * 		PduInfoType pduInfo;
 *		Std_ReturnType resp;
 *		pduInfo.SduDataPtr = txRuntime->canFrameBuffer.data;
 *		pduInfo.SduLength = txRuntime->canFrameBuffer.byteCount;
 *
 *		// change state to verify tx confirm within timeout
 *		txRuntime->iso15765.stateTimeoutCount = (txConfig->CanTpNas);
 *		txRuntime->iso15765.state = TX_WAIT_TX_CONFIRMATION;
 *		resp = canTansmitPaddingHelper(txConfig, txRuntime, &pduInfo);
 *		if(resp == E_OK) {
 *			// sending done
 *		} else {
 *			// failed to send
 *			ret = BUFREQ_NOT_OK;
 *		}
 *	}
 */
	return ret;
}





