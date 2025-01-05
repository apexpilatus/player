package home.music.streamer

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Intent
import android.graphics.BitmapFactory
import android.net.Uri
import android.os.IBinder
import java.io.BufferedReader
import java.io.InputStreamReader
import java.net.ServerSocket
import java.net.SocketException
import java.net.SocketTimeoutException
import java.net.URL

const val CHANNEL_ID = "main"
const val CHANNEL_NAME = "main"

object StoreStatus {
    private var connected = false
    private var started = false
    private var storeIP = ""

    @Synchronized
    fun setConnected(connected: Boolean) {
        this.connected = connected
    }

    @Synchronized
    fun getConnected(): Boolean {
        return this.connected
    }

    @Synchronized
    fun setStarted(started: Boolean) {
        this.started = started
    }

    @Synchronized
    fun getStarted(): Boolean {
        return this.started
    }

    @Synchronized
    fun setIP(ip: String) {
        this.storeIP = ip
    }

    @Synchronized
    fun getIP(): String {
        return storeIP
    }
}

class MainService : Service() {
    private val sockServer by lazy { ServerSocket(9696) }

    private val updateStatus = Runnable {
        while (true) {
            try {
                val sock = sockServer.accept()
                StoreStatus.setConnected(true)
                val msg = BufferedReader(InputStreamReader(sock.getInputStream())).readLine()
                sock.getOutputStream().write("OK\n".toByteArray())
                sock.close()
                if (!msg.equals(StoreStatus.getIP())) {
                    StoreStatus.setIP(msg)
                    val intent =
                        Intent(Intent.ACTION_VIEW, Uri.parse("http://${StoreStatus.getIP()}"))
                    val stream =
                        URL("http://${StoreStatus.getIP()}/apple-touch-icon.png").openStream()
                    (getSystemService(NOTIFICATION_SERVICE) as NotificationManager).notify(
                        1, Notification.Builder(this, CHANNEL_ID)
                            .setSmallIcon(R.drawable.ic_notification)
                            .setLargeIcon(BitmapFactory.decodeStream(stream))
                            .setContentIntent(
                                PendingIntent.getActivity(
                                    this,
                                    0,
                                    intent,
                                    PendingIntent.FLAG_IMMUTABLE
                                )
                            )
                            .setContentText(StoreStatus.getIP())
                            .build()
                    )
                    stream.close()
                }
            } catch (_: SocketException) {
                break
            } catch (_: SocketTimeoutException) {
            }
        }
    }

    private val checkStop = Runnable {
        while (true) {
            Thread.sleep(20000)
            if (StoreStatus.getConnected())
                StoreStatus.setConnected(false)
            else {
                stopSelf()
                break
            }
        }
    }

    override fun onCreate() {
        (getSystemService(NOTIFICATION_SERVICE) as NotificationManager).createNotificationChannel(
            NotificationChannel(
                CHANNEL_ID,
                CHANNEL_NAME,
                NotificationManager.IMPORTANCE_DEFAULT
            )
        )
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        StoreStatus.setStarted(true)
        StoreStatus.setIP("")
        this.startForeground(
            1, Notification.Builder(this, CHANNEL_ID)
                .setSmallIcon(R.drawable.ic_notification)
                .setContentText("not connected")
                .build()
        )
        Thread(updateStatus).start()
        Thread(checkStop).start()
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    override fun onDestroy() {
        sockServer.close()
        StoreStatus.setStarted(false)
    }
}