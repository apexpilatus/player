package home.music.streamer

import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import home.music.streamer.MainService.Companion.stopPlaying
import java.io.InputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder


class Streamer(private val reader: InputStream) {

    private var bits: Short = 0
    private var rate: Int = 0
    private var bytesLeft: Int = 0

    private fun readHeader(): Boolean {
        reader.skip(24)
        var arr = reader.readNBytes(4)
        if (arr.size != 4) return false
        rate = ByteBuffer.wrap(arr).order(ByteOrder.LITTLE_ENDIAN).getInt()
        reader.skip(6)
        arr = reader.readNBytes(2)
        if (arr.size != 2) return false
        bits = ByteBuffer.wrap(arr).order(ByteOrder.LITTLE_ENDIAN).getShort()
        if (bits == 16.toShort()) {
            reader.skip(4)
            arr = reader.readNBytes(4)
            if (arr.size == 4) bytesLeft =
                ByteBuffer.wrap(arr).order(ByteOrder.LITTLE_ENDIAN).getInt()
        }
        if (bits == 24.toShort()) {
            reader.skip(28)
            arr = reader.readNBytes(4)
            if (arr.size == 4) bytesLeft =
                ByteBuffer.wrap(arr).order(ByteOrder.LITTLE_ENDIAN).getInt()
        }
        return bytesLeft != 0
    }

    fun play() {
        if (readHeader()) {
            val format = AudioFormat.Builder()
                .setEncoding(if (bits == 16.toShort()) AudioFormat.ENCODING_PCM_16BIT else AudioFormat.ENCODING_PCM_24BIT_PACKED)
                .setSampleRate(rate).setChannelMask(AudioFormat.CHANNEL_OUT_STEREO).build()
            val audioAttributes = AudioAttributes.Builder().setUsage(AudioAttributes.USAGE_MEDIA)
                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC).build()
            val player =
                AudioTrack.Builder().setAudioAttributes(audioAttributes).setAudioFormat(format)
                    .build()
            val channels = 2
            val bufSize = (bits / 8) * channels * 1000
            val arr = ByteArray(bufSize)
            var readSize: Int
            with(player) {
                while (!stopPlaying) {
                    readSize = reader.readNBytes(arr, 0, bufSize)
                    if (readSize < bufSize && readSize < bytesLeft) {
                        break
                    }
                    write(ByteBuffer.wrap(arr), readSize, AudioTrack.WRITE_BLOCKING)
                    if (playState != AudioTrack.PLAYSTATE_PLAYING) play()
                    bytesLeft -= readSize
                    if (bytesLeft == 0) break
                }
                stop()
                release()
            }
        }
    }
}