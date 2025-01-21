package home.music.streamer

import android.app.Activity
import android.content.Intent
import android.os.Bundle

class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (!MainService.started) startForegroundService(Intent(this, MainService::class.java))
        finishAndRemoveTask()
    }
}