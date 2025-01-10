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
import java.net.SocketException
import java.net.SocketTimeoutException
import java.net.URL

const val CHANNEL_ID = "main"
const val CHANNEL_NAME = "main"

class MainService : Service() {
    private val sockServer by lazy { ServerSocket(9696) }

    companion object {
        @Volatile
        private var connected = false

        @Volatile
        var started = false
    }

    private val updateStatus = Runnable {
        var storeIP = ""
        val playList = ArrayList<MediaPlayer>()
        while (true) {
            try {
                val sock = sockServer.accept()
                connected = true
                val msg = BufferedReader(InputStreamReader(sock.getInputStream())).readLine()
                sock.close()
                if (msg[0] == '/') {
                    for (player in playList){
                        player.reset()
                        player.release()
                    }
                    playList.clear()
                    for (ref in msg.split("|")) {
                        playList.add(MediaPlayer().apply {
                            setAudioAttributes(
                                AudioAttributes.Builder()
                                    .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                                    .setUsage(AudioAttributes.USAGE_MEDIA)
                                    .build()
                            )
                            setDataSource("http://$storeIP$ref")
                            prepare()
                        })
                    }
                    val playIterator = playList.iterator()
                    if (playIterator.hasNext()){
                        var playCurrent = playIterator.next()
                        while (playIterator.hasNext()){
                            val playNext = playIterator.next()
                            playCurrent.setNextMediaPlayer(playNext)
                            playCurrent = playNext
                        }
                        playList[0].start()
                    }
                } else {
                    if (!msg.equals(storeIP)) {
                        storeIP = msg
                        val intent = Intent(Intent.ACTION_VIEW, Uri.parse("http://$storeIP"))
                        val stream = URL("http://$storeIP/apple-touch-icon.png").openStream()
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
                                .setContentText("")
                                .build()
                        )
                        stream.close()
                    }
                }
            } catch (_: SocketException) {
                for (player in playList){
                    player.reset()
                    player.release()
                }
                playList.clear()
                break
            } catch (_: SocketTimeoutException) {
            }
        }
    }

    private val checkStop = Runnable {
        while (true) {
            Thread.sleep(20000)
            if (connected)
                connected = false
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
        started = true
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
        started = false
    }
}