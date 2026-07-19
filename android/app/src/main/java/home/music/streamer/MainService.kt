package home.music.streamer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Intent
import android.os.IBinder
import java.io.InputStreamReader
import java.net.ServerSocket
import java.net.Socket

const val CHANNEL_ID = "main"
const val CHANNEL_NAME = "main"

class MainService : Service() {
    private val sockServer by lazy { ServerSocket(8888) }
    private val notificationManager by lazy { getSystemService(NOTIFICATION_SERVICE) as NotificationManager }

    companion object {
        @Volatile
        var started = false
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
            when (req.split("\r\n")[0].split(" ")[1].split("?")[0]) {
                "/setip" -> Proxy().setIp(req, connection.getOutputStream(), baseContext)
                else -> Proxy().forwardIfConnected(req, connection.getOutputStream(), baseContext)
            }
        } catch (_: Exception) {
            notificationManager.notify(
                1,
                Notification.Builder(this, CHANNEL_ID)
                    .setSmallIcon(R.drawable.ic_notification)
                    .setOnlyAlertOnce(true).setShowWhen(false).setContentText("fuck")
                    .build()
            )
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
}
