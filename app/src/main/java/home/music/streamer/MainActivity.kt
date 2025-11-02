package home.music.streamer

import android.app.Activity
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import home.music.streamer.MainService.Companion.started

class MainActivity : Activity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (checkSelfPermission(android.Manifest.permission.POST_NOTIFICATIONS) == PackageManager.PERMISSION_GRANTED) {
            if (!started) startForegroundService(Intent(this, MainService::class.java))
            finishAndRemoveTask()
        } else {
            requestPermissions(arrayOf(android.Manifest.permission.POST_NOTIFICATIONS), 1)
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<out String?>, grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (checkSelfPermission(permissions[0]!!) == PackageManager.PERMISSION_GRANTED && !started) startForegroundService(
            Intent(this, MainService::class.java)
        )
        finishAndRemoveTask()
    }
}