/*
 * Copyright 2013 David Jackson
 *
 * This file is part of Basenc
 *
 * Basenc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Basenc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Basenc.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * \file base32.h
 * \brief A header for encoding and decoding data to/from Base32
 */

#ifndef BASE32_H
#define BASE32_H

#include <string.h>
#include <assert.h>

#define BASE32_PADDING_CHAR '='

/**
 * \var typedef unsigned char base32_byte
 * \brief A type definition for a generic byte
 *
 * The base32_byte type is an unsigned character because it has to be one byte
 * long and the sign bit is always irrelevant.
 */
typedef unsigned char base32_byte;

/**
 * Base32 Encode Block
 *
 * Given a bytestring with a length of exactly 5 bytes, return a character
 * string with a length of 8 bytes that is the Base32-encoded version of that
 * initial bytestring of data. It is the programmer's responsibility to ensure
 * that the "encoded" string has enough space for 8 characters and a trailing
 * null character.
 *
 * \param data The initial data bytestring
 * \param enc A pointer to the encoded string
 */
void
base32_encode_block(const base32_byte *data, char* enc);

/**
 * Base32 Encode Small Block
 *
 * Given a block of data that is less than 5 bytes long, encode it to Base32,
 * padding it at the end.
 *
 * \param data The array of data to encode
 * \param len The length of the data
 * \param enc The encoded data array
 */
void
base32_encode_small_block(const base32_byte *data, int len, char* enc);


/**
 * Base32 Encode
 *
 * Given a bytestring of data, find the Base32 encoding of that bytestring. It
 * is the programmer's responsibility to ensure that enough memory is allocated
 * to store the encoded data.
 *
 * \param data The original data to encode
 * \param len The length of the data in bytes
 * \param enc A pointer to the string into which to store the encoded version.
 * \return An intger: 1 on pass, 0 on failure
 */
int
base32_encode(const base32_byte *data, int len, char* enc);

/**
 * Base32 Decode Block
 *
 * Given a block of base32-encoded data that is 8 characters long, determine the
 * array of binary data (5 bytes) to which it corresponds.
 *
 * \param encoded The base32-encoded data
 * \param raw Raw data array
 */
void
base32_decode_block(const char* encoded, base32_byte *raw);

/**
 * Base32 Decode
 *
 * Given an array of Base32-encoded data, find the decoded raw binary data that
 * corresponds to said encoded data. It is the responsibility of the programmer to
 * ensure that the array of decoded data has enough space in it.
 *
 * \param encoded The encoded data array
 * \param raw The decoded data array
 * \return An integer: 0 on fail, 1 on success
 */
int
base32_decode(const char* encoded, base32_byte *raw);

/**
 * Base32 Allocated Size
 *
 * Given the length of a piece of binary data that is to be encoded to Base32,
 * return the minimum required size for a buffer that is to hold the encodeed
 * data.
 *
 * \param data_len The length of the data
 * \return the size of the buffer or -1 on error
 */
int
base32_allocated_size(int data_len);

#endif /* BASE32_H */
