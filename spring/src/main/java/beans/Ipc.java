package beans;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;

public class Ipc {
    final String playerHost = "player";
    final int playerPort = 8888;
    final int timeOut = 5000;

    public int action2GetVol() {
        int ret = -1;
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            byte op = 2;
            writer.write(op);
            writer.flush();
            ret = reader.read();
        } catch (IOException ignored) {
        }
        return ret;
    }
}
