#ifndef __BREADCRUMBS_H
#define __BREADCRUMBS_H

#define kBCImgLoadImageTooLarge		1
#define kBCImgLoadCountNotZero		2
#define kBCImgLoadCountIsZero		3

#define kBCImg4TooLarge			1
#define kBCImg4NullPtr			2
#define kBCImg4HeaderErr		3
#define kBCImg4Truncated		4
#define kBCImg4DecodeInitFail		5
#define kBCImg4ImageTypeMismatch	6
#define kBCImg4ManifestInvalid		7
#define kBCImg4TrustEvalFail		8
#define kBCImg4BootManifestFail		9
#define kBCImg4ProdStatusMismatch	10
#define kBCImg4SecureModeMismatch	11
#define kBCImg4PayloadDecodeFail	12
/* gonna skip this value given my luck with OTA changes */
#define kBCImg4PayloadDecryptFail	14


#define kBCImg3TooLarge			1
#define kBCImg3NullPtr			2
#define kBCImg3BDEVReadFail		3
#define kBCImg3BadMagic			4
#define kBCImg3UnknownPrivateMagic	5
#define kBCImg3InstantiationFailed	6
#define kBCImg3UnacceptedType		7
#define kBCImg3SignatureValidationFail	8
#define kBCImg3ConstraintValidationFail 9
#define kBCImg3GetTagStructFailed	10
#define kBCImg3DecryptFailed		11


#endif