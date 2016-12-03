/*
 * Copyright 2016 Joey  <joeyye@foxmail.com>
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
#include "data.h"

// Translates FFmpeg codec and profile to VA profile
bool ffmpeg2vaprofile(enum AVCodecID ff_codec, int ff_profile,VAProfile *profile_ptr)
{
    int profile = -1;

    switch (ff_codec) {
    case AV_CODEC_ID_MPEG2VIDEO:
        switch (ff_profile) {
            DEFINE_PROFILE(MPEG2, SIMPLE, Simple);
            DEFINE_PROFILE(MPEG2, MAIN, Main);
        }
        break;
    case AV_CODEC_ID_MPEG4:
        switch (ff_profile) {
            DEFINE_PROFILE(MPEG4, SIMPLE, Simple);
            DEFINE_PROFILE(MPEG4, MAIN, Main);
            DEFINE_PROFILE(MPEG4, ADVANCED_SIMPLE, AdvancedSimple);
        }
        break;
    case AV_CODEC_ID_H264:
        switch (ff_profile) {
            DEFINE_PROFILE(H264, BASELINE, Baseline);
            DEFINE_PROFILE(H264, CONSTRAINED_BASELINE, ConstrainedBaseline);
            DEFINE_PROFILE(H264, MAIN, Main);
            DEFINE_PROFILE(H264, HIGH, High);
        }
        break;
    case AV_CODEC_ID_WMV3:
    case AV_CODEC_ID_VC1:
        switch (ff_profile) {
            DEFINE_PROFILE(VC1, SIMPLE, Simple);
            DEFINE_PROFILE(VC1, MAIN, Main);
            DEFINE_PROFILE(VC1, ADVANCED, Advanced);
        }
        break;
    case AV_CODEC_ID_HEVC:
        switch (ff_profile) {
            DEFINE_PROFILE(HEVC, MAIN, Main);
            DEFINE_PROFILE(HEVC, MAIN_10, Main10);
        }
        break;
    case AV_CODEC_ID_VP8:
        profile = VAProfileVP8Version0_3;
        break;
    case AV_CODEC_ID_VP9:
        profile = VAProfileVP9Version0;
        break;
    default:
        break;
    }

    if (profile_ptr)
        *profile_ptr = (VAProfile)profile;
    return profile != -1;
}

static int init_decoder(vaglContext *ctx, VAProfile profile, VAEntrypoint entrypoint)
{
	vaInitSurface (ctx,profile,entrypoint);
	gfxInit(ctx);
	setupKHRTexture(ctx);
}
static int ensureProfiles(vaglContext *ctx)
{
    VAProfile *profiles;
    int num_profiles;
    VAStatus status;

    if (ctx->va.profiles && ctx->va.num_profiles > 0)
        return 0;

    num_profiles = vaMaxNumProfiles(ctx->va.display);
    profiles = new VAProfile[num_profiles];
    if (!profiles)
        return ENOMEM;

    status = vaQueryConfigProfiles(ctx->va.display, profiles, &num_profiles);
    if (!checkVAStatus(status, "vaQueryConfigProfiles()")){
		printf("failed to query the set of supported profiles\n");
		delete profiles;
		return status;
	}

    ctx->va.profiles = profiles;
    ctx->va.num_profiles = num_profiles;
    return 0;
}

static bool hasConfig(vaglContext *ctx, VAProfile profile, VAEntrypoint entrypoint)
{
    uint32_t i;

    if (ensureProfiles(ctx) != 0)
        return false;

    for (i = 0; i < ctx->va.num_profiles; i++) {
        if (ctx->va.profiles[i] == profile)
            break;
    }
    if (i == ctx->va.num_profiles)
        return false;
    return true;
}

static enum AVPixelFormat get_format(AVCodecContext *avctx, const enum AVPixelFormat *pixfmts)
{
    VAProfile profiles[3];
    uint32_t i, num_profiles;

    // Find a VA format
    for (i = 0; pixfmts[i] != AV_PIX_FMT_NONE; i++) {
        if (pixfmts[i] == AV_PIX_FMT_VAAPI)
            break;
    }
    if (pixfmts[i] == AV_PIX_FMT_NONE)
        return AV_PIX_FMT_NONE;

    // Find a suitable VA profile that fits FFmpeg config
    num_profiles = 0;
    if (!ffmpeg2vaprofile(avctx->codec_id, avctx->profile,&profiles[num_profiles]))
        return AV_PIX_FMT_NONE;
#if ANDROIDVER==7
profiles[num_profiles]=VAProfileH264ConstrainedBaseline; 
#endif
	
    switch (profiles[num_profiles++]) {
    case VAProfileMPEG2Simple:
        profiles[num_profiles++] = VAProfileMPEG2Main;
        break;
    case VAProfileMPEG4Simple:
        profiles[num_profiles++] = VAProfileMPEG4AdvancedSimple;
        // fall-through
    case VAProfileMPEG4AdvancedSimple:
        profiles[num_profiles++] = VAProfileMPEG4Main;
        break;
    case VAProfileH264ConstrainedBaseline:
        profiles[num_profiles++] = VAProfileH264Main;
        // fall-through
    case VAProfileH264Main:
        profiles[num_profiles++] = VAProfileH264High;
        break;
    case VAProfileVC1Simple:
        profiles[num_profiles++] = VAProfileVC1Main;
        // fall-through
    case VAProfileVC1Main:
        profiles[num_profiles++] = VAProfileVC1Advanced;
        break;
    default:
        break;
    }
    for (i = 0; i < num_profiles; i++) {
        if (hasConfig((vaglContext *)avctx->opaque, profiles[i], VAEntrypointVLD))
            break;
    }
    if (i == num_profiles)
        return AV_PIX_FMT_NONE;
    if (init_decoder((vaglContext *)avctx->opaque, profiles[i], VAEntrypointVLD) < 0)
        return AV_PIX_FMT_NONE;
    return AV_PIX_FMT_VAAPI;
}

static VASurfaceID * acquire_surface(vaglContext * const ctx)
{
    ctx->va.surfacesptr = (ctx->va.surfacesptr + 1) % SURFACE_NUM;
	VASurfaceID *surface=&ctx->va.surfaces[ctx->va.surfacesptr];
	return surface;
}
static int release_surface(vaglContext * const ctx,uint8_t *s)
{
    return 0;
}
static void get_buffer_common(AVCodecContext *avctx, AVFrame *frame, VASurfaceID *s)
{
    memset(frame->data, 0, sizeof(frame->data));
    frame->data[0] = (uint8_t *)(uintptr_t)*s;
    frame->data[3] = (uint8_t *)(uintptr_t)*s;
    memset(frame->linesize, 0, sizeof(frame->linesize));
    frame->linesize[0] = avctx->coded_width; /* XXX: 8-bit per sample only */
    frame->data[5] = (uint8_t *)s;
}

static int get_buffer2(AVCodecContext *avctx, AVFrame *frame, int flags)
{
    vaglContext * const ctx = (vaglContext *)avctx->opaque;
    VASurfaceID *s;
    AVBufferRef *buf;
    int ret;

    if (!(avctx->codec->capabilities & CODEC_CAP_DR1))
        return avcodec_default_get_buffer2(avctx, frame, flags);
	
    s=acquire_surface(ctx);

    buf = av_buffer_create((uint8_t *)s, 0,(void (*)(void *, uint8_t *))release_surface, ctx,AV_BUFFER_FLAG_READONLY);
    if (!buf) {
        release_surface(ctx, (uint8_t *)s);
        return ENOMEM;
    }
    frame->buf[0] = buf;

    get_buffer_common(avctx, frame, s);
    return 0;
}

static void initVAContext(vaglContext *ctx)
{
    AVCodecContext * const avctx = ctx->ff.avctx;

	vaInit (ctx);
    avctx->hwaccel_context = &ctx->ffva; //pass va context to ffmpeg
    avctx->thread_count = 1;
    avctx->draw_horiz_band = 0;
    avctx->slice_flags = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;

    avctx->get_format = get_format;
    avctx->get_buffer2 = get_buffer2;
}
void closeavfile(vaglContext *ctx)
{
   	gfxUninit(ctx);
	vaUninit(ctx);
}
int openavfile(vaglContext *ctx, const char *filename)
{
    AVFormatContext *fmtctx;
    AVCodecContext *avctx;
    AVCodec *codec;
    int i, ret;

    // Open and identify media file
	av_register_all();
    ret = avformat_open_input(&ctx->ff.fmtctx, filename, NULL, NULL);
    if (ret != 0){
		printf("can't open file %s: %d\n", filename,ret);
		return ret;
	}
    ret = avformat_find_stream_info(ctx->ff.fmtctx, NULL);
    if (ret < 0){
		printf("can't find stream info %s: %d\n", filename,ret);
		return ret;
	}
    av_dump_format(ctx->ff.fmtctx, 0, filename, 0);

    // Find the video stream and identify the codec
    for (i = 0; i < ctx->ff.fmtctx->nb_streams; i++) {
        if (ctx->ff.fmtctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && !ctx->ff.stream)
            ctx->ff.stream = ctx->ff.fmtctx->streams[i];
        else
            ctx->ff.fmtctx->streams[i]->discard = AVDISCARD_ALL;
    }
    if (!ctx->ff.stream){
		printf("can't find a video stream\n");
		return AVERROR_STREAM_NOT_FOUND;
	}

	ctx->ff.avctx = ctx->ff.stream->codec;
	ctx->ff.avctx->opaque = ctx;
	initVAContext(ctx);

    ctx->ff.codec = avcodec_find_decoder(ctx->ff.avctx->codec_id);
    if (!ctx->ff.codec){
		printf("can't find codec %d\n",avctx->codec_id);
		return AVERROR_DECODER_NOT_FOUND;
	}
    ret = avcodec_open2(ctx->ff.avctx, ctx->ff.codec, NULL);
    if (ret < 0){
		printf("can't open codec %d\n", avctx->codec_id);
		return ret;
	}

    ctx->ff.frame = av_frame_alloc();
    if (!ctx->ff.frame){
		printf("can't allocate video frame\n");
		return ENOMEM;
	}

    return 0;

}
static VASurfaceID  get_frame_surface(AVCodecContext *avctx, AVFrame *frame)
{
    return (VASurfaceID)*((uint8_t *)frame->data[5]);
}

static int handle_frame(vaglContext *ctx, AVFrame *frame)
{
    VARectangle * const crop_rect = &ctx->ff.crop_rect;
    int data_offset;

    ctx->ff.frame = frame;
    ctx->ff.surface = get_frame_surface(ctx->ff.avctx, frame);

    data_offset = frame->data[0] - frame->data[3];
    ctx->ff.has_crop_rect = data_offset > 0   ||
        frame->width  != ctx->ff.avctx->coded_width ||
        frame->height != ctx->ff.avctx->coded_height;
    crop_rect->x = data_offset % frame->linesize[0];
    crop_rect->y = data_offset / frame->linesize[0];
    crop_rect->width = frame->width;
    crop_rect->height = frame->height;
    return 0;
}

static int decode_packet(vaglContext *ctx, AVPacket *packet, int *got_frame_ptr)
{
    char errbuf[BUFSIZ];
    int got_frame, ret;

    if (!got_frame_ptr)
        got_frame_ptr = &got_frame;

    ret = avcodec_decode_video2(ctx->ff.avctx, ctx->ff.frame, got_frame_ptr, packet);
    if (ret < 0){
		printf("failed to decode frame: %d\n",ret);
		return ret;
	}
    if (*got_frame_ptr)
        return handle_frame(ctx, ctx->ff.frame);
    return EAGAIN;
}
static int decoder_run(vaglContext *ctx)
{
    AVPacket packet;
    int got_frame, ret;

    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;

    do {
        // Read frame from file
        ret = av_read_frame(ctx->ff.fmtctx, &packet);
        if (ret == AVERROR_EOF)
            break;
        else if (ret < 0){
			printf("failed to read frame: %d\n",ret);
			return ret;
		}

        // Decode video packet
        if (packet.stream_index == ctx->ff.stream->index)
            ret = decode_packet(ctx, &packet, NULL);
        else
            ret = EAGAIN;
        av_free_packet(&packet);
    } while (ret == EAGAIN);
    if (ret == 0)
        return 0;
	if (ret == AVERROR_EOF)
		return ret;

    // Decode cached frames
    packet.data = NULL;
    packet.size = 0;
    ret = decode_packet(ctx, &packet, &got_frame);
    if (ret == EAGAIN && !got_frame)
        ret = AVERROR_EOF;
    return ret;

}

static int decoder_get_frame(vaglContext *ctx, AVFrame **out_frame_ptr)
{
    int ret;

    ret = decoder_run(ctx);
    if (ret == 0 && out_frame_ptr)
        *out_frame_ptr = ctx->ff.frame;

    return ret;
}

bool renderer_put_surface(vaglContext *ctx)
{
	renderFrame(ctx);
    return true;
}


static bool render_surface(vaglContext *ctx, VASurfaceID s, const VARectangle *rect,uint32_t flags)
{
	ctx->gl.width=rect->width;
	ctx->gl.height=rect->height;

    return renderer_put_surface(ctx);
}

static int render_frame(vaglContext *ctx, AVFrame *frame)
{
    VASurfaceID  const s = ctx->ff.surface;
    const VARectangle *rect;
    VARectangle tmp_rect;
    uint32_t i, flags;

    if (ctx->ff.has_crop_rect)
        rect = &ctx->ff.crop_rect;
    else {
        tmp_rect.x = 0;
        tmp_rect.y = 0;
        tmp_rect.width = ctx->ff.crop_rect.width;
        tmp_rect.height = ctx->ff.crop_rect.height;
        rect = &tmp_rect;
    }
    flags = 0;
    for (i = 0; i < 1 + !!frame->interlaced_frame; i++) {
        flags &= ~(VA_TOP_FIELD|VA_BOTTOM_FIELD);
        if (frame->interlaced_frame) {
            flags |= ((i == 0) ^ !!frame->top_field_first) == 0 ?
                VA_TOP_FIELD : VA_BOTTOM_FIELD;
        }
        if (!render_surface(ctx, s, rect, flags))
            return AVERROR_UNKNOWN;
                
    }
    return 0;
}

int decode_frame(vaglContext *ctx)
{
    AVFrame *frame;
    int ret;

	ret=decoder_get_frame(ctx, &frame);
    if (ret == 0) {
        ret = render_frame(ctx, frame);
    }
    return ret;
}


