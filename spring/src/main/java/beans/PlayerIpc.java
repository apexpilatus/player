package beans;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;

public class PlayerIpc {
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

    public void player1SetVol(int vol) {
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '1';
            sockWriter.write(op);
            sockWriter.flush();
            sockReader.readLine();
            sockWriter.write(String.valueOf(vol));
            sockWriter.flush();
            sockReader.readLine();
        } catch (IOException ignored) {
        }
    }

    public String player2GetVol() {
        String volData = "0;0";
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '2';
            writer.write(op);
            writer.flush();
            volData = reader.readLine();
        } catch (IOException ignored) {
        }
        return volData;
    }
}
