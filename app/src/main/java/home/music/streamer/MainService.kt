package home.music.streamer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.graphics.BitmapFactory
import android.media.AudioAttributes
import android.media.MediaPlayer
import android.os.IBinder
import java.io.BufferedReader
import java.io.InputStreamReader
import java.net.ServerSocket
import java.net.URL
import java.util.concurrent.ConcurrentLinkedQueue

const val CHANNEL_ID = "main"
const val CHANNEL_NAME = "main"

class MainService : Service(), MediaPlayer.OnCompletionListener {
    private val sockServer by lazy { ServerSocket(9696) }
    private val notificationManager by lazy { getSystemService(NOTIFICATION_SERVICE) as NotificationManager }

    companion object {
        @Volatile
        var started = false

        private val refs = ConcurrentLinkedQueue<String>()
        private val players = ConcurrentLinkedQueue<MediaPlayer>()
    }

    private val updateStatus = Runnable {
        var remoteIP = ""
        while (true) {
            try {
                sockServer.accept().use {
                    if (remoteIP != it.remoteSocketAddress.toString().replace("/", "")
                            .split(":")[0]
                    ) remoteIP = it.remoteSocketAddress.toString().replace("/", "").split(":")[0]
                    val msg = BufferedReader(InputStreamReader(it.getInputStream())).readLine()
                    for (player in players) player.reset()
                    refs.clear()
                    for (ref in msg.split("|")) refs.add("http://$remoteIP$ref")
                }
                with(players.peek() as MediaPlayer) {
                    setDataSource(refs.poll())
                    prepare()
                    start()
                    val ref = refs.poll()
                    if (ref != null) {
                        with(players.last()) {
                            setDataSource(ref)
                            prepare()
                        }
                        setNextMediaPlayer(players.last())
                    } else {
                        players.poll()
                        players.add(this)
                    }
                }
                URL("http://$remoteIP/apple-touch-icon.png").openStream().use {
                    notificationManager.notify(
                        1,
                        Notification.Builder(this, CHANNEL_ID)
                            .setSmallIcon(R.drawable.ic_notification)
                            .setLargeIcon(BitmapFactory.decodeStream(it)).setOnlyAlertOnce(true)
                            .setShowWhen(false).setContentText("").build()
                    )
                }
            } catch (_: Exception) {
                for (player in players) player.reset()
                refs.clear()
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
        for (i in 1..2) players.add(MediaPlayer().apply {
            setAudioAttributes(
                AudioAttributes.Builder().setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                    .setUsage(AudioAttributes.USAGE_MEDIA).build()
            )
            setOnCompletionListener(this@MainService)
        })
        Thread(updateStatus).start()
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
        if (mp === players.last()) notificationManager.notify(
            1,
            Notification.Builder(this, CHANNEL_ID).setSmallIcon(R.drawable.ic_notification)
                .setOnlyAlertOnce(true).setShowWhen(false).setContentText("").build()
        ) else {
            val ref = refs.poll()
            if (ref != null) with(players.poll() as MediaPlayer) {
                reset()
                try {
                    setDataSource(ref)
                    prepare()
                    players.last().setNextMediaPlayer(this)
                } catch (_: Exception) {
                }
                players.add(this)
            }
        }
    }
}