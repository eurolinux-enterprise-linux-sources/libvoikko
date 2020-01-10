/* Copyright (C) 1995 Bjoern Beutel.
 *               2009 Harri Pitkänen <hatapitk@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *********************************************************************************/

/* Description. =============================================================*/

/* Operations for files and file names. */

namespace libvoikko { namespace morphology { namespace malaga {

/* File operations. =========================================================*/

extern FILE *open_stream( string_t file_name, string_t stream_mode );
/* Open file FILE_NAME and create a stream from/to it in mode STREAM_MODE.
 * Works like "fopen", but calls "error" if it doesn't work. */

extern void close_stream(FILE **stream_p);
/* Close the stream *STREAM_P
 * and set *STREAM_P to NULL. Don't do anything if *STREAM_P == NULL.
 */

extern void read_vector( void *address, 
                         int_t item_size, 
                         int_t item_count, 
                         FILE *stream);
/* Read ITEM_COUNT items, of size ITEM_SIZE each, from STREAM,
 * and store them at *ADDRESS.
 */

extern void *read_new_vector( int_t item_size, 
                              int_t item_count, 
                              FILE *stream);
/* Read ITEM_COUNT items, of size ITEM_SIZE each, from STREAM,
 * into allocated memory block,
 * and return a pointer to that block. */

extern void map_file( string_t file_name, void **address, int_t *length );
/* Map file "file_name" into the memory. It will be available in the 
 * memory region starting at *ADDRESS and will occupy LENGTH bytes.
 * After usage, return the memory region via "unmap_file". */

extern void unmap_file( void **address, int_t length );
/* Return the memory region that has been allocated by "map_file".
 * The region starts at *ADDRESS and occupies LENGTH bytes. */

/* File name operations. ====================================================*/

extern char_t *absolute_path( string_t src_path, string_t relative_to );
/* Return the absolute path name which is equivalent to SRC_PATH.
 * If SRC_PATH starts with "~", it's replaced by the home directory of the
 * user whose login name is following (current user if no login name).
 * If RELATIVE_TO is not NULL, SRC_NAME is relative to that path name.
 * RELATIVE_TO must be an absolute path name (a directory or a file).
 * The returned path must be freed after use. */

}}}
