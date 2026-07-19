package home.music.streamer

import android.content.Context
import java.io.OutputStream
import java.io.OutputStreamWriter
import java.net.InetAddress
import java.net.InetSocketAddress
import java.net.Socket

const val PREFS_FILE = "prefs"
const val PREF_IP = "ip"

class Proxy {
    private fun sendConfigPage(writer: OutputStream, context: Context) {
        context.assets.open("config.html").use {
            val buf = ByteArray(4096)
            val size: Int = it.read(buf)
            if (size < 0)
                return
            val hdr =
                "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: $size\r\n\r\n"
            writer.write(hdr.toByteArray(), 0, hdr.length)
            writer.write(buf, 0, size)
            writer.flush()
        }
    }

    private fun forward(req: String, socket: Socket, writer: OutputStream) {
        val store = OutputStreamWriter(socket.getOutputStream())
        store.write(req)
        store.flush()
        val buf = ByteArray(2048)
        val reader = socket.getInputStream()
        var size: Int
        while (true) {
            size = reader.read(buf)
            if (size < 0)
                break
            writer.write(buf, 0, size)
            writer.flush()
        }
    }

    fun setIp(req: String, writer: OutputStream, context: Context) {
        val prefs = context.getSharedPreferences(PREFS_FILE, Context.MODE_PRIVATE).edit()
        prefs.putString(PREF_IP, req.split("\r\n")[0].split(" ")[1].split("?")[1].split("=")[1])
        if (prefs.commit()) {
            forwardIfConnected("GET / HTTP/1.1\r\n\r\n", writer, context)
            return
        }
        val resp =
            "HTTP/1.1 404 shit happens\r\nCache-control: no-cache\r\nX-Content-Type-Options: nosniff\r\n\r\n"
        writer.write(resp.toByteArray(), 0, resp.length)
        writer.flush()
    }

    fun forwardIfConnected(req: String, writer: OutputStream, context: Context) {
        Socket().use {
            val prefs = context.getSharedPreferences(PREFS_FILE, Context.MODE_PRIVATE)
            try {
                it.connect(
                    InetSocketAddress(
                        InetAddress.getByName(prefs.getString(PREF_IP, "1.2.3.4.")),
                        80
                    ), 7000
                )
            } catch (_: Exception) {
                if (req.split("\r\n")[0].split(" ")[1] == "/")
                    sendConfigPage(writer, context)
                return
            }
            forward(req, it, writer)
        }
    }
}