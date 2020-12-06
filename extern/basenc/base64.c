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

#include "base64.h"

/*
 * Base64 Encoding Table
 */
char base64_encoding[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
			   'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
			   'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
			   'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
			   's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2',
			   '3', '4', '5', '6', '7', '8', '9', '+', '/' };

/* Encode a 3-byte block of data to Base64 */
void
base64_encode_block(const base64_byte *data, char* enc)
{
  int i;
  int enc_index[BASE64_ENCODED_BLOCK_SIZE];

  enc_index[0] = (data[0] >> 2) & 0x3F;
  enc_index[1] = ((data[0] << 4) & 0x3C) | ((data[1] >> 4) & 0xF);
  enc_index[2] = ((data[1] << 2) & 0x3C) | ((data[2] >> 6) & 0x3);
  enc_index[3] = (data[2] & 0x3F);

  /* Encode the data block to Base64 */
  for(i = 0; i < BASE64_ENCODED_BLOCK_SIZE; i++) {
    enc[i] = base64_encoding[enc_index[i]];
  }
}

/* Encode Data to Base64 */
void
base64_encode(base64_byte *data, int len, char* encoded)
{
  int i, j;
  int remaining;
  j = 0;
  for (i = 0; i + 3 <= len; i += 3) {
    base64_encode_block(&data[i], &encoded[j]);
    j += 4;
  }

  remaining = len - i;
  if (remaining == 1) {
    encoded[j] = base64_encoding[(data[i] >> 2) & 0x3F];
    encoded[j+1] = base64_encoding[(data[i] << 4) & 0x30];
    encoded[j+2] = BASE64_PADDING_CHAR;
    encoded[j+3] = BASE64_PADDING_CHAR;
  } else if (remaining == 2) {
    encoded[j] = base64_encoding[(data[i] >> 2) & 0x3F];
    encoded[j+1] = base64_encoding[((data[i] << 4) & 0x30)
				   | ((data[i+1] >> 4) & 0xF)];
    encoded[j+2] = base64_encoding[(data[i+1] << 2) & 0x3C];
    encoded[j+3] = BASE64_PADDING_CHAR;
  }
}

/* Get index of Base64 Character */
static int
base64_get_index(char ch)
{
  int index;

  if (ch >= 'A' && ch <= 'Z') {
    index = ch - 'A';
  } else if (ch >= 'a' && ch <= 'z') {
    index = 26 + (ch - 'a');
  } else if (ch >= '0' && ch <= '9') {
    index = 26 + 26 + (ch - '0');
  } else if (ch == '+') {
    index = 26 + 26 + 10;
  } else if (ch == '/') {
    index = 26 + 26 + 11;
  } else if (ch == '=') {
    index = 0; /* Padding */
  } else {
    printf("Incorrect input, illegal character '%c'\n", ch);
    assert(0); /* Fail on incorrect input */
  }

  return index;
}

/* Decode Base64-encoded data */
void
base64_decode(char* encoded, base64_byte *raw)
{
  int encoded_len = strlen(encoded);
  int i, j, k;
  base64_byte indices[4];
  
  /*2015.10.19 by kun*/
  /*add eqlNum to record the symbol '==',because the 'A' decode index equal 0,as same as '=='*/
  int eqlNum = 0;		
  k = 0;
  for (i = 0; i + 4 <= encoded_len; i += 4) {
    for (j = 0; j < 4; j++) {
      indices[j] = base64_get_index(encoded[i+j]);
      if (encoded[i+j] =='=')
      {
      	eqlNum++;
      }
    }
    
    /*2015.10.19 by kun*/
    /*change all indices[i] => indices[j]*/
    /*change the code indices[j] << 6  => indices[j] << 2*/
    j = 0;
    raw[k] = ((indices[j] << 2) & 0xFC) | ((indices[j+1] >> 4) & 0x3);
    raw[k+1] = ((indices[j+1] << 4) & 0xF0) | ((indices[j+2] >> 2) & 0xF);
    raw[k+2] = ((indices[j+2] << 6) & 0xC0) | (indices[j+3] & 0x3F);
    k += 3;
  }
  
  /*2015.10.19 by kun*/
  /*record the correct num of the string with '\0',and need to decrease the '\0' which be decoded from '=='*/
	k = k - eqlNum;
}

/* Find allocated buffer size */
int
base64_allocated_size(int data_len)
{
  int size = -1;
  if (data_len > 0) {
    size = size * 4;
    if (size % 3 == 0) {
      size /= 3;
    } else {
      size = (size / 3) + 3;
    }
    size += 1; /* Need for null character */
  }
  return size;
}
