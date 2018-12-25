/****************************************************
 *  ErrorCode.h                                         
 *  Created on: 16-May-2017 6:33:11 PM                      
 *  Implementation of the Class ErrorCode       
 *  Original author: Positivo                     
 ****************************************************/

#if !defined(EA_824C3A98_00AF_4dad_B205_5951402DE9ED__INCLUDED_)
#define EA_824C3A98_00AF_4dad_B205_5951402DE9ED__INCLUDED_


#ifdef __cplusplus
extern "C" {
#endif

typedef enum ErrorCode_t
{
	CUBEE_ERROR_OK = 0,
	CUBEE_ERROR_INVALID_PARAMETER = 1,
	CUBEE_ERROR_NOT_INITIALIZED = 2,
	CUBEE_ERROR_TIMEOUT = 2,

	CUBEE_ERROR_UNDEFINED,
}CubeeErrorCode_t;


#ifdef __cplusplus
}
#endif


#endif /*!defined(EA_824C3A98_00AF_4dad_B205_5951402DE9ED__INCLUDED_)*/
 
