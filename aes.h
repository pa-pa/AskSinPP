//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// adoption of Daniel Otte's cool AVR-Crypto-Lib, Copyright (C) 2008, GPLv3 or later
// cleaned and tidied to fit the purpose for AskSin++ by trilu
//- -----------------------------------------------------------------------------------------------------------------------


#ifndef AES_H_
#define AES_H_

#include <stdint.h>

typedef struct {
	uint8_t ks[16];
} aes_roundkey_t;

typedef struct {
	aes_roundkey_t key[10 + 1];
} aes128_ctx_t;

typedef struct {
	aes_roundkey_t key[1]; /* just to avoid the warning */
} aes_genctx_t;

typedef struct {
	uint8_t s[16];
} aes_cipher_state_t;

void aes128_init(const void* key, aes128_ctx_t* ctx);
void aes128_enc(void* buffer, aes128_ctx_t* ctx);
void aes128_dec(void* buffer, aes128_ctx_t* ctx);

//-init
void aes_rotword(void* a);
void aes_init(const void* key, uint16_t keysize_b, aes_genctx_t* ctx);
//-encrypt
void aes_shiftcol(void* data, uint8_t shift);
void aes_enc_round(aes_cipher_state_t* state, const aes_roundkey_t* k);
void aes_enc_lastround(aes_cipher_state_t* state, const aes_roundkey_t* k);
void aes_encrypt_core(aes_cipher_state_t* state, const aes_genctx_t* ks, uint8_t rounds);
//-decrypt
void aes_invshiftrow(void* data, uint8_t shift);
void aes_invshiftcol(void* data, uint8_t shift);
void aes_dec_round(aes_cipher_state_t* state, const aes_roundkey_t* k);
void aes_dec_firstround(aes_cipher_state_t* state, const aes_roundkey_t* k);
void aes_decrypt_core(aes_cipher_state_t* state, const aes_genctx_t* ks, uint8_t rounds);


#endif
