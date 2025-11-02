package home.music.streamer

import android.app.Activity
import android.app.NotificationManager
import android.content.Intent
import android.content.pm.PackageManager
import android.media.AudioManager
import android.os.Bundle
import android.provider.Settings
import home.music.streamer.MainService.Companion.started

class MainActivity : Activity() {
    private val notificationManager by lazy { getSystemService(NOTIFICATION_SERVICE) as NotificationManager }
    private val audioManager by lazy { getSystemService(AUDIO_SERVICE) as AudioManager }
    private fun exit(ok: Boolean) {
        if (ok) {
            if (!started) startForegroundService(Intent(this, MainService::class.java))
            audioManager.ringerMode = AudioManager.RINGER_MODE_SILENT
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
        )
        else {
            if (!notificationManager.isNotificationPolicyAccessGranted) {
                val intent = Intent(Settings.ACTION_NOTIFICATION_POLICY_ACCESS_SETTINGS)
                startActivityForResult(intent, 1)
            } else exit(true)
        }
    }

    override fun onActivityResult(
        requestCode: Int, resultCode: Int, data: Intent?
    ) {
        super.onActivityResult(requestCode, resultCode, data)
        exit(notificationManager.isNotificationPolicyAccessGranted)
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
        if (ok) {
            if (!notificationManager.isNotificationPolicyAccessGranted) {
                val intent = Intent(Settings.ACTION_NOTIFICATION_POLICY_ACCESS_SETTINGS)
                startActivityForResult(intent, 1)
            } else exit(true)
        } else exit(false)
    }
}