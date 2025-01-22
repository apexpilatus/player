package home.music.streamer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Intent
import android.graphics.BitmapFactory
import android.media.AudioAttributes
import android.media.MediaPlayer
import android.net.Uri
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

    companion object {
        @Volatile
        private var connected = false

        @Volatile
        var started = false

        val refs = ConcurrentLinkedQueue<String>()
        val players = ConcurrentLinkedQueue<MediaPlayer>()
    }

    private val updateStatus = Runnable {
        var storeIP = ""
        var notifyWhenStopped = false
        while (true) {
            try {
                val sock = sockServer.accept()
                connected = true
                val msg = BufferedReader(InputStreamReader(sock.getInputStream())).readLine()
                sock.close()
                if (msg[0] == '/') {
                    if (storeIP != "") {
                        refs.clear()
                        for (player in players) {
                            player.reset()
                        }
                        notifyWhenStopped = true
                        for (ref in msg.split("|")) {
                            refs.add("http://$storeIP$ref")
                        }
                        val fileName =
                            "/" + (refs.peek() as String).split("/")[(refs.peek() as String).split("/").size - 1]
                        val stream = URL((refs.peek() as String).replace(fileName, "")).openStream()
                        val intent = Intent(Intent.ACTION_VIEW, Uri.parse("http://$storeIP"))
                        (getSystemService(NOTIFICATION_SERVICE) as NotificationManager).notify(
                            1,
                            Notification.Builder(this, CHANNEL_ID)
                                .setSmallIcon(R.drawable.ic_notification)
                                .setLargeIcon(BitmapFactory.decodeStream(stream)).setContentIntent(
                                    PendingIntent.getActivity(
                                        this, 0, intent, PendingIntent.FLAG_IMMUTABLE
                                    )
                                ).setOnlyAlertOnce(true).setShowWhen(false).setContentText("").build()
                        )
                        stream.close()
                        with(players.peek() as MediaPlayer) {
                            try {
                                setDataSource(refs.poll())
                                prepare()
                                start()
                            } catch (_: Exception) {
                            }
                            val ref = refs.poll()
                            if (ref != null) {
                                try {
                                    with(players.last()) {
                                        setDataSource(ref)
                                        prepare()
                                    }
                                    setNextMediaPlayer(players.last())
                                } catch (_: Exception) {
                                }
                            }
                        }
                    }
                } else {
                    if (storeIP != msg || (!(players.first().isPlaying || players.last().isPlaying) && notifyWhenStopped)) {
                        notifyWhenStopped = false
                        storeIP = msg
                        val intent = Intent(Intent.ACTION_VIEW, Uri.parse("http://$storeIP"))
                        val stream = URL("http://$storeIP/apple-touch-icon.png").openStream()
                        (getSystemService(NOTIFICATION_SERVICE) as NotificationManager).notify(
                            1,
                            Notification.Builder(this, CHANNEL_ID)
                                .setSmallIcon(R.drawable.ic_notification)
                                .setLargeIcon(BitmapFactory.decodeStream(stream)).setContentIntent(
                                    PendingIntent.getActivity(
                                        this, 0, intent, PendingIntent.FLAG_IMMUTABLE
                                    )
                                ).setOnlyAlertOnce(true).setShowWhen(false).setContentText("").build()
                        )
                        stream.close()
                    }
                }
            } catch (_: Exception) {
                refs.clear()
                for (player in players) {
                    player.reset()
                    player.release()
                }
                players.clear()
                break
            }
        }
    }

    private val checkStop = Runnable {
        while (true) {
            Thread.sleep(15000)
            if (connected) connected = false
            else {
                stopSelf()
                break
            }
        }
    }

    override fun onCreate() {
        (getSystemService(NOTIFICATION_SERVICE) as NotificationManager).createNotificationChannel(
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
                .setShowWhen(false).setContentText("not connected").build()
        )
        for (i in 1..2) {
            players.add(MediaPlayer().apply {
                setAudioAttributes(
                    AudioAttributes.Builder().setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                        .setUsage(AudioAttributes.USAGE_MEDIA).build()
                )
                setOnCompletionListener(this@MainService)
            })
        }
        Thread(updateStatus).start()
        Thread(checkStop).start()
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
        val ref = refs.poll()
        if (ref != null) {
            with(players.poll() as MediaPlayer) {
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