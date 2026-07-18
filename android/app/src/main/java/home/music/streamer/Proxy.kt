package home.music.streamer

import java.io.OutputStream
import java.io.OutputStreamWriter
import java.net.InetAddress
import java.net.InetSocketAddress
import java.net.Socket

class Proxy {
    fun forwardIfNoStatic(req: String, writer: OutputStream) {
        Socket().use {
            it.connect(InetSocketAddress(InetAddress.getByName("10.0.2.2"), 9696), 4000)
            val store = OutputStreamWriter(it.getOutputStream())
            store.write(req)
            store.flush()
            val reader = it.getInputStream()
            val buf = ByteArray(2048)
            var size: Int
            while (true) {
                size = reader.read(buf)
                if (size < 0)
                    break
                writer.write(buf, 0, size)
                writer.flush()
            }
        }
    }
}