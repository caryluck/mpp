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

#define MODULE_TAG "h264e_mlvec"

#include <string.h>

#include "mpp_log.h"
#include "mpp_mem.h"

#include "h264e_mlvec.h"

typedef struct H264eMlvecCtxImpl_t {
    H264eMlvecStaticCfg     static_cfg;
    H264eMlvecDynamicCfg    dynamic_cfg;

    H264eMlvecStaticCfg     static_cfg_old;
    H264eMlvecDynamicCfg    dynamic_cfg_old;

    RK_S32                  enabled;

    RK_S32                  max_ltr_idx;
    RK_S32                  cur_ltr_idx;
} H264eMlvecCtxImpl;

static void reset_dynamic_cfg(H264eMlvecDynamicCfg *cfg)
{
    cfg->change = 0;
    cfg->mark_ltr = -1;
    cfg->use_ltr = -1;
    cfg->base_layer_pid = -1;
    /* NOTE: frame qp can not be reset */
}

MPP_RET mlvec_init(H264eMlvecCtx *ctx)
{
    if (NULL == ctx) {
        mpp_err_f("invalid NULL input\n");
        return MPP_ERR_NULL_PTR;
    }

    MPP_RET ret = MPP_OK;
    H264eMlvecCtxImpl *p = mpp_calloc(H264eMlvecCtxImpl, 1);
    if (NULL == p) {
        mpp_err_f("failed to create MLVEC context\n");
        ret = MPP_ERR_NOMEM;
    } else {
        p->dynamic_cfg.frame_qp = -1;
        reset_dynamic_cfg(&p->dynamic_cfg);
    }
    *ctx = p;

    return ret;
}

MPP_RET mlvec_deinit(H264eMlvecCtx ctx)
{
    MPP_FREE(ctx);
    return MPP_OK;
}

MPP_RET mlvec_set_static_config(H264eMlvecCtx ctx, H264eMlvecStaticCfg *cfg)
{
    if (NULL == ctx) {
        mpp_err_f("invalid NULL input\n");
        return MPP_ERR_NULL_PTR;
    }

    H264eMlvecCtxImpl *impl = (H264eMlvecCtxImpl *)ctx;
    H264eMlvecStaticCfg *dst = &impl->static_cfg;

    if (NULL == cfg) {
        // disable all feature
        memset(impl, 0, sizeof(*impl));
    } else {
        RK_U32 change = cfg->change;

        if (change & MLVEC_CHANGE_LTR_FRAMES)
            dst->ltr_frames = cfg->ltr_frames;

        if (change & MLVEC_CHANGE_MAX_TEMPORAL_LAYER_COUNT)
            dst->max_temporal_layer_count = cfg->max_temporal_layer_count;

        if (change & MLVEC_CHANGE_ADD_PREFIX_NAL)
            dst->add_prefix = cfg->add_prefix;

        dst->change = change;
        cfg->change = 0;
    }

    return MPP_OK;
}

MPP_RET mlvec_set_dynamic_config(H264eMlvecCtx ctx, H264eMlvecDynamicCfg *cfg)
{
    if (NULL == ctx) {
        mpp_err_f("invalid NULL input\n");
        return MPP_ERR_NULL_PTR;
    }

    H264eMlvecCtxImpl *impl = (H264eMlvecCtxImpl *)ctx;
    H264eMlvecDynamicCfg *dst = &impl->dynamic_cfg;

    if (NULL == cfg) {
        // disable all feature
        memset(impl, 0, sizeof(*impl));
    } else {
        RK_U32 change = cfg->change;

        if (change & MLVEC_CHANGE_MARK_LTR)
            dst->mark_ltr = cfg->mark_ltr;

        if (change & MLVEC_CHANGE_USE_LTR)
            dst->use_ltr = cfg->use_ltr;

        if (change & MLVEC_CHANGE_FRAME_QP)
            dst->frame_qp = cfg->frame_qp;

        if (change & MLVEC_CHANGE_BASE_LAYER_PID)
            dst->base_layer_pid = cfg->base_layer_pid;

        dst->change |= change;
        cfg->change = 0;
    }

    return MPP_OK;
}

MPP_RET mlvec_frame_start(H264eMlvecCtx ctx, MppEncRefFrmUsrCfg *cfg)
{
    if (NULL == ctx || NULL == cfg) {
        mpp_err_f("invalid NULL input ctx %p cfg %p\n", ctx, cfg);
        return MPP_ERR_NULL_PTR;
    }

    H264eMlvecCtxImpl *impl = (H264eMlvecCtxImpl *)ctx;
    H264eMlvecStaticCfg *cfg_st = &impl->static_cfg;
    H264eMlvecDynamicCfg *cfg_dy = &impl->dynamic_cfg;

    if (cfg_st->max_temporal_layer_count)
        cfg_st->add_prefix = 1;

    if (cfg_dy->mark_ltr >= 0) {
        cfg->force_flag |= ENC_FORCE_LT_REF_IDX;
        cfg->force_lt_idx = cfg_dy->mark_ltr;
        mpp_log_f("force_lt_idx %d\n", cfg->force_lt_idx);
        cfg_dy->mark_ltr = -1;
    }

    if (cfg_dy->use_ltr >= 0) {
        cfg->force_flag |= ENC_FORCE_REF_MODE;
        cfg->force_ref_mode = REF_TO_LT_REF_IDX;
        cfg->force_ref_arg = cfg_dy->use_ltr;
        mpp_log_f("force_ref mode %d arg %d\n", cfg->force_ref_mode, cfg->force_ref_arg);
        cfg_dy->use_ltr = -1;
    }

    return MPP_OK;
}

MPP_RET mlvec_rc_setup(H264eMlvecCtx ctx, EncRcForceCfg *cfg)
{
    if (NULL == ctx || NULL == cfg) {
        mpp_err_f("invalid NULL input ctx %p cfg %p\n", ctx, cfg);
        return MPP_ERR_NULL_PTR;
    }

    H264eMlvecCtxImpl *impl = (H264eMlvecCtxImpl *)ctx;
    H264eMlvecDynamicCfg *cfg_dy = &impl->dynamic_cfg;

    mpp_log_f("change %x frame_qp %d\n", cfg_dy->change, cfg_dy->frame_qp);
    if (cfg_dy->frame_qp >= 0) {
        cfg->force_flag = ENC_RC_FORCE_QP;
        cfg->force_qp = cfg_dy->frame_qp;
    } else {
        cfg->force_flag = 0;
        cfg->force_qp = -1;
    }

    return MPP_OK;
}

MPP_RET mlvec_frame_end(H264eMlvecCtx ctx)
{
    if (NULL == ctx) {
        mpp_err_f("invalid NULL input\n");
        return MPP_ERR_NULL_PTR;
    }

    H264eMlvecCtxImpl *impl = (H264eMlvecCtxImpl *)ctx;

    reset_dynamic_cfg(&impl->dynamic_cfg);

    return MPP_OK;
}
