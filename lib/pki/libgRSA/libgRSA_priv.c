/* Copyright (c) 2005-2007 Apple Inc.  All Rights Reserved. */

/*
 * libgRSA_priv.c - private libRSA routines.
 *
 * Created Aug. 10 2005 by Doug Mitchell.
 */

#include <libgRSA/libgRSA_priv.h>

/* convert a libGiants GIReturn to our own RSAStatus */
RSAStatus giantStatusToRSA(GIReturn grtn)
{
	switch(grtn) {
		case GR_Success:		return RSA_Success;
		case GR_IllegalArg:		return RSA_ParamErr;
		case GR_Unimplemented:	return RSA_Unimplemented;
		case GR_Internal:		return RSA_InternalError;
		case GR_Overflow:		return RSA_Overflow;
		default:				return RSA_OtherError;
	}
}

/* convert a libDER DERReturn to our own RSAStatus */
RSAStatus derStatusToRSA(DERReturn drtn)
{
	switch(drtn) {
		case DR_Success:		return RSA_Success;
		case DR_ParamErr:		return RSA_ParamErr;
		case DR_BufOverflow:	return RSA_Overflow;
		case DR_Unimplemented:	return RSA_Unimplemented;
		default:				return RSA_BadKeyFormat;
	}
}

#ifndef	NDEBUG

#include <libGiantsUtils/GIReturnString.h>

/* be sure to update the mirrored copy in libgRSAUtils when this changes */
const char *rsaStatusStr(RSAStatus rrtn)
{
	static char unknownStr[100];
	
	if(rrtn < RSA_RETURN_BASE) {
		if(rrtn == RSA_Success) {
			return "RSA_Success";
		}
		else {
			/* libGiants error */
			return GIReturnString((GIReturn)rrtn);
		}
	}
	switch(rrtn) {
		case RSA_BadKeyData:
			return "RSA_BadKeyData";
		case RSA_ParamErr:
			return "RSA_ParamErr";
		case RSA_IncompleteKey:
			return "RSA_IncompleteKey";
		case RSA_VerifyFail:
			return "RSA_VerifyFail";
		case RSA_BadSigFormat:
			return "RSA_BadSigFormat";
		case RSA_BadDataFormat:
			return "RSA_BadDataFormat";
		case RSA_BadKeyFormat:
			return "RSA_BadKeyFormat";
		case RSA_InternalError:
			return "RSA_InternalError";
		case RSA_Overflow:
			return "RSA_Overflow";
		case RSA_Unimplemented:
			return "RSA_Unimplemented";
		case RSA_IllegalKeySize:
			return "RSA_IllegalKeySize";
		case RSA_OtherError:
			return "RSA_OtherError";
		default:
			sprintf(unknownStr, "Unknown (%d)", rrtn);
			return unknownStr;
	}
}


#endif	/* NDEBUG */
