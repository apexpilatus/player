package beans;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.ByteBuffer;

public class PlayerIpc {
    static Socket volSock = null;
    final String playerHost = "player";
    final int playerPort = 8888;
    final int timeOut = 5000;

    public void player0Play(String albumToPlay, String trackToPlay) {
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '0';
            sockWriter.write(op);
            sockWriter.flush();
            sockWriter.write(albumToPlay);
            sockWriter.flush();
            sockReader.readLine();
            sockWriter.write(trackToPlay);
            sockWriter.flush();
            sockReader.readLine();
        } catch (IOException ignored) {
        }
    }

    public String player1SetVolumeOpenVolSoc() {
        if (volSock != null && !volSock.isClosed()) {
            try {
                volSock.close();
            } catch (Exception ignored) {
            }
        }
        volSock = new Socket();
        try {
            volSock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
            volSock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(volSock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(volSock.getInputStream()));
            char op = '1';
            sockWriter.write(op);
            sockWriter.flush();
            return sockReader.readLine();
        } catch (IOException ignored1) {
            if (!volSock.isClosed()) {
                try {
                    volSock.close();
                } catch (IOException ignored2) {
                }
            }
            volSock = null;
            return "0;0";
        }
    }

    public void player1SetVolumeSetVol(long vol) {
        if (volSock != null && !volSock.isClosed()) {
            try {
                BufferedOutputStream sockWriter = new BufferedOutputStream(volSock.getOutputStream());
                sockWriter.write(ByteBuffer.allocate(8).putLong(vol).array());
                sockWriter.flush();
            } catch (IOException ignored) {
            }
        }
    }

    public void player1SetVolumeCloseVolSoc() {
        if (volSock != null && !volSock.isClosed()) {
            try {
                volSock.close();
            } catch (Exception ignored) {
            }
        }
        volSock = null;
    }
}
