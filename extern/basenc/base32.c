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

#include "base32.h"

/**
 * \var char base32_encoding
 * \brief The encoding array for Base32
 *
 * The Base32 alphabet maps the numebers 0 to 31 to uppercase Latin characters
 * and numbers between 2 and 7.
 */
char base32_encoding[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
			   'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
			   'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5',
			   '6', '7' };

/* Encode a 5-byte block of data to Base32 */
void
base32_encode_block(const base32_byte *data, char* enc)
{
  int i;
  int enc_index[8];

  enc_index[0] = (data[0] >> 3) & 0x1F;
  enc_index[1] = ((data[0] << 2) & 0x1C) | ((data[1] >> 6) & 0x3);
  enc_index[2] = (data[1] >> 1) & 0x1F;
  enc_index[3] = ((data[1] << 4) & 0x10) | ((data[2] >> 4) & 0xF);
  enc_index[4] = ((data[2] << 1) & 0x1E) | ((data[3] >> 7) & 0x1);
  enc_index[5] = (data[3] >> 2) & 0x1F;
  enc_index[6] = ((data[3] << 3) & 0x18) | ((data[4] >> 5) & 0x7);
  enc_index[7] = data[4] & 0x1F;

  for (i = 0; i < 8; i++) {
    enc[i] = base32_encoding[enc_index[i]];
  }
}

/* Encode small block */
void
base32_encode_small_block(const base32_byte *data, int len, char* enc)
{
  int i;

  enc[0] = base32_encoding[((data[0] >> 3) & 0x1F)];
  if (len == 1) {
    enc[1] = base32_encoding[(data[0] << 2) & 0x1C];
    for (i = 2; i < 8; i++) {
      enc[i] = BASE32_PADDING_CHAR;
    }
  } else {
    enc[1] = base32_encoding[((data[0] << 2) & 0x1C) | ((data[1] >> 6) & 0x3)];
    enc[2] = base32_encoding[(data[1] >> 1) & 0x1F];
    if (len == 2) {
      enc[3] = base32_encoding[(data[1] << 4) & 0x10];
      for (i = 4; i < 8; i++) {
	enc[i] = BASE32_PADDING_CHAR;
      }
    } else {
      enc[3] = base32_encoding[((data[1] << 4) & 0x10)
			       | ((data[2] >> 4) & 0xF)];
      if (len == 3) {
	enc[4] = base32_encoding[((data[2] << 1) & 0x1E)];
	for (i = 5; i < 8; i++) {
	  enc[i] = BASE32_PADDING_CHAR;
	}
      } else {
	enc[4] = base32_encoding[((data[2] << 1) & 0x1E)
				 | ((data[3] >> 7) & 0x1)];
	enc[5] = base32_encoding[((data[3] >> 2) & 0x1F)];
	if (len == 4) {
	  enc[6] = base32_encoding[((data[3] << 3) & 0x18)];
	  for (i = 7; i < 8; i++) {
	    enc[i] = BASE32_PADDING_CHAR;
	  }
	} else {
	  enc[6] = base32_encoding[((data[3] << 3) & 0x18)
				   | ((data[4] >> 5) & 0x7)];
	  enc[7] = base32_encoding[(data[4] & 0x1F)];
	}
      }
    }
  }
}

/* Encode data to Base32 */
int
base32_encode(const base32_byte *data, int len, char* enc)
{
  int success;

  int i, j, k;
  int remaining;
  base32_byte final_block[5];

  if (len > 0 && len >= 5) {
    j = 0;
    /* Encode first part of the data */
    for (i = 0; i + 5 <= len; i += 5) {
      base32_encode_block(&data[i], &enc[j]);
      j += 8;
    }
    remaining = len - i;
    if (remaining > 0) {
      for (k = 0; k < remaining; k++) {
	final_block[k] = data[i+k];
      }
      /* Finish encoding the remaining data */
      base32_encode_small_block(final_block, remaining, &enc[j]);
    }
    j+= 8;
    enc[j] = '\0';

    success = 1;
  } else if (len > 0 && len < 5) {
    /* Encode data of a length less than 5 */
    base32_encode_small_block(data, len, enc);
  } else {
    success = 0;
  }

  return success;
}

/* Get Base32 index from encoded data */
static unsigned int
base32_get_index(char ch)
{
  unsigned int index;

  if (ch >= 'A' && ch <= 'Z') {
    index = ch - 'A';
  } else if (ch >= '2' && ch <= '7') {
    index = ch - '2' + 26;
  } else if (ch == '=') {
    /* Deal with padding */
    index = 0;
  } else {
    /* Fail on error case */
    assert(0);
  }

  return index;
}

/* Decode a Base32-encoded block of data */
void
base32_decode_block(const char* encoded, base32_byte *raw)
{
  unsigned int indices[8];
  int i;
  for (i = 0; i < 8; i++) {
    indices[i] = base32_get_index(encoded[i]);
  }

  raw[0] = ((indices[0] << 3) & 0xF8) | ((indices[1] >> 2) & 0x7);
  raw[1] = ((indices[1] << 6) & 0xC0) | ((indices[2] << 1) & 0x3E)
    | ((indices[3] >> 4) & 0x1);
  raw[2] = ((indices[3] << 4) & 0xF0) | ((indices[4] >> 1) & 0xF);
  raw[3] = ((indices[4] << 7) & 0x80) | ((indices[5] << 2) & 0x7C)
    | ((indices[6] >> 3) & 0x3);
  raw[4] = ((indices[6] << 5) & 0xE0) | (indices[7] & 0x1F);
}

/* Decode Base32-Encoded Data */
int
base32_decode(const char* encoded, base32_byte *raw)
{
  int success = 0;

  int encoded_len = strlen(encoded);

  int i, j;
  j = 0;
  for (i = 0; i + 8 <= encoded_len; i += 8) {
    base32_decode_block(&encoded[i], &raw[j]);
    j += 5;
  }
  success = 1;

  return success;
}

/* Find minimum size for Base32-encoded data buffer */
int
base32_allocated_size(int data_len)
{
  int size = -1;
  if (data_len > 0) {
    size = data_len * 8;
    if (size % 5 == 0) {
      size = size / 5;
    } else {
      size = (size / 5) + 5;
    }
    size += 1; /* Needed for null character */
  }
  return size;
}

