#ifndef FS_STREAM_H
#define FS_STREAM_H

#include <fs/fs.h>

/**
 * \brief Creates a new stream, and returns the file representation
 * 
 * @param length Length of stream buffer
 * 
 * @return Pointer to kfile representing the stream
 */
struct kfile *stream_create(int length);

#endif