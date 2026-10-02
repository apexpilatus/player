package home.music.streamer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.media.AudioAttributes
import android.media.AudioDeviceInfo
import android.media.AudioManager
import android.media.AudioMixerAttributes
import android.os.IBinder
import androidx.media3.common.MediaItem
import androidx.media3.exoplayer.ExoPlayer
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import java.io.InputStreamReader
import java.io.OutputStreamWriter
import java.net.ServerSocket
import java.net.URL

const val CHANNEL_ID = "main"
const val CHANNEL_NAME = "main"
const val PREFS_FILE = "prefs"
const val PREF_IP = "ip"

class MainService : Service() {
    private val notificationManager by lazy { getSystemService(NOTIFICATION_SERVICE) as NotificationManager }
    private val audioManager by lazy { getSystemService(AUDIO_SERVICE) as AudioManager }
    private val player by lazy { ExoPlayer.Builder(this).build() }
    private val sockServer = ServerSocket(8888)

    companion object {
        @Volatile
        var started = false
    }

    private fun handle() {
        val proxy = Proxy(this)
        val mixer = Mixer(audioManager)
        while (true) {
            val connection = sockServer.accept()
            var req = ""
            try {
                val reader = InputStreamReader(connection.getInputStream())
                val buf = CharArray(1)
                while (req.length < 4 || req.substring(req.length - 4) != "\r\n\r\n") {
                    reader.read(buf, 0, 1)
                    req += String(buf)
                }
                val url = req.split("\r\n")[0].split(" ")[1]
                when (url.split("?")[0]) {
                    "/setip" -> {
                        proxy.setIp(url, connection.getOutputStream())
                        connection.close()
                        continue
                    }

                    "/getcards" -> {
                        mixer.getCards(connection.getOutputStream())
                        connection.close()
                        continue
                    }

                    "/getvolume" -> {
                        mixer.getVolume(connection.getOutputStream())
                        connection.close()
                        continue
                    }

                    "/setvolume" -> {
                        mixer.setVolume(url, connection.getOutputStream())
                        connection.close()
                        continue
                    }

                    "/stream" -> {
                        val resp =
                            "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nCache-control: no-cache\r\nX-Content-Type-Options: nosniff\r\n\r\n"
                        val writer = OutputStreamWriter(connection.getOutputStream())
                        writer.write(resp, 0, resp.length)
                        writer.flush()
                        var album = ""
                        var track = ""
                        for (param in url.split("?")[1].split("&")) {
                            if (param.startsWith("album=")) {
                                album = param.split("=")[1]
                            }
                            if (param.startsWith("track=")) {
                                track = param.split("=")[1]
                            }
                        }
                        val ip = getSharedPreferences(PREFS_FILE, MODE_PRIVATE).getString(
                            PREF_IP,
                            "1.2.3.4."
                        )
                        val files = URL(
                            "http://$ip/files?album=$album"
                        ).readText()
                        checkUSB()
                        startForegroundService(Intent(this, MainService::class.java).apply {
                            putExtra("album", album)
                            putExtra("track", track)
                            putExtra("files", files)
                        })
                        connection.close()
                        continue
                    }
                }
            } catch (_: Exception) {
                connection.close()
                continue
            }
            CoroutineScope(Job()).launch {
                try {
                    val url = req.split("\r\n")[0].split(" ")[1]
                    proxy.forwardIfConnected(url, connection.getOutputStream())
                } catch (_: Exception) {
                } finally {
                    connection.close()
                }
            }
        }
    }

    override fun onCreate() {
        notificationManager.createNotificationChannel(
            NotificationChannel(
                CHANNEL_ID, CHANNEL_NAME, NotificationManager.IMPORTANCE_DEFAULT
            )
        )
    }

    private fun checkUSB() {
        for (devevice in audioManager.getDevices(AudioManager.GET_DEVICES_OUTPUTS))
            if (devevice.type == AudioDeviceInfo.TYPE_USB_HEADSET) {
                val attr: AudioAttributes by lazy {
                    AudioAttributes.Builder().setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                        .setUsage(AudioAttributes.USAGE_MEDIA).build()
                }
                audioManager.clearPreferredMixerAttributes(attr, devevice)//if (audioManager.getPreferredMixerAttributes(attr, devevice) == null) {
                    var prefMixerAttr: AudioMixerAttributes? = null
                    for (mixerAttr in audioManager.getSupportedMixerAttributes(devevice)) {
                        with(mixerAttr.format) {
                            if (prefMixerAttr == null || (frameSizeInBytes >= prefMixerAttr.format.frameSizeInBytes && sampleRate >= prefMixerAttr.format.sampleRate))
                                prefMixerAttr = mixerAttr
                        }
                    }
                    if (prefMixerAttr != null)
                        audioManager.setPreferredMixerAttributes(
                            attr,
                            devevice,
                            prefMixerAttr
                        )
                    break
                //}
            }
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        val album = intent?.getParcelableExtra("album", String::class.java)
        val track = intent?.getParcelableExtra("track", String::class.java)
        val files = intent?.getParcelableExtra("files", String::class.java)
        if (album != null && track != null && files != null) {
            player.stop()
            player.clearMediaItems()
            val ip = getSharedPreferences(PREFS_FILE, MODE_PRIVATE).getString(PREF_IP, "1.2.3.4")
            val filesList = files.split("\r\n")
            var trackNum = track.toInt()
            player.setMediaItem(MediaItem.fromUri("http://$ip/fetch?album=$album&file=${filesList[trackNum]}"))
            while (++trackNum < filesList.size)
                player.addMediaItem(MediaItem.fromUri("http://$ip/fetch?album=$album&file=${filesList[trackNum]}"))
            player.prepare()
            player.play()
        } else {
            started = true
            this.startForeground(
                1,
                Notification.Builder(this, CHANNEL_ID).setSmallIcon(R.drawable.ic_notification)
                    .setShowWhen(false).setContentText("").build()
            )
            CoroutineScope(Job()).launch {
                handle()
            }
        }
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    override fun onDestroy() {
        sockServer.close()
        started = false
    }
}
