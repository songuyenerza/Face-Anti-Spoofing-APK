#include <jni.h>
#include <string>
#include <opencl-c-base.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_testjni_MainActivity_stringFromJNI(
    JNIEnv *env,
    jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


uchar *YUV_420_888toNV21(
    jbyteArray yBuf,
    jbyteArray uBuf,
    jbyteArray vBuf,
    jbyte *fullArrayNV21,
    int width,
    int height,
    int yRowStride,
    int yPixelStride,
    int uRowStride,
    int uPixelStride,
    int vRowStride,
    int vPixelStride,
    JNIEnv *env) {

    /* Check that our frame has right format, as specified at android docs for
     * YUV_420_888 (https://developer.android.com/reference/android/graphics/ImageFormat?authuser=2#YUV_420_888):
     *      - Plane Y not overlaped with UV, and always with pixelStride = 1
     *      - Planes U and V have the same rowStride and pixelStride (overlaped or not)
     */
    if (yPixelStride != 1 || uPixelStride != vPixelStride || uRowStride != vRowStride) {
        jclass Exception = env->FindClass("java/lang/Exception");
        env->ThrowNew(Exception,
                      "Invalid YUV_420_888 byte structure. Not agree with https://developer.android.com/reference/android/graphics/ImageFormat?authuser=2#YUV_420_888"
        );
    }

    int ySize = width * height;
    int uSize = env->GetArrayLength(uBuf);
    int vSize = env->GetArrayLength(vBuf);
    int newArrayPosition = 0; //Posicion por la que vamos rellenando el array NV21
    if (fullArrayNV21 == nullptr) {
        fullArrayNV21 = new jbyte[ySize + uSize + vSize];
    }
    if (yRowStride == width) {
        //Best case. No padding, copy direct
        env->GetByteArrayRegion(yBuf, newArrayPosition, ySize, fullArrayNV21);
        newArrayPosition = ySize;
    } else {
        // Padding at plane Y. Copy Row by Row
        long yPlanePosition = 0;
        for (; newArrayPosition < ySize; newArrayPosition += width) {
            env->GetByteArrayRegion(yBuf, yPlanePosition, width, fullArrayNV21 + newArrayPosition);
            yPlanePosition += yRowStride;
        }
    }

    // Check UV channels in order to know if they are overlapped (best case)
    // If they are overlapped, U and B first bytes are consecutives and pixelStride = 2
    long uMemoryAdd = (long) &uBuf;
    long vMemoryAdd = (long) &vBuf;
    long diff = std::abs(uMemoryAdd - vMemoryAdd);
    if (vPixelStride == 2 && diff == 8) {
        if (width == vRowStride) {
            // Best Case: Valid NV21 representation (UV overlapped, no padding). Copy direct
            env->GetByteArrayRegion(uBuf, 0, uSize, fullArrayNV21 + ySize);
            env->GetByteArrayRegion(vBuf, 0, vSize, fullArrayNV21 + ySize + uSize);
        } else {
            // UV overlapped, but with padding. Copy row by row (too much performance improvement compared with copy byte-by-byte)
            int limit = height / 2 - 1;
            for (int row = 0; row < limit; row++) {
                env->GetByteArrayRegion(
                    /*array=*/ uBuf,
                    /*start=*/ row * vRowStride,
                    /*len=*/ width,
                    /*buf=*/ fullArrayNV21 + ySize + (row * width)
                );
            }
        }
    } else {
        //WORST: not overlapped UV. Copy byte by byte
        int rowLen = height / 2;
        int colLen = width / 2;
        for (int row = 0; row < rowLen; ++row) {
            for (int col = 0; col < colLen; ++col) {
                int vuPos = col * uPixelStride + row * uRowStride;
                env->GetByteArrayRegion(vBuf, vuPos, 1, fullArrayNV21 + newArrayPosition);
                ++newArrayPosition;
                env->GetByteArrayRegion(uBuf, vuPos, 1, fullArrayNV21 + newArrayPosition);
                ++newArrayPosition;
            }
        }
    }
    return (uchar *) fullArrayNV21;
}