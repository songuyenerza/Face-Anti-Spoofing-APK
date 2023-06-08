package com.example.test

import android.graphics.Rect
import androidx.databinding.BaseObservable
import com.mv.engine.FaceBox

class DetectionResult(): BaseObservable() {

    var left: Int = 0

    var top: Int = 0

    var right: Int = 0

    var bottom: Int = 0

    var confidence: Float = 0.toFloat()

    var time: Long = 0

    var threshold: Float = 0F

    var hasFace: Boolean = false

    var conf_: Float = 0.toFloat()

    constructor(faceBox: FaceBox, time: Long, hasFace: Boolean) : this() {
        this.left = faceBox.left
        this.top = faceBox.top
        this.right = faceBox.right
        this.bottom = faceBox.bottom
        this.confidence = faceBox.confidence
        this.time = time
        this.hasFace = hasFace
        this.conf_ = faceBox.confidence

    }

    fun updateLocation(rect: Rect): DetectionResult {
        this.left = rect.left
        this.top = rect.top
        this.right = rect.right
        this.bottom = rect.bottom

        return this
    }


}

