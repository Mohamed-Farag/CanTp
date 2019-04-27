
/*
 * CanTp_Cfg.c
 *
 *  Created on: Mar 13, 2019
 *      Author: mohos
 */



#include "CanTp_Types.h"




 CanTp_NTaType  CanTp_NTa =
{
					 .CanTpNTa = 0,

};

 CanTp_NSaType  CanTp_NSa =
{
				     .CanTpNSa = 0,
};


 CanTp_NAeType  CanTpNAe =
{
					 .CanTpNAe = 0,

};


 CanTp_TxNPduType  CanTp_TxNPdu =
{
					.CanTpTxNPduConfirmationPduId = 0,
					// uint32 CanTpTxNPduRef;          // pointer
};


 const CanTp_TxFcNPduType  CanTp_TxFcNPdu =
{
					.CanTpTxFcNPduConfirmationPduId = 0,
					// uint16 CanTpRxFcNPduRef;  // pointer
};


 CanTp_RxFcNPduType  CanTp_RxFcNPdu =
{
		.CanTpRxFcNPduId = 0,
		// uint16 CanTpRxFcNPduRef;  // pointer

};




const CanTp_TxNSduType CanTp_TxNSdu[] =
{
		{
					.CanTpNas 			      = 				0,
					.CanTpNbs 				  = 				0,
					.CanTpNcs 				  = 				0,
					.CanTpTc			      = 				FALSE,
					.CanTpTxAddressingFormat  =  			    CANTP_EXTENDED,
					.CanTpTxNSduId 			  =					0,
					.CanTpTxPaddingActivation = 				CANTP_ON,
					.CanTpTxTaType 			  = 				CANTP_PHYSICAL,
					.CanTpTxChannel           =                 2,
					//CanTpTxNSduRef;       //pointer
					.CanTpTxNPdu 			  =					{
					             			                    .CanTpTxNPduConfirmationPduId = 0,
					             			                    // uint32 CanTpTxNPduRef;          // pointer
					             			},
			//		.CanTpRxFcNPdu			  =				 	CanTp_RxFcNPdu,
					.CanTpNTa				  = 				{
					         				                     .CanTpNTa = 0,

					         				},
					.CanTpNSa				  =					{
					         				                     .CanTpNSa = 0,
					         				},
					.CanTpNAe				  =			        {
					         				                     .CanTpNAe = 0,

					         				},
		},

		{
					.CanTpNas				  =					 0,
					.CanTpNbs 				  =					 0,
					.CanTpNcs 				  =					 0,
					.CanTpTc				  =					 TRUE,
					.CanTpNAe 			      = 				 {
					          			                        .CanTpNAe = 0,

					          			   },
					.CanTpTxAddressingFormat  =  				 CANTP_STANDARD,
					.CanTpTxNSduId            =					 1,
					.CanTpTxPaddingActivation =  				 CANTP_OFF,
					.CanTpTxTaType 			  =					 CANTP_PHYSICAL,
					.CanTpTxChannel            =                 3,
					//CanTpTxNSduRef;         //pointer
					.CanTpTxNPdu 			  = 			     {
					             			                     .CanTpTxNPduConfirmationPduId = 0,
					             			                     // uint32 CanTpTxNPduRef;          // pointer
					             			 },
			//		.CanTpRxFcNPdu 		      =				     CanTp_TxFcNPdu,
					.CanTpNTa 		          = 			     {
					          		                            .CanTpNTa = 0,

					          		       },
					.CanTpNSa			      = 			     {
					         			                         .CanTpNSa = 0,
					         			    },
		},

};




const CanTp_RxNSduType CanTp_RxNSdu[] =
{
		{
				//.CanTpRxNSdu				 = 				 0,    // why is this written here ??? do you have any idea ??
				.CanTpBs					 =				 0,
				.CanTpNar					 = 				1,
				.CanTpNbr					 = 				2,
				.CanTpNcr 					 = 				3,
				.CanTpRxAddressingFormat 	 = 				CANTP_EXTENDED,    // THIS IS AN ENUMERATOR IT CAN HAVE ONE	OF THE FOLLOWING VALUES {CANTP_EXTENDED , CANTP_MIXED , CANTP_MIXED29BIT , CANTP_NORMALFIXED ,  CANTP_STANDARD}
				.CanTpRxNSduId 				 =			 	0,
				.CanTpRxPaddingActivation 	 =				CANTP_OFF,		 // THIS IS AN ENUMERATOR IT AN HAVE ONEOF THE FOLLOWING VALUES {CANTP_OFF, CANTP_ON}
				.CanTpRxChannel              =              0,
				.CanTpRxTaType				 =				CANTP_PHYSICAL,            // you can put either one of these values on the ENUM {CANTP_FUNCTIONAL, CANTP_PHYSICAL}
				.CanTpRxWftMax				 = 				5,
				.CanTpSTmin					 = 				TRUE,
				// .CanTpRxNSduRef 			 = 				TRUE,
		//		.CanTpRxNPdu 				 =				CanTp_RxNPdu,
				.CanTpTxFcNPdu				 =				{
				              			                    .CanTpTxFcNPduConfirmationPduId = 0,
				              			                    // uint16 CanTpRxFcNPduRef;  // pointer
				              			},
				.CanTpNTa 					 = 				{
				          				                     .CanTpNTa = 0,

				          				},
				.CanTpNSa					 = 				{
				         				                     .CanTpNSa = 0,
				         				},
				.CanTpNAe					 = 				{
				         				                     .CanTpNAe = 0,

				         				},

		},

		{
				//.CanTpRxNSdu 				=				 0,    // why is this written here ??? do you have any idea ??
				.CanTpBs 					= 				 0,
				.CanTpNar 					=				3,
				.CanTpNbr				    = 				4,
				.CanTpNcr					= 				6,
				.CanTpRxAddressingFormat    =  				CANTP_STANDARD,    // THIS IS AN ENUMERATOR IT CAN HAVE ONE	OF THE FOLLOWING VALUES {CANTP_EXTENDED , CANTP_MIXED , CANTP_MIXED29BIT , CANTP_NORMALFIXED ,  CANTP_STANDARD}
				.CanTpRxNSduId 				= 				1,
				.CanTpRxPaddingActivation   = 				CANTP_ON,		 // THIS IS AN ENUMERATOR IT AN HAVE ONEOF THE FOLLOWING VALUES {CANTP_OFF, CANTP_ON}
				.CanTpRxChannel             =               1,
				.CanTpRxTaType				= 				CANTP_PHYSICAL,            // you can put either one of these values on the ENUM {CANTP_FUNCTIONAL, CANTP_PHYSICAL}
				.CanTpRxWftMax				= 				5,
				.CanTpSTmin 				= 				TRUE,
				// .CanTpRxNSduRef 			=				TRUE,
		//		.CanTpRxNPdu				=				CanTp_TxNPdu,
				.CanTpTxFcNPdu 				=
				{
                    .CanTpTxFcNPduConfirmationPduId = 0,
                                                        // uint16 CanTpRxFcNPduRef;  // pointer
				 },
				.CanTpNTa 					= 				{
				          				                     .CanTpNTa = 0,

				          				},
				.CanTpNSa 					= 				{
				          				                     .CanTpNSa = 0,
				          				},
				.CanTpNAe 					= 				{
				          				                     .CanTpNAe = 0,

				          				}

		}

};
















CanTp_ChannelType  CanTp_Channel =
{
				.CanTpChannelMode		 =			 CANTP_MODE_FULL_DUPLEX,  // it can also take this value (CANTP_MODE_HALF_DUPLEX)
				.direction               =             ISO15765_RECEIVE,
				.CanTpRxNSdu			 = 				 CanTp_RxNSdu,
				.CanTpTxNSdu			 = 				 CanTp_TxNSdu,
};









CanTp_GeneralType CanTp_General =
{
			 .CanTpChangeParameterApi        = 			    FALSE,
			 .CanTpDevErrorDetect            = 				FALSE,
			 .CanTpDynIdSupport 		     = 				FALSE,
			 .CanTpFlexibleDataRateSupport   =		        TRUE,
			 .CanTpGenericConnectionSupport  = 	        	FALSE,
			 .CanTpPaddingByte               = 				10,
			 .CanTpReadParameterApi          = 				FALSE,
			 .CanTpVersionInfoApi            = 				FALSE

};


/*This is only for Testing */

const CanTp_ConfigType CanTp_Config =
{
			.CanTpMainFunctionPeriod 		= 					5,
			.CanTpMaxChannelCnt     		= 					5,
			.direction                       =                   1,
			.CanTpChannel             		=
			{
			     .CanTpChannelMode        =           CANTP_MODE_FULL_DUPLEX,  // it can also take this value (CANTP_MODE_HALF_DUPLEX)
			     .direction               =             ISO15765_RECEIVE,
			     .CanTpRxNSdu             =               CanTp_RxNSdu,
			     .CanTpTxNSdu             =               CanTp_TxNSdu,
			}

};
// - - - - - - - - - - - - - -



//
//CanTp_Type CanTp =
//{
//		.CanTpConfig   = 	  &CanTp_Config ,
//		.CanTp_General =      &CanTp_General,
//
//};
//

