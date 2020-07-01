/*
 * Copyright 2015 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __H264E_MLVEC_H__
#define __H264E_MLVEC_H__

#include "mpp_rc_defs.h"
#include "mpp_enc_refs.h"

#define MLVEC_CHANGE_LTR_FRAMES                 (0x00000001)
#define MLVEC_CHANGE_MAX_TEMPORAL_LAYER_COUNT   (0x00000002)
#define MLVEC_CHANGE_ADD_PREFIX_NAL             (0x00000004)

#define MLVEC_CHANGE_MARK_LTR                   (0x00010000)
#define MLVEC_CHANGE_USE_LTR                    (0x00020000)
#define MLVEC_CHANGE_FRAME_QP                   (0x00040000)
#define MLVEC_CHANGE_BASE_LAYER_PID             (0x00080000)

typedef struct H264eMlvecStaticCfg_t {
    RK_U32      change;
    RK_S32      add_prefix;
    RK_S32      ltr_frames;
    RK_S32      max_temporal_layer_count;
} H264eMlvecStaticCfg;

typedef struct H264eMlvecDynamicCfg_t {
    RK_U32      change;
    RK_S32      max_temporal_layer_count;
    RK_S32      mark_ltr;
    RK_S32      use_ltr;
    RK_S32      frame_qp;
    RK_S32      base_layer_pid;
} H264eMlvecDynamicCfg;

typedef void* H264eMlvecCtx;

#ifdef __cplusplus
extern "C" {
#endif

MPP_RET mlvec_init(H264eMlvecCtx *ctx);
MPP_RET mlvec_deinit(H264eMlvecCtx ctx);

MPP_RET mlvec_set_static_config(H264eMlvecCtx ctx, H264eMlvecStaticCfg *cfg);
MPP_RET mlvec_set_dynamic_config(H264eMlvecCtx ctx, H264eMlvecDynamicCfg *cfg);

MPP_RET mlvec_frame_start(H264eMlvecCtx ctx, MppEncRefFrmUsrCfg *frm);
MPP_RET mlvec_rc_setup(H264eMlvecCtx ctx, EncRcForceCfg *cfg);

MPP_RET mlvec_frame_end(H264eMlvecCtx ctx);

#ifdef __cplusplus
}
#endif

#endif /* __H264E_MLVEC_H__ */
