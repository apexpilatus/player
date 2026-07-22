package home.music.streamer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.media.AudioAttributes
import android.media.MediaPlayer
import android.os.IBinder
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import java.io.InputStreamReader
import java.io.OutputStreamWriter
import java.net.ServerSocket
import java.net.Socket
import java.net.URL
import java.util.LinkedList

const val CHANNEL_ID = "main"
const val CHANNEL_NAME = "main"
const val PREFS_FILE = "prefs"
const val PREF_IP = "ip"

class MainService : Service(), MediaPlayer.OnCompletionListener {
    private val sockServer by lazy { ServerSocket(8888) }
    private val notificationManager by lazy { getSystemService(NOTIFICATION_SERVICE) as NotificationManager }

    companion object {
        @Volatile
        var started = false

        @Volatile
        var track = 1

        var album = ""
        private val players = LinkedList<MediaPlayer>()
    }

    @Synchronized
    private fun album(new: String? = null): String? {
        if (new == null)
            return album
        else {
            album = new
            return null
        }
    }

    private fun prepareMedia(player: MediaPlayer) {
        val ref = "http://${
            getSharedPreferences(PREFS_FILE, MODE_PRIVATE).getString(
                PREF_IP,
                "1.2.3.4"
            )
        }/fetch?album=${album()}&track=$track"
        with(player) {
            setDataSource(ref)
            prepare()
        }
    }

    private fun handle(connection: Socket) {
        try {
            val reader = InputStreamReader(connection.getInputStream())
            val buf = CharArray(1)
            var req = ""
            while (req.length < 4 || req.substring(req.length - 4) != "\r\n\r\n") {
                reader.read(buf, 0, 1)
                req += String(buf)
            }
            val url = req.split("\r\n")[0].split(" ")[1]
            when (url.split("?")[0]) {
                "/setip" -> Proxy().setIp(req, connection.getOutputStream(), baseContext)
                "/fetch" -> {
                    val resp =
                        "HTTP/1.1 404 shit happens\r\nCache-control: no-cache\r\nX-Content-Type-Options: nosniff\r\n\r\n"
                    val writer = OutputStreamWriter(connection.getOutputStream())
                    writer.write(resp, 0, resp.length)
                    writer.flush()
                    for (player in players) player.reset()
                    for (param in url.split("?")[1].split("&")) {
                        if (param.startsWith("album=")) {
                            album(param.split("=")[1])
                        }
                        if (param.startsWith("track=")) {
                            track = param.split("=")[1].toInt()
                        }
                    }
                    val ip = getSharedPreferences(PREFS_FILE, MODE_PRIVATE).getString(
                        PREF_IP,
                        "1.2.3.4."
                    )
                    val title = URL(
                        "http://$ip/meta?album=${album()}&meta=TITLE=&track=$track"
                    ).readText()
                    notificationManager.notify(
                        1,
                        Notification.Builder(this, CHANNEL_ID)
                            .setSmallIcon(R.drawable.ic_notification).setOnlyAlertOnce(true)
                            .setShowWhen(false).setContentText(title).build()
                    )
                    prepareMedia(players.first())
                    players.first().start()
                    track++
                    URL(
                        "http://$ip/meta?album=${album()}&meta=TITLE=&track=$track"
                    ).readText()
                    prepareMedia(players.last())
                    players.first().setNextMediaPlayer(players.last())
                }

                else -> Proxy().forwardIfConnected(req, connection.getOutputStream(), baseContext)
            }
        } catch (_: Exception) {
        } finally {
            connection.close()
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
        while (players.size < 2) players.add(MediaPlayer().apply {
            setAudioAttributes(
                AudioAttributes.Builder().setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                    .setUsage(AudioAttributes.USAGE_MEDIA).build()
            )
            setOnCompletionListener(this@MainService)
        })
        Thread {
            while (true) {
                val connection = sockServer.accept()
                Thread { handle(connection) }.start()
            }
        }.start()
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
        val context = this
        val ip = getSharedPreferences(PREFS_FILE, MODE_PRIVATE).getString(PREF_IP, "1.2.3.4")
        CoroutineScope(Job()).launch {
            try {
                val title = URL(
                    "http://$ip/meta?album=${album()}&meta=TITLE=&track=$track"
                ).readText()
                notificationManager.notify(
                    1,
                    Notification.Builder(context, CHANNEL_ID)
                        .setSmallIcon(R.drawable.ic_notification).setOnlyAlertOnce(true)
                        .setShowWhen(false).setContentText(title).build()
                )
            } catch (_: Exception) {
            }
            track++
            with(mp) {
                try {
                    this!!.reset()
                    URL(
                        "http://$ip/meta?album=${album()}&meta=TITLE=&track=$track"
                    ).readText()
                    setDataSource("http://$ip/fetch?album=${album()}&track=$track")
                    prepare()
                    for (player in players)
                        if (player !== this)
                            player.setNextMediaPlayer(this)
                } catch (_: Exception) {
                    for (player in players)
                        if (player !== this && !player.isPlaying)
                            notificationManager.notify(
                                1,
                                Notification.Builder(context, CHANNEL_ID)
                                    .setSmallIcon(R.drawable.ic_notification)
                                    .setOnlyAlertOnce(true).setShowWhen(false).setContentText("")
                                    .build()
                            )
                }
            }
        }
    }
}
