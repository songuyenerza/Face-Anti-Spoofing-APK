package com.example.test

import android.content.res.AssetManager
import android.util.Log
import com.mv.engine.FaceBox
import com.mv.engine.FaceDetector
import com.mv.engine.Live


class EngineWrapper(private var assetManager: AssetManager) {

    private var faceDetector: FaceDetector = FaceDetector()
    private var live: Live = Live()

    fun init(): Boolean {
        var ret = faceDetector.loadModel(assetManager)
        if (ret == 0) {
            ret = live.loadModel(assetManager)
            return ret == 0
        }

        return false
    }

    fun destroy() {
        faceDetector.destroy()
        live.destroy()
    }

    fun detect(yuv: ByteArray, width: Int, height: Int, orientation: Int): List<DetectionResult> {
        val begin = System.currentTimeMillis()

        val results = mutableListOf<DetectionResult>()
        var boxes = detectFace(yuv, width, height, orientation)
        Log.d("soncheck_input", "check width img=${boxes.size}")

        /// get one box face
        boxes = boxes.take(1)

        boxes.forEach {

            val box = it.apply {
                val c = detectLive(yuv, width, height, orientation, this)
                confidence = c


            }
            val end = System.currentTimeMillis()
            val result = DetectionResult(box, end - begin, true)
            results.add(result)
        }

        return results
    }

    private fun detectFace(
        yuv: ByteArray,
        width: Int,
        height: Int,
        orientation: Int
    ): List<FaceBox> = faceDetector.detect(yuv, width, height, orientation)

    private fun detectLive(
        yuv: ByteArray,
        width: Int,
        height: Int,
        orientation: Int,
        faceBox: FaceBox
    ): Float = live.detect(yuv, width, height, orientation, faceBox)

}