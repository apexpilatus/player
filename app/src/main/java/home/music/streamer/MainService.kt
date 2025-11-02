package home.music.streamer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.graphics.BitmapFactory
import android.media.AudioAttributes
import android.media.AudioManager
import android.media.MediaPlayer
import android.os.IBinder
import java.io.BufferedReader
import java.io.InputStreamReader
import java.net.ServerSocket
import java.net.URL

const val CHANNEL_ID = "main"
const val CHANNEL_NAME = "main"

class MainService : Service(), MediaPlayer.OnCompletionListener {
    private val sockServer by lazy { ServerSocket(9696) }
    private val notificationManager by lazy { getSystemService(NOTIFICATION_SERVICE) as NotificationManager }
    private val audioManager by lazy { getSystemService(AUDIO_SERVICE) as AudioManager }
    private val player by lazy {
        MediaPlayer().apply {
            setAudioAttributes(
                AudioAttributes.Builder().setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                    .setUsage(AudioAttributes.USAGE_MEDIA).build()
            )
            setOnCompletionListener(this@MainService)
        }
    }

    companion object {
        @Volatile
        var started = false
    }

    private val listen = Thread {
        var url: String
        var picture: String
        while (true) {
            try {
                sockServer.accept().use {
                    audioManager.mode = AudioManager.MODE_RINGTONE
                    url = BufferedReader(InputStreamReader(it.getInputStream())).readText()
                    picture =
                        if (url.contains("stream_cd")) url.split("stream_cd")[0] + "apple-touch-icon.png"
                        else url.split("stream_album?")[0] + url.split("?/")[1].split("&")[0]
                    audioManager.mode = AudioManager.MODE_NORMAL
                    with(player) {
                        reset()
                        setDataSource(url)
                        prepare()
                        start()
                    }
                    URL(picture).openStream().use { picture ->
                        notificationManager.notify(
                            1,
                            Notification.Builder(this, CHANNEL_ID)
                                .setSmallIcon(R.drawable.ic_notification)
                                .setLargeIcon(BitmapFactory.decodeStream(picture))
                                .setOnlyAlertOnce(true).setShowWhen(false).setContentText("")
                                .build()
                        )
                    }
                }
            } catch (_: Exception) {
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
        listen.start()
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    override fun onDestroy() {
        sockServer.close()
        started = false
    }

    override fun onCompletion(mp: MediaPlayer?) {
        notificationManager.notify(
            1,
            Notification.Builder(this, CHANNEL_ID).setSmallIcon(R.drawable.ic_notification)
                .setOnlyAlertOnce(true).setShowWhen(false).setContentText("").build()
        )
    }
}
