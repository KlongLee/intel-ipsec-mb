/*******************************************************************************
  Copyright (c) 2012-2022, Intel Corporation

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of Intel Corporation nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SSE
#define CLEAR_SCRATCH_SIMD_REGS clear_scratch_xmms_sse

#include "intel-ipsec-mb.h"
#include "include/ipsec_ooo_mgr.h"
#include "include/kasumi_internal.h"
#include "include/zuc_internal.h"
#include "include/snow3g.h"
#include "include/gcm.h"
#include "include/chacha20_poly1305.h"
#include "include/snow3g_submit.h"

#include "include/save_xmms.h"
#include "include/des.h"
#include "include/cpu_feature.h"
#include "include/noaesni.h"
#include "include/aesni_emu.h"
#include "include/error.h"

#include "include/arch_sse_type1.h"
#include "include/arch_sse_type2.h"
#include "include/arch_sse_type3.h"

#include "include/ooo_mgr_reset.h"

#define SAVE_XMMS               save_xmms
#define RESTORE_XMMS            restore_xmms

#define SUBMIT_JOB_AES128_ENC submit_job_aes128_enc_ptr
#define SUBMIT_JOB_AES128_DEC submit_job_aes128_dec_sse
#define FLUSH_JOB_AES128_ENC  flush_job_aes128_enc_ptr
#define SUBMIT_JOB_AES192_ENC submit_job_aes192_enc_ptr
#define SUBMIT_JOB_AES192_DEC submit_job_aes192_dec_sse
#define FLUSH_JOB_AES192_ENC  flush_job_aes192_enc_ptr
#define SUBMIT_JOB_AES256_ENC submit_job_aes256_enc_ptr
#define SUBMIT_JOB_AES256_DEC submit_job_aes256_dec_sse
#define FLUSH_JOB_AES256_ENC  flush_job_aes256_enc_ptr
#define SUBMIT_JOB_AES_ECB_128_ENC submit_job_aes_ecb_128_enc_sse
#define SUBMIT_JOB_AES_ECB_128_DEC submit_job_aes_ecb_128_dec_sse
#define SUBMIT_JOB_AES_ECB_192_ENC submit_job_aes_ecb_192_enc_sse
#define SUBMIT_JOB_AES_ECB_192_DEC submit_job_aes_ecb_192_dec_sse
#define SUBMIT_JOB_AES_ECB_256_ENC submit_job_aes_ecb_256_enc_sse
#define SUBMIT_JOB_AES_ECB_256_DEC submit_job_aes_ecb_256_dec_sse
#define SUBMIT_JOB_HMAC       submit_job_hmac_sse
#define FLUSH_JOB_HMAC        flush_job_hmac_sse
#define SUBMIT_JOB_HMAC_NI    submit_job_hmac_ni_sse
#define FLUSH_JOB_HMAC_NI     flush_job_hmac_ni_sse
#define SUBMIT_JOB_HMAC_SHA_224       submit_job_hmac_sha_224_sse
#define FLUSH_JOB_HMAC_SHA_224        flush_job_hmac_sha_224_sse
#define SUBMIT_JOB_HMAC_SHA_224_NI    submit_job_hmac_sha_224_ni_sse
#define FLUSH_JOB_HMAC_SHA_224_NI     flush_job_hmac_sha_224_ni_sse
#define SUBMIT_JOB_HMAC_SHA_256       submit_job_hmac_sha_256_sse
#define FLUSH_JOB_HMAC_SHA_256        flush_job_hmac_sha_256_sse
#define SUBMIT_JOB_HMAC_SHA_256_NI    submit_job_hmac_sha_256_ni_sse
#define FLUSH_JOB_HMAC_SHA_256_NI     flush_job_hmac_sha_256_ni_sse
#define SUBMIT_JOB_HMAC_SHA_384       submit_job_hmac_sha_384_sse
#define FLUSH_JOB_HMAC_SHA_384        flush_job_hmac_sha_384_sse
#define SUBMIT_JOB_HMAC_SHA_512       submit_job_hmac_sha_512_sse
#define FLUSH_JOB_HMAC_SHA_512        flush_job_hmac_sha_512_sse
#define SUBMIT_JOB_HMAC_MD5   submit_job_hmac_md5_sse
#define FLUSH_JOB_HMAC_MD5    flush_job_hmac_md5_sse
#define SUBMIT_JOB_AES_XCBC   submit_job_aes_xcbc_sse
#define FLUSH_JOB_AES_XCBC    flush_job_aes_xcbc_sse
#define SUBMIT_JOB_SHA1   submit_job_sha1_sse
#define FLUSH_JOB_SHA1    flush_job_sha1_sse
#define SUBMIT_JOB_SHA224   submit_job_sha224_sse
#define FLUSH_JOB_SHA224    flush_job_sha224_sse
#define SUBMIT_JOB_SHA256   submit_job_sha256_sse
#define FLUSH_JOB_SHA256    flush_job_sha256_sse

#define SUBMIT_JOB_AES_CNTR   submit_job_aes_cntr_sse
#define SUBMIT_JOB_AES_CNTR_BIT   submit_job_aes_cntr_bit_sse

#define SUBMIT_JOB_ZUC_EEA3   submit_job_zuc_eea3_sse
#define FLUSH_JOB_ZUC_EEA3    flush_job_zuc_eea3_sse
#define SUBMIT_JOB_ZUC_EIA3   submit_job_zuc_eia3_sse
#define FLUSH_JOB_ZUC_EIA3    flush_job_zuc_eia3_sse
#define SUBMIT_JOB_ZUC256_EEA3   submit_job_zuc256_eea3_sse
#define FLUSH_JOB_ZUC256_EEA3    flush_job_zuc256_eea3_sse
#define SUBMIT_JOB_ZUC256_EIA3   submit_job_zuc256_eia3_sse
#define FLUSH_JOB_ZUC256_EIA3    flush_job_zuc256_eia3_sse

#define AES_CBC_DEC_128       aes128_cbc_dec_ptr
#define AES_CBC_DEC_192       aes192_cbc_dec_ptr
#define AES_CBC_DEC_256       aes256_cbc_dec_ptr

#define AES_CNTR_128       aes_cntr_128_sse
#define AES_CNTR_192       aes_cntr_192_sse
#define AES_CNTR_256       aes_cntr_256_sse

#define AES_CNTR_CCM_128   aes_cntr_ccm_128_sse
#define AES_CNTR_CCM_256   aes_cntr_ccm_256_sse

#define AES_ECB_ENC_128       aes_ecb_enc_128_sse
#define AES_ECB_ENC_192       aes_ecb_enc_192_sse
#define AES_ECB_ENC_256       aes_ecb_enc_256_sse
#define AES_ECB_DEC_128       aes_ecb_dec_128_sse
#define AES_ECB_DEC_192       aes_ecb_dec_192_sse
#define AES_ECB_DEC_256       aes_ecb_dec_256_sse

#define SUBMIT_JOB_PON_ENC        submit_job_pon_enc_sse
#define SUBMIT_JOB_PON_DEC        submit_job_pon_dec_sse
#define SUBMIT_JOB_PON_ENC_NO_CTR submit_job_pon_enc_no_ctr_sse
#define SUBMIT_JOB_PON_DEC_NO_CTR submit_job_pon_dec_no_ctr_sse

#define AES_GCM_DEC_128   aes_gcm_dec_128_sse
#define AES_GCM_ENC_128   aes_gcm_enc_128_sse
#define AES_GCM_DEC_192   aes_gcm_dec_192_sse
#define AES_GCM_ENC_192   aes_gcm_enc_192_sse
#define AES_GCM_DEC_256   aes_gcm_dec_256_sse
#define AES_GCM_ENC_256   aes_gcm_enc_256_sse

#define AES_GCM_DEC_IV_128   aes_gcm_dec_var_iv_128_sse
#define AES_GCM_ENC_IV_128   aes_gcm_enc_var_iv_128_sse
#define AES_GCM_DEC_IV_192   aes_gcm_dec_var_iv_192_sse
#define AES_GCM_ENC_IV_192   aes_gcm_enc_var_iv_192_sse
#define AES_GCM_DEC_IV_256   aes_gcm_dec_var_iv_256_sse
#define AES_GCM_ENC_IV_256   aes_gcm_enc_var_iv_256_sse

#define SUBMIT_JOB_AES_GCM_DEC submit_job_aes_gcm_dec_sse
#define SUBMIT_JOB_AES_GCM_ENC submit_job_aes_gcm_enc_sse

/* ====================================================================== */

#define SUBMIT_JOB         submit_job_sse
#define FLUSH_JOB          flush_job_sse
#define SUBMIT_JOB_NOCHECK submit_job_nocheck_sse
#define GET_NEXT_JOB       get_next_job_sse
#define GET_COMPLETED_JOB  get_completed_job_sse
#define SUBMIT_BURST       submit_burst_sse
#define SUBMIT_BURST_NOCHECK submit_burst_nocheck_sse
#define SUBMIT_CIPHER_BURST submit_cipher_burst_sse
#define SUBMIT_CIPHER_BURST_NOCHECK submit_cipher_burst_nocheck_sse
#define SUBMIT_HASH_BURST submit_hash_burst_sse
#define SUBMIT_HASH_BURST_NOCHECK submit_hash_burst_nocheck_sse

#define QUEUE_SIZE queue_size_sse

/* ====================================================================== */

#define SUBMIT_JOB_AES_ENC SUBMIT_JOB_AES_ENC_SSE
#define FLUSH_JOB_AES_ENC  FLUSH_JOB_AES_ENC_SSE
#define SUBMIT_JOB_AES_DEC SUBMIT_JOB_AES_DEC_SSE
#define SUBMIT_JOB_HASH    SUBMIT_JOB_HASH_SSE
#define FLUSH_JOB_HASH     FLUSH_JOB_HASH_SSE

/* ====================================================================== */

#define AES_CFB_128_ONE    aes_cfb_128_one_sse
#define AES_CFB_256_ONE    aes_cfb_256_one_sse

#define FLUSH_JOB_AES128_CCM_AUTH     flush_job_aes128_ccm_auth_ptr
#define SUBMIT_JOB_AES128_CCM_AUTH    submit_job_aes128_ccm_auth_ptr

#define FLUSH_JOB_AES256_CCM_AUTH     flush_job_aes256_ccm_auth_ptr
#define SUBMIT_JOB_AES256_CCM_AUTH    submit_job_aes256_ccm_auth_ptr

#define FLUSH_JOB_AES128_CMAC_AUTH    flush_job_aes128_cmac_auth_ptr
#define SUBMIT_JOB_AES128_CMAC_AUTH   submit_job_aes128_cmac_auth_ptr

#define FLUSH_JOB_AES256_CMAC_AUTH    flush_job_aes256_cmac_auth_ptr
#define SUBMIT_JOB_AES256_CMAC_AUTH   submit_job_aes256_cmac_auth_ptr

/* ====================================================================== */

#define SUBMIT_JOB_AES128_CBCS_1_9_ENC submit_job_aes128_cbcs_1_9_enc_sse
#define FLUSH_JOB_AES128_CBCS_1_9_ENC  flush_job_aes128_cbcs_1_9_enc_sse
#define SUBMIT_JOB_AES128_CBCS_1_9_DEC submit_job_aes128_cbcs_1_9_dec_sse
#define AES_CBCS_1_9_DEC_128           aes_cbcs_1_9_dec_128_sse

#define SUBMIT_JOB_CHACHA20_ENC_DEC submit_job_chacha20_enc_dec_sse
#define SUBMIT_JOB_CHACHA20_POLY1305 aead_chacha20_poly1305_sse
#define SUBMIT_JOB_CHACHA20_POLY1305_SGL aead_chacha20_poly1305_sgl_sse
#define POLY1305_MAC poly1305_mac_scalar

#define SUBMIT_JOB_SNOW_V snow_v_sse
#define SUBMIT_JOB_SNOW_V_AEAD snow_v_aead_init_sse

#define SUBMIT_JOB_SNOW3G_UEA2 submit_snow3g_uea2_job_sse
#define FLUSH_JOB_SNOW3G_UEA2  flush_snow3g_uea2_job_sse

/* ====================================================================== */

/*
 * Used to decide if SHA1/SHA256 SIMD or SHA1NI OOO scheduler should be
 * called.
 */
#define HASH_USE_SHAEXT 1


/* ====================================================================== */

uint32_t
ethernet_fcs_sse_local(const void *msg, const uint64_t len,
                       const void *tag_ouput);

#define ETHERNET_FCS ethernet_fcs_sse_local


/* ====================================================================== */

/*
 * CBC decrypt function pointers
 */

typedef void (* cbc_dec_fn_t)(const void *, const uint8_t *, const void *,
                              void *, uint64_t);

static cbc_dec_fn_t aes128_cbc_dec_ptr = aes_cbc_dec_128_sse;
static cbc_dec_fn_t aes192_cbc_dec_ptr = aes_cbc_dec_192_sse;
static cbc_dec_fn_t aes256_cbc_dec_ptr = aes_cbc_dec_256_sse;

/*
 * CBC encrypt function pointers
 */

typedef IMB_JOB *(*aes_submit_job_t)(MB_MGR_AES_OOO *, IMB_JOB *);

static aes_submit_job_t submit_job_aes128_enc_ptr =
        submit_job_aes128_enc_sse;
static aes_submit_job_t submit_job_aes192_enc_ptr =
        submit_job_aes192_enc_sse;
static aes_submit_job_t submit_job_aes256_enc_ptr =
        submit_job_aes256_enc_sse;

typedef IMB_JOB *(*aes_flush_job_t)(MB_MGR_AES_OOO *);

static aes_flush_job_t flush_job_aes128_enc_ptr = flush_job_aes128_enc_sse;
static aes_flush_job_t flush_job_aes192_enc_ptr = flush_job_aes192_enc_sse;
static aes_flush_job_t flush_job_aes256_enc_ptr = flush_job_aes256_enc_sse;

/* ====================================================================== */

/*
 * CMAC function pointers
 */

typedef IMB_JOB *(*cmac_submit_job_t)(MB_MGR_CMAC_OOO *, IMB_JOB *);
typedef IMB_JOB *(*cmac_flush_job_t)(MB_MGR_CMAC_OOO *);

static cmac_submit_job_t submit_job_aes128_cmac_auth_ptr =
        submit_job_aes128_cmac_auth_sse;
static cmac_flush_job_t flush_job_aes128_cmac_auth_ptr =
        flush_job_aes128_cmac_auth_sse;
static cmac_submit_job_t submit_job_aes256_cmac_auth_ptr =
        submit_job_aes256_cmac_auth_sse;
static cmac_flush_job_t flush_job_aes256_cmac_auth_ptr =
        flush_job_aes256_cmac_auth_sse;

/* ====================================================================== */

/*
 * CCM function pointers
 */

typedef IMB_JOB *(*ccm_submit_job_t)(MB_MGR_CCM_OOO *, IMB_JOB *);
typedef IMB_JOB *(*ccm_flush_job_t)(MB_MGR_CCM_OOO *);

static ccm_submit_job_t submit_job_aes128_ccm_auth_ptr =
        submit_job_aes128_ccm_auth_sse;
static ccm_flush_job_t flush_job_aes128_ccm_auth_ptr =
        flush_job_aes128_ccm_auth_sse;

static ccm_submit_job_t submit_job_aes256_ccm_auth_ptr =
        submit_job_aes256_ccm_auth_sse;
static ccm_flush_job_t flush_job_aes256_ccm_auth_ptr =
        flush_job_aes256_ccm_auth_sse;

/* ====================================================================== */

/*
 * ECB function pointers
 */
static void
(*aes_ecb_enc_128_sse) (const void *in, const void *keys,
                void *out, uint64_t len_bytes)=
                aes_ecb_enc_128_by4_sse;

static void
(*aes_ecb_enc_192_sse) (const void *in, const void *keys,
                void *out, uint64_t len_bytes)=
                aes_ecb_enc_192_by4_sse;

static void
(*aes_ecb_enc_256_sse) (const void *in, const void *keys,
                void *out, uint64_t len_bytes)=
                aes_ecb_enc_256_by4_sse;

static void
(*aes_ecb_dec_128_sse) (const void *in, const void *keys,
                void *out, uint64_t len_bytes)=
                aes_ecb_dec_128_by4_sse;

static void
(*aes_ecb_dec_192_sse) (const void *in, const void *keys,
                void *out, uint64_t len_bytes)=
                aes_ecb_dec_192_by4_sse;

static void
(*aes_ecb_dec_256_sse) (const void *in, const void *keys,
                void *out, uint64_t len_bytes)=
                aes_ecb_dec_256_by4_sse;

/* ====================================================================== */

/*
 * GCM submit / flush API for SSE arch
 */
static IMB_JOB *
submit_job_aes_gcm_dec_sse(IMB_MGR *state, IMB_JOB *job)
{
        DECLARE_ALIGNED(struct gcm_context_data ctx, 16);
        (void) state;

        if (16 == job->key_len_in_bytes) {
                AES_GCM_DEC_IV_128(job->dec_keys,
                                   &ctx, job->dst,
                                   job->src +
                                   job->cipher_start_src_offset_in_bytes,
                                   job->msg_len_to_cipher_in_bytes,
                                   job->iv, job->iv_len_in_bytes,
                                   job->u.GCM.aad,
                                   job->u.GCM.aad_len_in_bytes,
                                   job->auth_tag_output,
                                   job->auth_tag_output_len_in_bytes);
        } else if (24 == job->key_len_in_bytes) {
                AES_GCM_DEC_IV_192(job->dec_keys,
                                   &ctx, job->dst,
                                   job->src +
                                   job->cipher_start_src_offset_in_bytes,
                                   job->msg_len_to_cipher_in_bytes,
                                   job->iv, job->iv_len_in_bytes,
                                   job->u.GCM.aad,
                                   job->u.GCM.aad_len_in_bytes,
                                   job->auth_tag_output,
                                   job->auth_tag_output_len_in_bytes);
        } else { /* assume 32 bytes */
                AES_GCM_DEC_IV_256(job->dec_keys,
                                   &ctx, job->dst,
                                   job->src +
                                   job->cipher_start_src_offset_in_bytes,
                                   job->msg_len_to_cipher_in_bytes,
                                   job->iv, job->iv_len_in_bytes,
                                   job->u.GCM.aad,
                                   job->u.GCM.aad_len_in_bytes,
                                   job->auth_tag_output,
                                   job->auth_tag_output_len_in_bytes);
        }

        job->status = IMB_STATUS_COMPLETED;
        return job;
}

static IMB_JOB *
submit_job_aes_gcm_enc_sse(IMB_MGR *state, IMB_JOB *job)
{
        DECLARE_ALIGNED(struct gcm_context_data ctx, 16);
        (void) state;

        if (16 == job->key_len_in_bytes) {
                AES_GCM_ENC_IV_128(job->enc_keys,
                                   &ctx, job->dst,
                                   job->src +
                                   job->cipher_start_src_offset_in_bytes,
                                   job->msg_len_to_cipher_in_bytes,
                                   job->iv, job->iv_len_in_bytes,
                                   job->u.GCM.aad,
                                   job->u.GCM.aad_len_in_bytes,
                                   job->auth_tag_output,
                                   job->auth_tag_output_len_in_bytes);
        } else if (24 == job->key_len_in_bytes) {
                AES_GCM_ENC_IV_192(job->enc_keys,
                                   &ctx, job->dst,
                                   job->src +
                                   job->cipher_start_src_offset_in_bytes,
                                   job->msg_len_to_cipher_in_bytes,
                                   job->iv, job->iv_len_in_bytes,
                                   job->u.GCM.aad,
                                   job->u.GCM.aad_len_in_bytes,
                                   job->auth_tag_output,
                                   job->auth_tag_output_len_in_bytes);
        } else { /* assume 32 bytes */
                AES_GCM_ENC_IV_256(job->enc_keys,
                                   &ctx, job->dst,
                                   job->src +
                                   job->cipher_start_src_offset_in_bytes,
                                   job->msg_len_to_cipher_in_bytes,
                                   job->iv, job->iv_len_in_bytes,
                                   job->u.GCM.aad,
                                   job->u.GCM.aad_len_in_bytes,
                                   job->auth_tag_output,
                                   job->auth_tag_output_len_in_bytes);
        }

        job->status = IMB_STATUS_COMPLETED;
        return job;
}

static IMB_JOB *
submit_snow3g_uea2_job_sse(IMB_MGR *state, IMB_JOB *job)
{
        MB_MGR_SNOW3G_OOO *snow3g_uea2_ooo = state->snow3g_uea2_ooo;

        if ((job->msg_len_to_cipher_in_bits & 7) ||
            (job->cipher_start_offset_in_bits & 7))
                return def_submit_snow3g_uea2_job(state, job);

        return submit_job_snow3g_uea2_sse(snow3g_uea2_ooo, job);
}

static IMB_JOB *
flush_snow3g_uea2_job_sse(IMB_MGR *state)
{
        MB_MGR_SNOW3G_OOO *snow3g_uea2_ooo = state->snow3g_uea2_ooo;

        return flush_job_snow3g_uea2_sse(snow3g_uea2_ooo);
}

IMB_DLL_LOCAL IMB_JOB *
submit_job_aes_cntr_sse(IMB_JOB *job)
{
        if (16 == job->key_len_in_bytes)
                AES_CNTR_128(job->src + job->cipher_start_src_offset_in_bytes,
                             job->iv,
                             job->enc_keys,
                             job->dst,
                             job->msg_len_to_cipher_in_bytes,
                             job->iv_len_in_bytes);
        else if (24 == job->key_len_in_bytes)
                AES_CNTR_192(job->src + job->cipher_start_src_offset_in_bytes,
                             job->iv,
                             job->enc_keys,
                             job->dst,
                             job->msg_len_to_cipher_in_bytes,
                             job->iv_len_in_bytes);
        else /* assume 32 bytes */
                AES_CNTR_256(job->src + job->cipher_start_src_offset_in_bytes,
                             job->iv,
                             job->enc_keys,
                             job->dst,
                             job->msg_len_to_cipher_in_bytes,
                             job->iv_len_in_bytes);

        job->status |= IMB_STATUS_COMPLETED_CIPHER;
        return job;
}

IMB_DLL_LOCAL IMB_JOB *
submit_job_aes_cntr_bit_sse(IMB_JOB *job)
{
        if (16 == job->key_len_in_bytes)
                aes_cntr_bit_128_sse(job->src +
                                     job->cipher_start_src_offset_in_bytes,
                                     job->iv,
                                     job->enc_keys,
                                     job->dst,
                                     job->msg_len_to_cipher_in_bits,
                                     job->iv_len_in_bytes);
        else if (24 == job->key_len_in_bytes)
                aes_cntr_bit_192_sse(job->src +
                                     job->cipher_start_src_offset_in_bytes,
                                     job->iv,
                                     job->enc_keys,
                                     job->dst,
                                     job->msg_len_to_cipher_in_bits,
                                     job->iv_len_in_bytes);
        else /* assume 32 bytes */
                aes_cntr_bit_256_sse(job->src +
                                     job->cipher_start_src_offset_in_bytes,
                                     job->iv,
                                     job->enc_keys,
                                     job->dst,
                                     job->msg_len_to_cipher_in_bits,
                                     job->iv_len_in_bytes);

        job->status |= IMB_STATUS_COMPLETED_CIPHER;
        return job;
}

/* ====================================================================== */

static IMB_JOB *
(*submit_job_zuc_eea3_sse)
        (MB_MGR_ZUC_OOO *state, IMB_JOB *job) =
                        submit_job_zuc_eea3_no_gfni_sse;

static IMB_JOB *
(*flush_job_zuc_eea3_sse)
        (MB_MGR_ZUC_OOO *state) = flush_job_zuc_eea3_no_gfni_sse;

static IMB_JOB *
(*submit_job_zuc_eia3_sse)
        (MB_MGR_ZUC_OOO *state, IMB_JOB *job) =
                        submit_job_zuc_eia3_no_gfni_sse;

static IMB_JOB *
(*flush_job_zuc_eia3_sse)
        (MB_MGR_ZUC_OOO *state) = flush_job_zuc_eia3_no_gfni_sse;

static IMB_JOB *
(*submit_job_zuc256_eea3_sse)
        (MB_MGR_ZUC_OOO *state, IMB_JOB *job) =
                        submit_job_zuc256_eea3_no_gfni_sse;

static IMB_JOB *
(*flush_job_zuc256_eea3_sse)
        (MB_MGR_ZUC_OOO *state) = flush_job_zuc256_eea3_no_gfni_sse;

static IMB_JOB *
(*submit_job_zuc256_eia3_sse)
        (MB_MGR_ZUC_OOO *state, IMB_JOB *job, const uint64_t tag_sz) =
                        submit_job_zuc256_eia3_no_gfni_sse;

static IMB_JOB *
(*flush_job_zuc256_eia3_sse)
        (MB_MGR_ZUC_OOO *state, const uint64_t tag_sz) =
                        flush_job_zuc256_eia3_no_gfni_sse;

static void
reset_ooo_mgrs(IMB_MGR *state)
{
        /* Init AES out-of-order fields */
        if (state->features & IMB_FEATURE_GFNI) {
                ooo_mgr_aes_reset(state->aes128_ooo, 8);
                submit_job_aes128_enc_ptr = submit_job_aes128_enc_x8_sse;
                flush_job_aes128_enc_ptr = flush_job_aes128_enc_x8_sse;
        } else {
                ooo_mgr_aes_reset(state->aes128_ooo, 4);
        }

        if (state->features & IMB_FEATURE_GFNI) {
                ooo_mgr_aes_reset(state->aes192_ooo, 8);
                submit_job_aes192_enc_ptr = submit_job_aes192_enc_x8_sse;
                flush_job_aes192_enc_ptr = flush_job_aes192_enc_x8_sse;
        } else {
                ooo_mgr_aes_reset(state->aes192_ooo, 4);
        }

        if (state->features & IMB_FEATURE_GFNI) {
                ooo_mgr_aes_reset(state->aes256_ooo, 8);
                submit_job_aes256_enc_ptr = submit_job_aes256_enc_x8_sse;
                flush_job_aes256_enc_ptr = flush_job_aes256_enc_x8_sse;
        } else {
                ooo_mgr_aes_reset(state->aes256_ooo, 4);
        }

        if (state->features & IMB_FEATURE_GFNI) {
                /* change AES-CBC decrypt implementation */
                aes128_cbc_dec_ptr = aes_cbc_dec_128_by8_sse;
                aes192_cbc_dec_ptr = aes_cbc_dec_192_by8_sse;
                aes256_cbc_dec_ptr = aes_cbc_dec_256_by8_sse;
        }

        if (state->features & IMB_FEATURE_GFNI) {
                /* change AES-ECB implementation */
                aes_ecb_enc_128_sse = aes_ecb_enc_128_by8_sse;
                aes_ecb_enc_192_sse = aes_ecb_enc_192_by8_sse;
                aes_ecb_enc_256_sse = aes_ecb_enc_256_by8_sse;

                aes_ecb_dec_128_sse = aes_ecb_dec_128_by8_sse;
                aes_ecb_dec_192_sse = aes_ecb_dec_192_by8_sse;
                aes_ecb_dec_256_sse = aes_ecb_dec_256_by8_sse;
        }

        /* DOCSIS SEC BPI uses same settings as AES CBC */
        if (state->features & IMB_FEATURE_GFNI) {
                ooo_mgr_docsis_aes_reset(state->docsis128_sec_ooo, 8);
                ooo_mgr_docsis_aes_reset(state->docsis128_crc32_sec_ooo, 8);
                ooo_mgr_docsis_aes_reset(state->docsis256_sec_ooo, 8);
                ooo_mgr_docsis_aes_reset(state->docsis256_crc32_sec_ooo, 8);
        } else {
                ooo_mgr_docsis_aes_reset(state->docsis128_sec_ooo, 4);
                ooo_mgr_docsis_aes_reset(state->docsis128_crc32_sec_ooo, 4);
                ooo_mgr_docsis_aes_reset(state->docsis256_sec_ooo, 4);
                ooo_mgr_docsis_aes_reset(state->docsis256_crc32_sec_ooo, 4);
        }

        /* Init ZUC out-of-order fields */
        ooo_mgr_zuc_reset(state->zuc_eea3_ooo, 4);
        ooo_mgr_zuc_reset(state->zuc_eia3_ooo, 4);
        ooo_mgr_zuc_reset(state->zuc256_eea3_ooo, 4);
        ooo_mgr_zuc_reset(state->zuc256_eia3_ooo, 4);

        /* Init HMAC/SHA1 out-of-order fields */
        ooo_mgr_hmac_sha1_reset(state->hmac_sha_1_ooo, SSE_NUM_SHA1_LANES);

#ifdef HASH_USE_SHAEXT
        if (state->features & IMB_FEATURE_SHANI) {
                /* Init HMAC/SHA1 NI out-of-order fields */
                ooo_mgr_hmac_sha1_reset(state->hmac_sha_1_ooo, 2);
        }
#endif /* HASH_USE_SHAEXT */

        /* Init HMAC/SHA224 out-of-order fields */
        ooo_mgr_hmac_sha224_reset(state->hmac_sha_224_ooo,
                                  SSE_NUM_SHA256_LANES);

#ifdef HASH_USE_SHAEXT
        if (state->features & IMB_FEATURE_SHANI) {
                /* Init HMAC/SHA224 NI out-of-order fields */
                ooo_mgr_hmac_sha224_reset(state->hmac_sha_224_ooo, 2);
        }
#endif /* HASH_USE_SHAEXT */

        /* Init HMAC/SHA_256 out-of-order fields */
        ooo_mgr_hmac_sha256_reset(state->hmac_sha_256_ooo,
                                  SSE_NUM_SHA256_LANES);

#ifdef HASH_USE_SHAEXT
        if (state->features & IMB_FEATURE_SHANI) {
                /* Init HMAC/SHA256 NI out-of-order fields */
                ooo_mgr_hmac_sha256_reset(state->hmac_sha_256_ooo, 2);
        }
#endif /* HASH_USE_SHAEXT */

        /* Init HMAC/SHA384 out-of-order fields */
        ooo_mgr_hmac_sha384_reset(state->hmac_sha_384_ooo,
                                  SSE_NUM_SHA512_LANES);

        /* Init HMAC/SHA512 out-of-order fields */
        ooo_mgr_hmac_sha512_reset(state->hmac_sha_512_ooo,
                                  SSE_NUM_SHA512_LANES);

        /* Init HMAC/MD5 out-of-order fields */
        ooo_mgr_hmac_md5_reset(state->hmac_md5_ooo, SSE_NUM_MD5_LANES);

        /* Init AES/XCBC OOO fields */
        ooo_mgr_aes_xcbc_reset(state->aes_xcbc_ooo, 4);

        /* Init AES-CCM auth out-of-order fields */
        if (state->features & IMB_FEATURE_GFNI) {
                submit_job_aes128_ccm_auth_ptr =
                        submit_job_aes128_ccm_auth_x8_sse;
                flush_job_aes128_ccm_auth_ptr =
                        flush_job_aes128_ccm_auth_x8_sse;
                ooo_mgr_ccm_reset(state->aes_ccm_ooo, 8);
        } else {
                ooo_mgr_ccm_reset(state->aes_ccm_ooo, 4);
        }

        if (state->features & IMB_FEATURE_GFNI) {
                submit_job_aes256_ccm_auth_ptr =
                        submit_job_aes256_ccm_auth_x8_sse;
                flush_job_aes256_ccm_auth_ptr =
                        flush_job_aes256_ccm_auth_x8_sse;
                ooo_mgr_ccm_reset(state->aes256_ccm_ooo, 8);
        } else {
                ooo_mgr_ccm_reset(state->aes256_ccm_ooo, 4);
        }

        /* Init AES-CMAC auth out-of-order fields */
        if (state->features & IMB_FEATURE_GFNI) {
                submit_job_aes128_cmac_auth_ptr =
                        submit_job_aes128_cmac_auth_x8_sse;
                flush_job_aes128_cmac_auth_ptr =
                        flush_job_aes128_cmac_auth_x8_sse;
                ooo_mgr_cmac_reset(state->aes_cmac_ooo, 8);
        } else {
                ooo_mgr_cmac_reset(state->aes_cmac_ooo, 4);
        }

        if (state->features & IMB_FEATURE_GFNI) {
                submit_job_aes256_cmac_auth_ptr =
                        submit_job_aes256_cmac_auth_x8_sse;
                flush_job_aes256_cmac_auth_ptr =
                        flush_job_aes256_cmac_auth_x8_sse;
                ooo_mgr_cmac_reset(state->aes256_cmac_ooo, 8);
        } else {
                ooo_mgr_cmac_reset(state->aes256_cmac_ooo, 4);
        }

        /* Init AES-CBCS out-of-order fields */
        ooo_mgr_aes_reset(state->aes128_cbcs_ooo, 4);

        /* Init SHA1 out-of-order fields */
        ooo_mgr_sha1_reset(state->sha_1_ooo, SSE_NUM_SHA1_LANES);

        /* Init SHA224 out-of-order fields */
        ooo_mgr_sha256_reset(state->sha_224_ooo, SSE_NUM_SHA256_LANES);

        /* Init SHA256 out-of-order fields */
        ooo_mgr_sha256_reset(state->sha_256_ooo, SSE_NUM_SHA256_LANES);

        /* Init SNOW3G-UEA out-of-order fields */
        ooo_mgr_snow3g_reset(state->snow3g_uea2_ooo, 4);
}

IMB_DLL_LOCAL void
init_mb_mgr_sse_internal(IMB_MGR *state, const int reset_mgrs)
{
#ifdef SAFE_PARAM
        if (state == NULL) {
                imb_set_errno(NULL, IMB_ERR_NULL_MBMGR);
                return;
        }
#endif

        /* reset error status */
        imb_set_errno(state, 0);

        state->features = cpu_feature_adjust(state->flags,
                                             cpu_feature_detect());

        if (!(state->features & IMB_FEATURE_AESNI)) {
                fallback_no_aesni(state, reset_mgrs);
                return;
        }

        /* Check if CPU flags needed for SSE interface are present */
        if ((state->features & IMB_CPUFLAGS_SSE) != IMB_CPUFLAGS_SSE) {
                imb_set_errno(state, IMB_ERR_MISSING_CPUFLAGS_INIT_MGR);
                return;
        }

        /* Set architecture for future checks */
        state->used_arch = (uint32_t) IMB_ARCH_SSE;

        if (state->features & IMB_FEATURE_GFNI) {
                submit_job_zuc_eea3_sse = submit_job_zuc_eea3_gfni_sse;
                flush_job_zuc_eea3_sse = flush_job_zuc_eea3_gfni_sse;
                submit_job_zuc_eia3_sse = submit_job_zuc_eia3_gfni_sse;
                flush_job_zuc_eia3_sse = flush_job_zuc_eia3_gfni_sse;
                submit_job_zuc256_eea3_sse = submit_job_zuc256_eea3_gfni_sse;
                flush_job_zuc256_eea3_sse = flush_job_zuc256_eea3_gfni_sse;
                submit_job_zuc256_eia3_sse = submit_job_zuc256_eia3_gfni_sse;
                flush_job_zuc256_eia3_sse = flush_job_zuc256_eia3_gfni_sse;
        }

        if (reset_mgrs) {
                reset_ooo_mgrs(state);

                /* Init "in order" components */
                state->next_job = 0;
                state->earliest_job = -1;
        }

        /* set SSE handlers */
        state->get_next_job        = get_next_job_sse;
        state->submit_job          = submit_job_sse;
        state->submit_burst        = submit_burst_sse;
        state->submit_burst_nocheck= submit_burst_nocheck_sse;
        state->submit_cipher_burst = submit_cipher_burst_sse;
        state->submit_cipher_burst_nocheck = submit_cipher_burst_nocheck_sse;
        state->submit_hash_burst   = submit_hash_burst_sse;
        state->submit_hash_burst_nocheck = submit_hash_burst_nocheck_sse;
        state->submit_job_nocheck  = submit_job_nocheck_sse;
        state->get_completed_job   = get_completed_job_sse;
        state->flush_job           = flush_job_sse;
        state->queue_size          = queue_size_sse;
        state->keyexp_128          = aes_keyexp_128_sse;
        state->keyexp_192          = aes_keyexp_192_sse;
        state->keyexp_256          = aes_keyexp_256_sse;
        state->cmac_subkey_gen_128 = aes_cmac_subkey_gen_sse;
        state->cmac_subkey_gen_256 = aes_cmac_256_subkey_gen_sse;
        state->xcbc_keyexp         = aes_xcbc_expand_key_sse;
        state->des_key_sched       = des_key_schedule;
        state->sha1_one_block      = sha1_one_block_sse;
        state->sha1                = sha1_sse;
        state->sha224_one_block    = sha224_one_block_sse;
        state->sha224              = sha224_sse;
        state->sha256_one_block    = sha256_one_block_sse;
        state->sha256              = sha256_sse;
        state->sha384_one_block    = sha384_one_block_sse;
        state->sha384              = sha384_sse;
        state->sha512_one_block    = sha512_one_block_sse;
        state->sha512              = sha512_sse;
        state->md5_one_block       = md5_one_block_sse;
        state->aes128_cfb_one      = aes_cfb_128_one_sse;
        state->crc32_ethernet_fcs  = ethernet_fcs_sse;
        state->crc16_x25           = crc16_x25_sse;
        state->crc32_sctp          = crc32_sctp_sse;
        state->crc24_lte_a         = crc24_lte_a_sse;
        state->crc24_lte_b         = crc24_lte_b_sse;
        state->crc16_fp_data       = crc16_fp_data_sse;
        state->crc11_fp_header     = crc11_fp_header_sse;
        state->crc7_fp_header      = crc7_fp_header_sse;
        state->crc10_iuup_data     = crc10_iuup_data_sse;
        state->crc6_iuup_header    = crc6_iuup_header_sse;
        state->crc32_wimax_ofdma_data = crc32_wimax_ofdma_data_sse;
        state->crc8_wimax_ofdma_hcs = crc8_wimax_ofdma_hcs_sse;

        state->eea3_1_buffer       = zuc_eea3_1_buffer_sse;
        if (state->features & IMB_FEATURE_GFNI) {
                state->eea3_4_buffer       = zuc_eea3_4_buffer_gfni_sse;
                state->eea3_n_buffer       = zuc_eea3_n_buffer_gfni_sse;
                state->eia3_n_buffer       = zuc_eia3_n_buffer_gfni_sse;
        } else {
                state->eea3_4_buffer       = zuc_eea3_4_buffer_sse;
                state->eea3_n_buffer       = zuc_eea3_n_buffer_sse;
                state->eia3_n_buffer       = zuc_eia3_n_buffer_sse;
        }

        state->eia3_1_buffer       = zuc_eia3_1_buffer_sse;

        state->f8_1_buffer         = kasumi_f8_1_buffer_sse;
        state->f8_1_buffer_bit     = kasumi_f8_1_buffer_bit_sse;
        state->f8_2_buffer         = kasumi_f8_2_buffer_sse;
        state->f8_3_buffer         = kasumi_f8_3_buffer_sse;
        state->f8_4_buffer         = kasumi_f8_4_buffer_sse;
        state->f8_n_buffer         = kasumi_f8_n_buffer_sse;
        state->f9_1_buffer         = kasumi_f9_1_buffer_sse;
        state->f9_1_buffer_user    = kasumi_f9_1_buffer_user_sse;
        state->kasumi_init_f8_key_sched = kasumi_init_f8_key_sched_sse;
        state->kasumi_init_f9_key_sched = kasumi_init_f9_key_sched_sse;
        state->kasumi_key_sched_size = kasumi_key_sched_size_sse;

        state->snow3g_f8_1_buffer_bit = snow3g_f8_1_buffer_bit_sse;
        state->snow3g_f8_1_buffer  = snow3g_f8_1_buffer_sse;
        state->snow3g_f8_2_buffer  = snow3g_f8_2_buffer_sse;
        state->snow3g_f8_4_buffer  = snow3g_f8_4_buffer_sse;
        state->snow3g_f8_8_buffer  = snow3g_f8_8_buffer_sse;
        state->snow3g_f8_n_buffer  = snow3g_f8_n_buffer_sse;
        state->snow3g_f8_8_buffer_multikey = snow3g_f8_8_buffer_multikey_sse;
        state->snow3g_f8_n_buffer_multikey = snow3g_f8_n_buffer_multikey_sse;
        state->snow3g_f9_1_buffer = snow3g_f9_1_buffer_sse;
        state->snow3g_init_key_sched = snow3g_init_key_sched_sse;
        state->snow3g_key_sched_size = snow3g_key_sched_size_sse;

        state->gcm128_enc          = aes_gcm_enc_128_sse;
        state->gcm192_enc          = aes_gcm_enc_192_sse;
        state->gcm256_enc          = aes_gcm_enc_256_sse;
        state->gcm128_dec          = aes_gcm_dec_128_sse;
        state->gcm192_dec          = aes_gcm_dec_192_sse;
        state->gcm256_dec          = aes_gcm_dec_256_sse;
        state->gcm128_init         = aes_gcm_init_128_sse;
        state->gcm192_init         = aes_gcm_init_192_sse;
        state->gcm256_init         = aes_gcm_init_256_sse;
        state->gcm128_init_var_iv  = aes_gcm_init_var_iv_128_sse;
        state->gcm192_init_var_iv  = aes_gcm_init_var_iv_192_sse;
        state->gcm256_init_var_iv  = aes_gcm_init_var_iv_256_sse;
        state->gcm128_enc_update   = aes_gcm_enc_128_update_sse;
        state->gcm192_enc_update   = aes_gcm_enc_192_update_sse;
        state->gcm256_enc_update   = aes_gcm_enc_256_update_sse;
        state->gcm128_dec_update   = aes_gcm_dec_128_update_sse;
        state->gcm192_dec_update   = aes_gcm_dec_192_update_sse;
        state->gcm256_dec_update   = aes_gcm_dec_256_update_sse;
        state->gcm128_enc_finalize = aes_gcm_enc_128_finalize_sse;
        state->gcm192_enc_finalize = aes_gcm_enc_192_finalize_sse;
        state->gcm256_enc_finalize = aes_gcm_enc_256_finalize_sse;
        state->gcm128_dec_finalize = aes_gcm_dec_128_finalize_sse;
        state->gcm192_dec_finalize = aes_gcm_dec_192_finalize_sse;
        state->gcm256_dec_finalize = aes_gcm_dec_256_finalize_sse;
        state->gcm128_precomp      = aes_gcm_precomp_128_sse;
        state->gcm192_precomp      = aes_gcm_precomp_192_sse;
        state->gcm256_precomp      = aes_gcm_precomp_256_sse;
        state->gcm128_pre          = aes_gcm_pre_128_sse;
        state->gcm192_pre          = aes_gcm_pre_192_sse;
        state->gcm256_pre          = aes_gcm_pre_256_sse;
        state->ghash               = ghash_sse;
        state->ghash_pre           = ghash_pre_sse;

        state->gmac128_init        = imb_aes_gmac_init_128_sse;
        state->gmac192_init        = imb_aes_gmac_init_192_sse;
        state->gmac256_init        = imb_aes_gmac_init_256_sse;
        state->gmac128_update      = imb_aes_gmac_update_128_sse;
        state->gmac192_update      = imb_aes_gmac_update_192_sse;
        state->gmac256_update      = imb_aes_gmac_update_256_sse;
        state->gmac128_finalize    = imb_aes_gmac_finalize_128_sse;
        state->gmac192_finalize    = imb_aes_gmac_finalize_192_sse;
        state->gmac256_finalize    = imb_aes_gmac_finalize_256_sse;

        state->hec_32              = hec_32_sse;
        state->hec_64              = hec_64_sse;

        state->chacha20_poly1305_init = init_chacha20_poly1305_sse;
        state->chacha20_poly1305_enc_update = update_enc_chacha20_poly1305_sse;
        state->chacha20_poly1305_dec_update = update_dec_chacha20_poly1305_sse;
        state->chacha20_poly1305_finalize = finalize_chacha20_poly1305_sse;
}

void
init_mb_mgr_sse(IMB_MGR *state)
{
        init_mb_mgr_sse_internal(state, 1);
}

#include "mb_mgr_code.h"
