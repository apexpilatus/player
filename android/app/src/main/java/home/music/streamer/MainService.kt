package home.music.streamer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.media.AudioManager
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

class MainService : Service(), AudioManager.OnModeChangedListener {
    private val sockServer by lazy { ServerSocket(8888) }
    private val mixer by lazy { Mixer(this) }
    private val player by lazy { ExoPlayer.Builder(this).build() }
    private val notificationManager by lazy { getSystemService(NOTIFICATION_SERVICE) as NotificationManager }
    private var album = ""
    private var track = 0
    private lateinit var files: List<String>

    companion object {
        @Volatile
        var started = false
    }

    private fun handle() {
        val proxy = Proxy(this)
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
                        mixer.audioManager.mode = AudioManager.MODE_RINGTONE
                        for (param in url.split("?")[1].split("&")) {
                            if (param.startsWith("album=")) {
                                album = param.split("=")[1]
                            }
                            if (param.startsWith("track=")) {
                                track = param.split("=")[1].toInt()
                            }
                        }
                        val ip = getSharedPreferences(PREFS_FILE, MODE_PRIVATE).getString(
                            PREF_IP,
                            "1.2.3.4."
                        )
                        files = URL(
                            "http://$ip/files?album=$album"
                        ).readText().split("\r\n")
                        mixer.audioManager.mode = AudioManager.MODE_NORMAL
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

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        started = true
        this.startForeground(
            1,
            Notification.Builder(this, CHANNEL_ID).setSmallIcon(R.drawable.ic_notification)
                .setShowWhen(false).setContentText("").build()
        )
        mixer.audioManager.addOnModeChangedListener(this.mainExecutor, this@MainService)
        CoroutineScope(Job()).launch {
            handle()
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

    override fun onModeChanged(mode: Int) {
        if (mode == AudioManager.MODE_NORMAL) {
            mixer.audioManager.registerAudioDeviceCallback(mixer, null)
            val ip = getSharedPreferences(PREFS_FILE, MODE_PRIVATE).getString(PREF_IP, "1.2.3.4")
            player.clearMediaItems()
            player.setMediaItem(MediaItem.fromUri("http://$ip/fetch?album=$album&file=${files[track]}"))
            while (++track < files.size)
                player.addMediaItem(MediaItem.fromUri("http://$ip/fetch?album=$album&file=${files[track]}"))
            mixer.audioManager.unregisterAudioDeviceCallback(mixer)
            player.prepare()
            player.play()
        }
    }
}
