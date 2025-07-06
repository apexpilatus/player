package home.music.streamer

import android.app.Activity
import android.content.Intent
import android.os.Bundle
import home.music.streamer.MainService.Companion.started

class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (!started) startForegroundService(Intent(this, MainService::class.java))
        finishAndRemoveTask()
    }
}