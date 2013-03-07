/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2013  David Brooks
 *
 *****************************************************/
/** @file
  * Use libcurl to transfer files in and out of a BioSignalML repository.
  */

#ifndef _BSML_TRANSFER_H
#define _BSML_TRANSFER_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Get a recording from a repository.
 *
 * @param uri    The URI of the recording in the repository.  Its location component
                 identifies the repository.
 * @param fname  The name of the file in which to store the recording.
 * @param type   The expected file type (e.g. "edf"). Pass NULL to disable
 *               type checking.
 * @param key    A token, issued by the repository, for authorisation, Pass
                 NULL for guest access.
 * @param error  The address of a pointer that is set to an error string if errors
                 are found during transfer, or NULL on success. free() should be
                 called on this pointer when it is no longer needed.
 * @return       Non-zero to indicate success; zero on failure.
 */ 
int bsml_to_file(const char *uri, const char *fname, const char *type, const char *key, char **error) ;

/*!
 * Put a recording into a repository.
 *
 * @param fname  The name of the file to send to the repository.
 * @param uri    The URI to allocate for the recording. Its location component
                 identifies the repository.
 * @param type   The expected file type (e.g. "edf"). Pass NULL to disable
 *               type checking.
 * @param key    A token, issued by the repository, for authorisation, Pass
                 NULL for guest access.
 * @param error  The address of a pointer that is set to an error string if errors
                 are found during transfer, or NULL on success. free() should be
                 called on this pointer when it is no longer needed.
 * @return       Non-zero to indicate success; zero on failure.
 */ 
int file_to_bsml(const char *fname, const char *uri, const char *type, const char *key, char **error) ;

#ifdef __cplusplus
  } ;
#endif

#endif
