//
// Created by Rayan on 2018/12/29.
//

#include <jni.h>
#include <stdio.h>
#include <errno.h>

#include "log.h"
#include "webp/decode.h"
#include "webp/encode.h"
#include "examples/anim_util.h"
#include "examples/unicode.h"
#include "imageio/image_enc.h"


AnimatedImage image;
jboolean isInitialized = JNI_FALSE;

JNIEXPORT jint JNICALL Java_com_google_webp_libwebpAnimJNI_WebPAnimInit(JNIEnv *jenv, jclass jcls) {
    jint jresult = 0 ;
    if (isInitialized == JNI_FALSE) {
        memset(&image, 0, sizeof(image));
        isInitialized = JNI_TRUE;
    } else {
        jresult = -1;
    }

    return jresult;
}


JNIEXPORT jint JNICALL Java_com_google_webp_libwebpAnimJNI_WebPAnimDecodeRGBA(JNIEnv *jenv,
                                                                              jclass jcls,
                                                                              jbyteArray jarg1,
                                                                              jintArray jarg2,
                                                                              jintArray jarg3,
                                                                              jintArray jarg4) {
    if (isInitialized) {
        memset(&image, 0, sizeof(image));
    } else {
        return -1;
    }

    int temp2 ;
    int temp3 ;
    int temp4 ;

    jbyte* dataPtr = (*jenv)->GetByteArrayElements(jenv, jarg1, NULL);
    int dataLen = (int)((*jenv)->GetArrayLength(jenv, jarg1));
    LOGD("data length: %d \n", dataLen);
    if (!ReadAnimatedImage2((const char*)dataPtr, dataLen, &image)) {
        LOGE("Error decoding file.\n Aborting.\n");
        return -2;
    } else {
        temp2 = image.canvas_width;
        temp3 = image.canvas_height;
        temp4 = image.num_frames;
        {
            jint jvalue = (jint)temp2;
            (*jenv)->SetIntArrayRegion(jenv, jarg2, 0, 1, &jvalue);
        }
        {
            jint jvalue = (jint)temp3;
            (*jenv)->SetIntArrayRegion(jenv, jarg3, 0, 1, &jvalue);
        }
        {
            jint jvalue = (jint)temp4;
            (*jenv)->SetIntArrayRegion(jenv, jarg4, 0, 1, &jvalue);
        }
    }

    return 0;
}


JNIEXPORT jbyteArray JNICALL Java_com_google_webp_libwebpAnimJNI_WebPGetDecodedFrame(JNIEnv *jenv, jclass jcls, jint jarg1) {
    size_t size = image.canvas_width * sizeof(uint32_t) * image.canvas_height;
    jbyteArray jresult = (*jenv)->NewByteArray(jenv, size);
    (*jenv)->SetByteArrayRegion(jenv, jresult, 0, size, image.frames[jarg1].rgba);

    return jresult;
}


JNIEXPORT jint JNICALL Java_com_google_webp_libwebpAnimJNI_WebPSaveImage(JNIEnv *jenv, jclass jcls, jstring jarg1) {
    const W_CHAR* prefix = TO_W_CHAR("dump_");
    const W_CHAR* suffix = TO_W_CHAR("bmp");
    WebPOutputFileFormat format = BMP;
    const W_CHAR* dump_folder = (*jenv)->GetStringUTFChars(jenv, jarg1, 0);

    for (int i = 0; i < image.num_frames; ++i) {
        W_CHAR out_file[1024];
        WebPDecBuffer buffer;
        WebPInitDecBuffer(&buffer);
        buffer.colorspace = MODE_RGBA;
        buffer.is_external_memory = 1;
        buffer.width = image.canvas_width;
        buffer.height = image.canvas_height;
        buffer.u.RGBA.rgba = image.frames[i].rgba;
        buffer.u.RGBA.stride = buffer.width * sizeof(uint32_t);
        buffer.u.RGBA.size = buffer.u.RGBA.stride * buffer.height;
        WSNPRINTF(out_file, sizeof(out_file), "%s/%s%.4d.%s",
                  dump_folder, prefix, i, suffix);
        if (!WebPSaveImage(&buffer, format, (const char*)out_file)) {
            LOGE("Error while saving image '%s'\n", out_file);
            WFPRINTF(stderr, "Error while saving image '%s'\n", out_file);
        }
        WebPFreeDecBuffer(&buffer);
    }

    return 0;
}


JNIEXPORT jint JNICALL Java_com_google_webp_libwebpAnimJNI_WebPAnimRelease(JNIEnv *jenv, jclass jcls) {
    isInitialized = JNI_FALSE;
    ClearAnimatedImage(&image);

    return 0;
}
