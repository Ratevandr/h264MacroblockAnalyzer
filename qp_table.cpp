/*
  * This file is part of FFmpeg.
  *
  * FFmpeg is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * FFmpeg is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License along
  * with FFmpeg; if not, write to the Free Software Foundation, Inc.,
  * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
  */

 #include <stdint.h>

 // for FF_QSCALE_TYPE_*
extern "C" {
// #include "libavcodec/internal.h"

 #include "libavutil/frame.h"

 #include "libavutil/mem.h"

 #include "video_enc_params.h"
}
 #include "qp_table.h"

#define AV_FRAME_DATA_VIDEO_ENC_PARAMS 21

//AVFrameSideData *av_frame_get_side_data(const AVFrame *frame,
//                                          int type)
//  {
//      int i;

//      for (i = 0; i < frame->nb_side_data; i++) {
//          if (frame->side_data[i]->type == type)
//              return frame->side_data[i];
//      }
//      return NULL;
//  }
struct qp_properties {
       int stride;
       int type;
   };



 int ff_qp_table_extract(AVFrame *frame, int8_t **table, int *table_w, int *table_h,
                         int *qscale_type)
 {
     AVFrameSideData *sd;
     AVVideoEncParams *par;
     unsigned int mb_h = (frame->height + 15) / 16;
       int mb_w = (frame->width + 15) / 16;
     const unsigned int nb_mb = mb_h * mb_w;
     unsigned int block_idx;

     *table = NULL;
   //   par = av_video_enc_params_create_side_data(frame,AV_VIDEO_ENC_PARAMS_H264,nb_mb);
//sd = av_frame_new_side_data(frame, AV_FRAME_DATA_QP_TABLE_DATA,sizeof(qp_properties));
   int kek =  frame->nb_side_data;
   sd = av_frame_get_side_data(frame, AV_FRAME_DATA_QP_TABLE_DATA);
     if (!sd)
         return 0;
     par = (AVVideoEncParams*)sd->data;
//     if (par->type != AV_VIDEO_ENC_PARAMS_MPEG2 ||
//         (par->nb_blocks != 0 && par->nb_blocks != nb_mb))
//         return AVERROR(ENOSYS);

     void* voidTable = av_malloc(nb_mb);
     *table = (int8_t *)voidTable;
     if (!*table)
         return AVERROR(ENOMEM);
     if (table_w)
         *table_w = mb_w;
     if (table_h)
         *table_h = mb_h;
     if (qscale_type)
         *qscale_type = AV_VIDEO_ENC_PARAMS_H264;

     if (par->nb_blocks == 0) {
         memset(*table, par->qp, nb_mb);
         return 0;
     }


     for (block_idx = 0; block_idx < nb_mb; block_idx++) {
         AVVideoBlockParams *b = av_video_enc_params_block(par, block_idx);
         (*table)[block_idx] = par->qp + b->delta_qp;
     }

     return 0;
 }
