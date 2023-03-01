package beans;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;

public class Ipc {
    final String playerHost = "player";
    final int playerPort = 8888;
    final int timeOut = 5000;

    public void action0Play(String albumToPlay, String trackToPlay) {
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '0';
            sockWriter.write(op);
            sockWriter.flush();
            sockReader.readLine();
            sockWriter.write(albumToPlay);
            sockWriter.flush();
            sockReader.readLine();
            sockWriter.write(trackToPlay);
            sockWriter.flush();
            sockReader.readLine();
        } catch (Exception ignored) {
        }
    }

    public void action1SetVol(int vol) {
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '1';
            sockWriter.write(op);
            sockWriter.flush();
            sockReader.readLine();
            sockWriter.write(vol);
            sockWriter.flush();
            sockReader.readLine();
        } catch (IOException ignored) {
        }
    }

    public int action2GetVol() {
        int ret = -1;
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '2';
            writer.write(op);
            writer.flush();
            ret = reader.read();
        } catch (IOException ignored) {
        }
        return ret;
    }
}
