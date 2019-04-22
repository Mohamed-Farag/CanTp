///*
// ============================================================================
// Name        : cantp.c
// Author      : CAN_TP_Team
// Version     :
// Copyright   : Your copyright notice
// Description : The CanTp Main Task is Segmentation and reassembling
// ============================================================================
// */
//
#include <stdio.h>
#include <stdlib.h>


#include "CanTp_Cfg.h"
#include "CanTp.h"
#include "CanTp_Types.h"
#include "CanTp_Cbk.h"
#include "SchM_CanTp.h"
#include "MemMap.h"
#include "Det.h"

/************************************************** Stup Functions ***********************************************************/

void PduR_CanTpRxIndication(PduIdType CanTpRxPduId,NotifResultType Result)
{
	;

}


BufReq_ReturnType PduR_CanTpProvideRxBuffer(PduIdType id, PduLengthType length,
		PduInfoType **PduInfoPtr)
{
	return BUFREQ_OK;
}




void CanTp_RxIndication(PduIdType CanTpRxPduId,const PduInfoType *CanTpRxPduPtr)
{

	const CanTp_RxNSduType *rxConfigParams;			 // Params reside in ROM.
	const CanTp_TxNSduType *txConfigParams;
	CanTp_AddressingFormatType addressingFormat; 	 // Configured
	CanTp_ChannelPrivateType *runtimeParams = NULL;  // Params reside in RAM.
	ISO15765FrameType frameType;
	PduIdType CanTpTxNSduId, CanTpRxNSduId;
	CanTpRxNSduId = CanTpRxPduId;
	uint16 TxIndex, RxIndex;
//	TxIndex = getindex();
	RxIndex = CanTpRxPduId;   /* Omar_Emad tells me that id = index */

	//Check if PduId is valid          /* we know that the value of id we recieve is already exist */
//	if (CanTpRxPduId >= CANTP_RXID_LIST_SIZE)
//	{
//		return;
//	}

	addressingFormat = CanTp_Config.CanTpChannel.CanTpRxNSdu[RxIndex].CanTpRxAddressingFormat;

	/* TODO John - Use a different indication of not set than 0xFFFF? */
	frameType = getFrameType(&addressingFormat, CanTpRxPduPtr);
//	 getFrameType(const CanTp_AddressingFormatType *formatType,const PduInfoType *CanTpRxPduPtr)



				/* in case of tx */

	if( frameType == FLOW_CONTROL_CTS_FRAME )
	{
		/* by check an al index mazboot */
		if( TxIndex != 0xFFFF )
		{
//	//		CanTpTxNSduId = CanTpConfig.CanTpRxIdList[CanTpRxPduId].CanTpReferringTxIndex;
//			//txConfigParams = &CanTpConfig.CanTpNSduList[CanTpTxNSduId].configData.CanTpTxNSdu;
//			txConfigParams = CanTp_Config.CanTpChannel.CanTpTxNSdu[CanTpTxNSduId];
//			runtimeParams = &CanTpRunTimeData.runtimeDataList[txConfigParams->CanTpTxChannel];
		}
		else
		{
			//Invalid FC received
			return;
		}
		rxConfigParams = NULL;
	}

		/* in case of rx */

	else   	 /*{SF,FF,CF}*/
	{
		if( RxIndex != 0xFFFF )
		{
		//	CanTpRxNSduId = CanTpConfig.CanTpRxIdList[CanTpRxPduId].CanTpNSduIndex; /*CanTpRxPDUID = CanTpRxSDUID  so we don't need this line */
			rxConfigParams = &CanTp_Config.CanTpChannel.CanTpRxNSdu[RxIndex];
			runtimeParams = &CanTpRunTimeData.runtimeDataList[rxConfigParams->CanTpRxChannel];	/* Question: Do we need CanTpRxChannel ??  */
		}
		else
		{
			//Invalid Frame received
			return;
		}
		txConfigParams = NULL;
	}



	switch (frameType)
	{
	case SINGLE_FRAME:
	{
		if (rxConfigParams != NULL)
		{
			handleSingleFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		}

		break;
	}

	case FIRST_FRAME:
	{
		if (rxConfigParams != NULL)
		{
			handleFirstFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		}
		break;
	}

	case CONSECUTIVE_FRAME:
	{
		if (rxConfigParams != NULL)
		{
			handleConsecutiveFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		}
		break;
	}

	case FLOW_CONTROL_CTS_FRAME:
	{
		if (txConfigParams != NULL)
		{
			handleFlowControlFrame(txConfigParams, runtimeParams, CanTpRxPduPtr);
		}
		break;
	}
	case INVALID_FRAME:
	{
		break;
	}

	default:
		break;
	}
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

/*************************************************** Helper Functions ************************************************************/
//ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime, data, pduLength, &bytesWrittenToSduRBuffer);

static BufReq_ReturnType copySegmentToPduRRxBuffer(   const CanTp_RxNSduType *rxConfig,
													CanTp_ChannelPrivateType *rxRuntime,
																	   uint8 *segment,         // data
																PduLengthType segmentSize,	   // pduLength
																PduLengthType *BytesWrittenSuccessfully)   // bytesWrittenToSduRBuffer
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
			/* startofreception hya hya PduR_CanTpProvideRxBuffer */ /* I should do mapping to id here */
			return_value = PduR_CanTpProvideRxBuffer(rxConfig->CanTpRxNPdu.CanTpRxNPduId, rxRuntime->transferTotal, &rxRuntime->pdurBuffer);

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

/* this function coping data and length to Rx_runtime.canFrameBufferData if segementsize < MAX_SEGMENT_DATA_SIZE and return True if copying Done */
static bool copySegmentToLocalRxBuffer(CanTp_ChannelPrivateType *rxRuntime, uint8 *segment,PduLengthType segmentSize)
{
	bool ret = FALSE;

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

/* This Fuction is used to get the length of the PDU from N_PCI */
static PduLengthType getPduLength(const CanTp_AddressingFormatType *formatType,const ISO15765FrameType iso15765Frame, const PduInfoType *CanTpRxPduPtr)
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

/* This Function get The Frame Type { SF,FF,CF,CTS_FC,WAIT_FC,OVERFLOW_FC } */

static ISO15765FrameType getFrameType(const CanTp_AddressingFormatType *formatType,const PduInfoType *CanTpRxPduPtr)
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


/* This function returns either it SF or FF */
static ISO15765FrameType calcRequiredProtocolFrameType(const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime)
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

static void handleFirstFrame(const CanTp_RxNSduType *rxConfig,CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData)
{
	BufReq_ReturnType ret;
	PduLengthType pduLength = 0;
	PduLengthType bytesWrittenToSduRBuffer;

	if (rxRuntime->iso15765.state != IDLE)
	{
		PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_NOT_OK); // Abort current reception, we need to tell the current receiver it has been aborted.
	}

	(void) initRx15765RuntimeData(rxRuntime);
	pduLength = getPduLength(&rxConfig->CanTpRxAddressingFormat, FIRST_FRAME,rxPduData);
	rxRuntime->transferTotal = pduLength;


	// Validate that that there is a reason for using the segmented transfers and
	// if not simply skip (single frame should have been used).
	if (rxConfig->CanTpRxAddressingFormat == CANTP_STANDARD)
	{
		if (pduLength <= MAX_PAYLOAD_SF_STD_ADDR){
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

	rxRuntime->iso15765.framesHandledCount = 1; // Segment count begins with 1 (FirstFrame has the 0).
	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr); /** @req CANTP166 */

	if (rxConfig->CanTpRxAddressingFormat == CANTP_STANDARD)
	{
		ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime,&rxPduData->SduDataPtr[2],MAX_PAYLOAD_FF_STD_ADDR,&bytesWrittenToSduRBuffer);
	}
	else
	{
		ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime,&rxPduData->SduDataPtr[3],MAX_PAYLOAD_FF_EXT_ADDR,&bytesWrittenToSduRBuffer);
	}
	if (ret == BUFREQ_OK) {
		rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNcr);
		rxRuntime->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret);
	}
	else if (ret == BUFREQ_BUSY)
	{
		if (rxConfig->CanTpRxAddressingFormat == CANTP_STANDARD) {
			(void)copySegmentToLocalRxBuffer(rxRuntime, &rxPduData->SduDataPtr[2], MAX_PAYLOAD_FF_STD_ADDR );
		}
		else
		{
			(void)copySegmentToLocalRxBuffer(rxRuntime, &rxPduData->SduDataPtr[3], MAX_PAYLOAD_FF_EXT_ADDR );
		}
		rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr);
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


static void handleSingleFrame(const CanTp_RxNSduType *rxConfig,CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData)
{
	BufReq_ReturnType ret;
	PduLengthType pduLength;
	uint8 *data = NULL;                       /* will pointer to points to N_PCI  */
	PduLengthType bytesWrittenToSduRBuffer;


	if (rxRuntime->iso15765.state != IDLE)
	{
		PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_NOT_OK);  // Abort current reception, we need to tell the current receiver it has been aborted.
	}

	(void) initRx15765RuntimeData(rxRuntime);
	pduLength = getPduLength(&rxConfig->CanTpRxAddressingFormat, SINGLE_FRAME, rxPduData);



	if (rxConfig->CanTpRxAddressingFormat == CANTP_STANDARD)
	{
		data = &rxPduData->SduDataPtr[1];
	}
	else								/* in case of Extended Addressing format */
	{
		data = &rxPduData->SduDataPtr[2];
	}

	rxRuntime->transferTotal = pduLength;
	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr);

	ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime, data, pduLength, &bytesWrittenToSduRBuffer);

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
		(void)copySegmentToLocalRxBuffer(rxRuntime, data, pduLength ); 	   // copy data to local buffer in case of BUFREQ_BUSY
		rxRuntime->iso15765.state = RX_WAIT_SDU_BUFFER;
		rxRuntime->mode = CANTP_RX_PROCESSING;
	}
	else
	{
		PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_NO_BUFFER);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}


static void handleFlowControlFrame(const CanTp_TxNSduType *txConfig,CanTp_ChannelPrivateType *txRuntime, const PduInfoType *txPduData)
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
		txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNcs);
		txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
		break;

		case ISO15765_FLOW_CONTROL_STATUS_WAIT:
			txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNbs);  /*CanTp: 264*/
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


static void handleConsecutiveFrame(const CanTp_RxNSduType *rxConfig,CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData)
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

	if (rxRuntime->iso15765.state == RX_WAIT_CONSECUTIVE_FRAME)
	{
		if (rxConfig->CanTpRxAddressingFormat == CANTP_EXTENDED)
		{
			extendedAddress = rxPduData->SduDataPtr[indexCount++];
		}

		// getting consecutive frame number
		segmentNumber = rxPduData->SduDataPtr[indexCount++] & SEGMENT_NUMBER_MASK;      //segment number equals consecutive frame index (3..0 bits in first byte)

		// checking if this consecutive frame is handled
		if (segmentNumber != (rxRuntime->iso15765.framesHandledCount & SEGMENT_NUMBER_MASK)) {
			PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_WRONG_SN);
			rxRuntime->iso15765.state = IDLE;
			rxRuntime->mode = CANTP_RX_WAIT;
		}
		else
		{
			currentSegmentMaxSize = MAX_SEGMENT_DATA_SIZE - indexCount;                  // 6 bytes in extended add. & 7 bytes in standard add.
			bytesLeftToCopy = rxRuntime->transferTotal - rxRuntime->transferCount;

			// determining segment size
			if (bytesLeftToCopy < currentSegmentMaxSize) {
				currentSegmentSize = bytesLeftToCopy; // 1-5.
			} else {
				currentSegmentSize = currentSegmentMaxSize; // 6 or 7, depends on addressing format used.
			}
			// Copy received data to buffer provided by SDUR.
			ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime,&rxPduData->SduDataPtr[indexCount],currentSegmentSize, &bytesCopiedToPdurRxBuffer);

			if (ret == BUFREQ_NOT_OK) {
				PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_NO_BUFFER);
				rxRuntime->iso15765.state = IDLE;
				rxRuntime->mode = CANTP_RX_WAIT;
			}
			else if (ret == BUFREQ_BUSY)
			{
				bool dataCopyFailure = FALSE;
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
					rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr);
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

					if (rxRuntime->iso15765.nextFlowControlCount == 0  && rxRuntime->iso15765.BS > 0) {
						sendFlowControlFrame(rxConfig, rxRuntime, BUFREQ_OK);
					}
					else
					{
						rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNcr);  //UH
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

static void sendFlowControlFrame(const CanTp_RxNSduType *rxConfig, CanTp_ChannelPrivateType *rxRuntime, BufReq_ReturnType flowStatus)
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

		spaceFreePduRBuffer = rxRuntime->pdurBuffer->SduLength - rxRuntime->pdurBufferCount;



		if (rxConfig->CanTpRxAddressingFormat == CANTP_EXTENDED)                         // for Extended
		{
			computedBs = (spaceFreePduRBuffer / MAX_PAYLOAD_SF_EXT_ADDR) + 1;  // + 1 is for local buffer.
		}
		else																			// for standard
		{
			computedBs = (spaceFreePduRBuffer / MAX_PAYLOAD_SF_STD_ADDR) + 1;  // + 1 is for local buffer.
		}

		if (computedBs > rxConfig->CanTpBs)
		{
			computedBs = rxConfig->CanTpBs;
		}

		sduData[indexCount++] = computedBs; 							// 734 PC-lint: Okej att casta till uint8?
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


//	ret = canReceivePaddingHelper(rxConfig, rxRuntime, &pduInfo);       // pduinfo contains pointer to data and data length

	if (ret != E_OK)
	{
		PduR_CanTpRxIndication(rxConfig->CanTpRxNPdu.CanTpRxNPduId, NTFRSLT_E_NOT_OK);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}


/*This fuction copies Data from pdurBuffer to CanFrameBuffer */
static BufReq_ReturnType sendNextTxFrame(const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime)
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
 *		txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNas);
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



/****************************************************** Primitive Functions *********************************************************/

/* This Function initate the global parameters of the CanTp Module and move the state to CanTp_ON if there is No Error */
void CanTp_Init( const CanTp_ConfigType* CfgPtr )
{


		CanTp_ChannelPrivateType *runtimeData;
		//uint8 TxChannel;
		//uint8 RxChannel;


		uint8 i;
		for (i=0; i < CANTP_NSDU_CONFIG_LIST_SIZE; i++)
			{



				if ( CfgPtr->CanTpChannel.direction == IS015765_TRANSMIT )
				{
										/* For Tx */

					/* this if handle if CanTpTxChannel < Runtime_list_size
					 *then access the  CanTpTxChannel element in the runtimeDataList
					 * else access the last item in the runtimeDataList
					 */
							if (CfgPtr->CanTpChannel.CanTpTxNSdu[i].CanTpTxChannel < CANTP_NSDU_RUNTIME_LIST_SIZE)
							{
								runtimeData = &CanTpRunTimeData.runtimeDataList[CfgPtr->CanTpChannel.CanTpTxNSdu[i].CanTpTxChannel];
							}
							 else
							{
								runtimeData = &CanTpRunTimeData.runtimeDataList[CANTP_NSDU_RUNTIME_LIST_SIZE-1];
							}

							initTx15765RuntimeData( runtimeData );

				}

				else
				{
										/* For Rx */
					/* this if handle if CanTpTxChannel < Runtime_list_size
					 *then access the  CanTpTxChannel element in the runtimeDataList
					 * else access the last item in the runtimeDataList
					 */

							if (CfgPtr->CanTpChannel.CanTpRxNSdu[i].CanTpRxChannel < CANTP_NSDU_RUNTIME_LIST_SIZE)
							{
								runtimeData = &CanTpRunTimeData.runtimeDataList[CfgPtr->CanTpChannel.CanTpRxNSdu[i].CanTpRxChannel];
							}
							else
							{
								runtimeData = &CanTpRunTimeData.runtimeDataList[CANTP_NSDU_RUNTIME_LIST_SIZE-1];
							}
							initRx15765RuntimeData( runtimeData );

				}
			}

		CanTpRunTimeData.internalState = CANTP_ON;    /* if the initfunc finished correcltly without errors ,then move into CANTP_ON state */

}

//Std_ReturnType CanTp_Transmit( PduIdType TxPduId, const PduInfoType* PduInfoPtr )      		 // our sws
//Std_ReturnType CanTp_Transmit( PduIdType CanTpTxSduId, const PduInfoType* PduInfoPtr )  		 // their sws


/* this function used to copy the Data Length of the data required to be sent from  PDURBuffer to the CanIF Buffer in SF or SF */
Std_ReturnType CanTp_Transmit( PduIdType TxPduId, const PduInfoType* PduInfoPtr )
{
	const CanTp_TxNSduType *txConfig = NULL;
	CanTp_ChannelPrivateType *txRuntime = NULL;
	Std_ReturnType ret = 0;
	PduIdType CanTp_InternalTxNSduId;							/* Unused Variable Now */

	/*Here we should make a fuction get the Txid and return the correct index in the cfg.c*/

		txConfig	=  &CanTp_Config.CanTpChannel.CanTpTxNSdu[TxPduId];        		             // habd mny :D

//		CanTp_InternalTxNSduId = CanTpConfig.CanTpRxIdList[CanTpTxSduId].CanTpNSduIndex;    	 // lsa hnshofha bokra

//		txConfig =&CanTpConfig.CanTpNSduList[CanTp_InternalTxNSduId].configData.CanTpTxNSdu;  	 // lsa hnshofha bokra

		txRuntime = &CanTpRunTimeData.runtimeDataList[txConfig->CanTpTxChannel]; 				 // Runtime data.

		if (txRuntime->iso15765.state == IDLE)
		{

			ISO15765FrameType iso15765Frame;
			txRuntime->canFrameBuffer.byteCount = 0;
			txRuntime->pdurBuffer = NULL;                   // Farag change this from 0 to NULL
			txRuntime->transferCount = 0;
			txRuntime->iso15765.framesHandledCount = 0;
			txRuntime->transferTotal = PduInfoPtr->SduLength;     /* copy SduLength to transferTotal */
//			txRuntime->pdurBuffer = PduInfoPtr->SduDataPtr
			txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNcs);
			txRuntime->mode = CANTP_TX_PROCESSING;

			iso15765Frame = calcRequiredProtocolFrameType(txConfig, txRuntime);

			if (txConfig->CanTpTxAddressingFormat == CANTP_EXTENDED)
			{
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
					 txConfig->CanTpNTa.CanTpNTa;					// putting CanTpNTa in the first byte of data in case of extended 		 // Target address.
			}
			switch(iso15765Frame)
			{
			/* This only put the Data length in the correct position in pci */

			/*TODO: We Will support id Can_DL > 8 */
			case SINGLE_FRAME:
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						ISO15765_TPCI_SF | (uint8)(txRuntime->transferTotal);   /* Can_DL < 8 so it will represent in 8 bits ( putting the SduLength at byte 0 )*/

				/*	Note that :	PduLengthType  is  16 bit */

				ret = E_OK;
				txRuntime->iso15765.state = TX_WAIT_TRANSMIT;   /* Activate a TP Task */
				break;


			case FIRST_FRAME:
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						ISO15765_TPCI_FF | (uint8)((txRuntime->transferTotal & 0xf00) >> 8);  /* putting the  Byte 1  */


				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						(uint8)(txRuntime->transferTotal & 0xff);						      /* putting the  Byte 2  */

				// setup block size so that state machine waits for flow control after first frame
				txRuntime->iso15765.nextFlowControlCount = 1;
				txRuntime->iso15765.BS = 1;						/* setting the block size and we can also set STmin  */
				ret = E_OK;
				txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
				break;

			default:
				ret = E_NOT_OK;
			}
		}
		else							/* if state not idle make  ret = E_NOT_OK */
		{
			ret = E_NOT_OK;
		}

	return ret; 		// CAN level error code.
}


/* This Function move the state to CanTp_OFF*/
void CanTp_Shutdown(void)
{

	CanTpRunTimeData.internalState = CANTP_OFF;

}


void CanTp_MainFunction(void)
{
	BufReq_ReturnType ret;         		    /* enum = {BUFREQ_OK ,BUFREQ_NOT_OK,BUFREQ_BUSY,BUFREQ_OVFL }*/

	PduLengthType bytesWrittenToSduRBuffer;

	CanTp_ChannelPrivateType *txRuntimeListItem = NULL;
	CanTp_ChannelPrivateType *rxRuntimeListItem = NULL;

	const CanTp_TxNSduType *txConfigListItem = NULL;
	const CanTp_RxNSduType *rxConfigListItem = NULL;


	uint8 i = 0;
	for( i=0; i < CANTP_NSDU_CONFIG_LIST_SIZE; i++ )
	{
				/* in case of TX */

		if ( CanTp_Config.CanTpChannel.direction == IS015765_TRANSMIT )
		{
			txConfigListItem = (CanTp_TxNSduType*)&CanTp_Config.CanTpChannel.CanTpTxNSdu[i];
			txRuntimeListItem = &CanTpRunTimeData.runtimeDataList[txConfigListItem->CanTpTxChannel];

			switch (txRuntimeListItem->iso15765.state)
			{

			case TX_WAIT_STMIN:
				TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount); 		// Make sure that STmin timer has expired.
				if (txRuntimeListItem->iso15765.stateTimeoutCount != 0)
				{
					break;
				}

				txRuntimeListItem->iso15765.state = TX_WAIT_TRANSMIT;


			case TX_WAIT_TRANSMIT:
			{
				ret = sendNextTxFrame(txConfigListItem, txRuntimeListItem);

				if ( ret == BUFREQ_OK )
				{
					// successfully sent frame
				}

				else if( ret == BUFREQ_BUSY )
				{
					// check N_Cs timeout
					TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
					if (txRuntimeListItem->iso15765.stateTimeoutCount == 0)
					{
						txRuntimeListItem->iso15765.state = IDLE;
						txRuntimeListItem->mode = CANTP_TX_WAIT;
						//PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);    			 // it is not important in the core Now
					}
					else
					{
						// For MISRA Rules only as i remember
					}
				}

				else
				{
					txRuntimeListItem->iso15765.state = IDLE;
					txRuntimeListItem->mode = CANTP_TX_WAIT;
					//PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); 				 // it is not important in the core Now
				}
				break;
			}

			case TX_WAIT_FLOW_CONTROL:

				TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
				if (txRuntimeListItem->iso15765.stateTimeoutCount == 0)
				{
					txRuntimeListItem->iso15765.state = IDLE;
					txRuntimeListItem->mode = CANTP_TX_WAIT;
					//PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); 			    // it is not important in the core Now
				}
				break;

			case TX_WAIT_TX_CONFIRMATION:
				TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
				if (txRuntimeListItem->iso15765.stateTimeoutCount == 0)
				{
					txRuntimeListItem->iso15765.state = IDLE;
					txRuntimeListItem->mode = CANTP_TX_WAIT;
					//PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);
				}
				break;

			default:
				break;

			}
		}



			/* in case of RX */

		else
		{
			rxConfigListItem =(CanTp_RxNSduType*)&CanTp_Config.CanTpChannel.CanTpRxNSdu[i];
			rxRuntimeListItem = &CanTpRunTimeData.runtimeDataList [ rxConfigListItem->CanTpRxChannel ];

			switch (rxRuntimeListItem->iso15765.state)
			{
			case RX_WAIT_CONSECUTIVE_FRAME:
			{
				TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);

				if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0)
				{
					//PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);
					rxRuntimeListItem->iso15765.state = IDLE;
					rxRuntimeListItem->mode = CANTP_RX_WAIT;
				}
				break;
			}

			case RX_WAIT_SDU_BUFFER:
			{
				TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);
				/* We end up here if we have requested a buffer from the
				 * PDUR but the response have been BUSY. We assume
				 * we have data in our local buffer and we are expected
				 * to send a flow-control clear to send (CTS).
				 */
				if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0)
				{
					//PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);			  /* CanTp_00214 */
					rxRuntimeListItem->iso15765.state = IDLE;
					rxRuntimeListItem->mode = CANTP_RX_WAIT;
				}


				else			//	stateTimeoutCount != 0
				{
					PduLengthType bytesRemaining = 0;

					 /* copies from local buffer to PDUR buffer. */
					ret = copySegmentToPduRRxBuffer(rxConfigListItem,rxRuntimeListItem,rxRuntimeListItem->canFrameBuffer.data,rxRuntimeListItem->canFrameBuffer.byteCount
							,&bytesWrittenToSduRBuffer);
					bytesRemaining = rxRuntimeListItem->transferTotal -  rxRuntimeListItem->transferCount;
					if (bytesRemaining > 0)
					{
						sendFlowControlFrame( rxConfigListItem, rxRuntimeListItem, ret ); 			/* (Busy or CTS) */
					}

					if (ret == BUFREQ_OK)
					{
						if ( bytesRemaining > 0 )
						{
							rxRuntimeListItem->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfigListItem->CanTpNcr);  //UH
							rxRuntimeListItem->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
						}
						else
						{
							//PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_OK);			  /* CanTp_00214 */
							rxRuntimeListItem->iso15765.state = IDLE;
							rxRuntimeListItem->mode = CANTP_RX_WAIT;
						}
					}
					else if (ret == BUFREQ_NOT_OK )
					{
						//PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);        /* CanTp_00214 */
						rxRuntimeListItem->iso15765.state = IDLE;
						rxRuntimeListItem->mode = CANTP_RX_WAIT;
					}
					else if ( ret == BUFREQ_BUSY )
					{

					}
				}
				break;
			}
			default:
				break;
			}
		}
	}
}

/****************************************************** int main *********************************************************/


int main()
{
	//uint8 Array[2] = {'S','A'};
	PduInfoType test;
	test.SduDataPtr[0] = 0x06;
	test.SduDataPtr[1] = 'S';
	test.SduDataPtr[2] = 'A';
	test.SduDataPtr[3] = 'L';
	test.SduDataPtr[4] = 'M';
	test.SduDataPtr[5] = 'A';


	test.SduLength=6;
	CanTp_Init(&CanTp_Config);
//	CanTp_Transmit(1, &test );

	CanTp_RxIndication(1,&test);

	return EXIT_SUCCESS;

}

//int main(void) {
//	int x = 10;
//	int z= x+ 1;
//	puts("Mohamed");
//	printf("%d\n",x);
//
//	printf("%d",z);
//
//
//
//	return EXIT_SUCCESS;
//}
