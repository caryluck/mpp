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

#ifndef __MPP_ENC_HAL_H__
#define __MPP_ENC_HAL_H__

#include "mpp_hal.h"

typedef struct MppEncHalCfg_t {
    // input for encoder
    MppCodingType   coding;
    MppEncCfgSet    *set;
    MppEncCfgSet    *cfg;

    // output for enc_impl
    HalWorkMode     work_mode;
    MppDeviceId     device_id;
} MppEncHalCfg;

typedef struct MppEncHalApi_t {
    char            *name;
    MppCodingType   coding;
    RK_U32          ctx_size;
    RK_U32          flag;

    MPP_RET (*init)(void *ctx, MppEncHalCfg *cfg);
    MPP_RET (*deinit)(void *ctx);

    // configure function
    MPP_RET (*get_task)(void *ctx, HalEncTask *task);
    MPP_RET (*gen_regs)(void *ctx, HalEncTask *task);

    // hw operation function
    MPP_RET (*start)(void *ctx, HalEncTask *task);
    MPP_RET (*wait)(void *ctx, HalEncTask *task);

    // return function
    MPP_RET (*ret_task)(void *ctx, HalEncTask *task);
} MppEncHalApi;

typedef void* MppEncHal;

#ifdef __cplusplus
extern "C" {
#endif

MPP_RET mpp_enc_hal_init(MppEncHal *ctx, MppEncHalCfg *cfg);
MPP_RET mpp_enc_hal_deinit(MppEncHal ctx);

MPP_RET mpp_enc_hal_get_task(MppEncHal ctx, HalEncTask *task);
MPP_RET mpp_enc_hal_gen_regs(MppEncHal ctx, HalEncTask *task);

// start / wait hardware
MPP_RET mpp_enc_hal_start(MppEncHal ctx, HalEncTask *task);
MPP_RET mpp_enc_hal_wait(MppEncHal ctx, HalEncTask *task);

MPP_RET mpp_enc_hal_ret_task(MppEncHal ctx, HalEncTask *task);

#ifdef __cplusplus
}
#endif

#endif /*__MPP_ENC_HAL_H__*/
