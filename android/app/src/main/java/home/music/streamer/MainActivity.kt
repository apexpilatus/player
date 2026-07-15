package home.music.streamer

import android.app.Activity
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import home.music.streamer.MainService.Companion.started

class MainActivity : Activity() {
    private fun exit(ok: Boolean) {
        if (ok && !started) {
            startForegroundService(Intent(this, MainService::class.java))
        }
        finishAndRemoveTask()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val permissions = arrayOf(
            android.Manifest.permission.POST_NOTIFICATIONS
        )
        var needPermission = false
        for (permission in permissions) if (checkSelfPermission(permission) == PackageManager.PERMISSION_DENIED) {
            needPermission = true
            break
        }
        if (needPermission) requestPermissions(
            permissions, 1
        ) else exit(true)
    }

    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<out String?>, grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        var ok = true
        for (permission in permissions) if (checkSelfPermission(permission!!) == PackageManager.PERMISSION_DENIED) {
            ok = false
            break
        }
        exit(ok)
    }
}