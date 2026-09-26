package home.music.streamer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.media.AudioManager
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
    private val proxy = Proxy(this)
    private val mixer by lazy { Mixer(this) }


    companion object {
        @Volatile
        var started = false
        @Volatile
        private var track = 0
        private var album = ""
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

    private fun prepareMedia(player: MediaPlayer, files: List<String>) {
        val ref = "http://${
            getSharedPreferences(PREFS_FILE, MODE_PRIVATE).getString(
                PREF_IP,
                "1.2.3.4"
            )
        }/fetch?album=${album()}&file=${files[track]}"
        with(player) {
            setDataSource(ref)
            prepare()
            mixer.prefDev(this)
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
                "/setip" -> proxy.setIp(req, connection.getOutputStream())
                "/getcards" -> mixer.getCards(connection.getOutputStream())
                "/getvolume" -> mixer.getVolume(connection.getOutputStream())
                "/setvolume" -> mixer.setVolume(url, connection.getOutputStream())
                "/stream" -> {
                    val resp =
                        "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nCache-control: no-cache\r\nX-Content-Type-Options: nosniff\r\n\r\n"
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
                    val files = URL(
                        "http://$ip/files?album=${album()}"
                    ).readText().split("\r\n")
                    val title = URL(
                        "http://$ip/meta?album=${album()}&tag=TITLE=&file=${files[track]}"
                    ).readText()
                    notificationManager.notify(
                        1,
                        Notification.Builder(this, CHANNEL_ID)
                            .setSmallIcon(R.drawable.ic_notification).setOnlyAlertOnce(true)
                            .setShowWhen(false).setContentText(title).build()
                    )
                    mixer.audioManager.registerAudioDeviceCallback(mixer, null)
                    prepareMedia(players.first(), files)
                    mixer.audioManager.mode = AudioManager.MODE_NORMAL
                    players.first().start()
                    if (++track < files.size) {
                        prepareMedia(players.last(), files)
                        players.first().setNextMediaPlayer(players.last())
                    }
                    mixer.audioManager.unregisterAudioDeviceCallback(mixer)
                }

                else -> proxy.forwardIfConnected(req, connection.getOutputStream())
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
                mixer.audioAttr
            )
            setOnCompletionListener(this@MainService)
        })
        CoroutineScope(Job()).launch {
            while (true) {
                val connection = sockServer.accept()
                CoroutineScope(Job()).launch { handle(connection) }
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

    override fun onCompletion(mp: MediaPlayer?) {
        val context = this
        val ip = getSharedPreferences(PREFS_FILE, MODE_PRIVATE).getString(PREF_IP, "1.2.3.4")
        CoroutineScope(Job()).launch {
            try {
                val files = URL(
                    "http://$ip/files?album=${album()}"
                ).readText().split("\r\n")
                val title = if (track < files.size)
                    URL(
                        "http://$ip/meta?album=${album()}&tag=TITLE=&file=${files[track]}"
                    ).readText()
                else
                    ""
                notificationManager.notify(
                    1,
                    Notification.Builder(context, CHANNEL_ID)
                        .setSmallIcon(R.drawable.ic_notification).setOnlyAlertOnce(true)
                        .setShowWhen(false).setContentText(title).build()
                )
                if (track < files.size && ++track < files.size) {
                    with(mp) {
                        this!!.reset()
                        setDataSource("http://$ip/fetch?album=${album()}&file=${files[track]}")
                        prepare()
                        mixer.prefDev(this)
                        for (player in players)
                            if (player !== this)
                                player.setNextMediaPlayer(this)
                    }
                }
            } catch (_: Exception) {
            }
        }
    }
}
