package home.music.streamer

import android.content.Context
import android.content.Context.AUDIO_SERVICE
import android.media.AudioAttributes
import android.media.AudioDeviceCallback
import android.media.AudioDeviceInfo
import android.media.AudioManager
import android.media.AudioManager.STREAM_MUSIC
import android.media.AudioMixerAttributes
import java.io.OutputStream

class Mixer(val context: Context) : AudioDeviceCallback() {
    private val audioManager by lazy { context.getSystemService(AUDIO_SERVICE) as AudioManager }
    val audioAttr: AudioAttributes by lazy {
        AudioAttributes.Builder().setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
            .setUsage(AudioAttributes.USAGE_MEDIA).build()
    }

    fun getCards(writer: OutputStream) {
        val resp =
            "HTTP/1.1 200 android\r\nContent-Type: text/html; charset=utf-8\r\nCache-control: no-cache\r\nX-Content-Type-Options: nosniff\r\n\r\n"
        writer.write(resp.toByteArray(), 0, resp.length)
        writer.flush()
    }

    fun getVolume(writer: OutputStream) {
        val cardName = "music"
        val resp =
            "HTTP/1.1 200 ${audioManager.getStreamMinVolume(STREAM_MUSIC)};${
                audioManager.getStreamVolume(
                    STREAM_MUSIC
                )
            };${audioManager.getStreamMaxVolume(STREAM_MUSIC)}\r\nContent-Length: ${cardName.length}\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n$cardName"
        writer.write(resp.toByteArray(), 0, resp.length)
        writer.flush()
    }

    fun setVolume(req: String, writer: OutputStream) {
        var value = 0
        for (param in req.split("?")[1].split("&")) {
            if (param.startsWith("value=")) {
                value = param.split("=")[1].toInt()
            }
        }
        audioManager.setStreamVolume(STREAM_MUSIC, value, 0)
        val resp =
            "HTTP/1.1 200 $value\r\nContent-Type: text/html; charset=utf-8\r\nCache-control: no-cache\r\nX-Content-Type-Options: nosniff\r\n\r\n"
        writer.write(resp.toByteArray(), 0, resp.length)
        writer.flush()
    }

    fun registerCallBack() {
        audioManager.registerAudioDeviceCallback(this@Mixer, null)
    }

    override fun onAudioDevicesAdded(addedDevices: Array<out AudioDeviceInfo?>?) {
        if (addedDevices != null) {
            for (device in addedDevices) {
                if (device?.type == AudioDeviceInfo.TYPE_USB_HEADSET) {
                    var prefMixerAttr: AudioMixerAttributes? = null
                    audioManager.clearPreferredMixerAttributes(audioAttr, device)
                    for (mixerAttr in audioManager.getSupportedMixerAttributes(device)) {
                        with(mixerAttr.format) {
                            if (prefMixerAttr == null || (frameSizeInBytes >= prefMixerAttr.format.frameSizeInBytes && sampleRate >= prefMixerAttr.format.sampleRate))
                                prefMixerAttr = mixerAttr
                        }
                    }
                    if (prefMixerAttr != null)
                        audioManager.setPreferredMixerAttributes(audioAttr, device, prefMixerAttr)
                    break
                }
            }
        }
    }
}