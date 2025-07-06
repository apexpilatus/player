package home.music.streamer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.graphics.BitmapFactory
import android.os.IBinder
import java.io.BufferedReader
import java.io.BufferedWriter
import java.io.InputStream
import java.io.InputStreamReader
import java.io.OutputStreamWriter
import java.net.ServerSocket
import java.net.Socket
import java.net.URL
import java.util.Arrays

const val CHANNEL_ID = "main"
const val CHANNEL_NAME = "main"

class MainService : Service() {
    private val sockServer by lazy { ServerSocket(9696) }
    private val notificationManager by lazy { getSystemService(NOTIFICATION_SERVICE) as NotificationManager }

    companion object {
        @Volatile
        var started = false

        @Volatile
        var stopPlaying = false
    }

    private fun checkHeader(reader: InputStream): Boolean {
        val hdr: MutableList<Byte> = mutableListOf()
        hdr.addAll(reader.readNBytes(1).asList())
        if (hdr.isNotEmpty()) {
            var hdrSize = hdr.size
            val maxSize = 1000
            while (hdrSize < maxSize) {
                hdr.addAll(reader.readNBytes(1).asList())
                if (hdr.size == hdrSize) {
                    return false
                }
                if (hdr.size > 3 && Arrays.compare(
                        hdr.toByteArray(),
                        hdr.size - 4,
                        hdr.size - 1,
                        "\r\n\r\n".toByteArray(),
                        0,
                        3
                    ) == 0
                ) break
                hdrSize = hdr.size
            }
            if (hdrSize == maxSize) return false
            if (String(hdr.toByteArray()).contains("HTTP/1.1 200 OK")) return true
        }
        return false
    }

    private val listen = Thread {
        var remoteIP: String
        var url: String
        var play = Thread {
            notificationManager.notify(
                1,
                Notification.Builder(this, CHANNEL_ID).setSmallIcon(R.drawable.ic_notification)
                    .setShowWhen(false).setOnlyAlertOnce(true).setContentText("waiting").build()
            )
        }
        play.start()
        while (true) {
            try {
                sockServer.accept().use {
                    stopPlaying = true
                    play.join()
                    stopPlaying = false
                    remoteIP = it.remoteSocketAddress.toString().replace("/", "").split(":")[0]
                    url = BufferedReader(InputStreamReader(it.getInputStream())).readLine()
                    play = Thread {
                        try {
                            Socket(remoteIP, 80).use { socket ->
                                val writer =
                                    BufferedWriter(OutputStreamWriter(socket.getOutputStream()))
                                writer.write("GET $url \r\n\r\n")
                                writer.flush()
                                val reader = socket.getInputStream()
                                if (checkHeader(reader)) {
                                    URL(
                                        "http://$remoteIP/apple-touch-icon.png"
                                    ).openStream().use { streamPicture ->
                                        notificationManager.notify(
                                            1,
                                            Notification.Builder(this, CHANNEL_ID)
                                                .setSmallIcon(R.drawable.ic_notification)
                                                .setLargeIcon(BitmapFactory.decodeStream(streamPicture))
                                                .setOnlyAlertOnce(true).setShowWhen(false)
                                                .setContentText("").build()
                                        )
                                    }
                                    Streamer(reader).play()
                                    notificationManager.notify(
                                        1,
                                        Notification.Builder(this, CHANNEL_ID)
                                            .setSmallIcon(R.drawable.ic_notification)
                                            .setOnlyAlertOnce(true).setShowWhen(false)
                                            .setContentText("").build()
                                    )
                                }
                            }
                        } catch (_: Exception) {
                        }
                    }
                    play.start()
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
}