import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;

public class Common {
    static final String exeDirPath = "/home/exe";
    static final int playerPort = 8888;
    static final int timeOut = 5000;
    static String[] musicDirPaths = {"/home/store/music/qbz", "/home/store/music/dzr", "/home/store/music/hack/1", "/home/store/music/hack/2", "/home/store/music/hack/3", "/home/store/music/hack/4"};
    static String[] playerHosts = {"default", "backup"};
    static private String currentPlayer = "none";

    static synchronized void action0Play(String albumToPlay, String trackToPlay) {
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(currentPlayer, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            byte op = 0;
            sockWriter.write(op);
            sockWriter.flush();
            sockReader.readLine();
            sockWriter.write(albumToPlay);
            sockWriter.flush();
            sockReader.readLine();
            sockWriter.write(trackToPlay == null ? "01.flac" : trackToPlay);
            sockWriter.flush();
            sockReader.readLine();
        } catch (Exception ignored) {
            currentPlayer = "none";
        }
    }

    static synchronized void action1SetVol(int vol) {
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(currentPlayer, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            byte op = 1;
            sockWriter.write(op);
            sockWriter.flush();
            sockReader.readLine();
            sockWriter.write(vol);
            sockWriter.flush();
            sockReader.readLine();
        } catch (IOException ignored) {
            currentPlayer = "none";
        }
    }

    static synchronized int action2GetVol() {
        int ret = -1;
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(currentPlayer, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            byte op = 2;
            writer.write(op);
            writer.flush();
            ret = reader.read();
        } catch (IOException ignored) {
            currentPlayer = "none";
        }
        return ret;
    }

    static synchronized int action2SwitchDevice() {
        int ret = -1;
        for (String playerHost : playerHosts) {
            try (Socket sock = new Socket()) {
                sock.connect(new InetSocketAddress(playerHost, playerPort), timeOut);
                sock.setSoTimeout(timeOut);
                BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
                BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
                byte op = 2;
                writer.write(op);
                writer.flush();
                ret = reader.read();
                currentPlayer = playerHost;
                break;
            } catch (IOException ignored) {
	    	if (playerHost.equals(playerHosts[1])) {
                	currentPlayer = "none";
		}
            }
        }
        return ret;
    }

    static synchronized void action3Stop() {
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(currentPlayer, playerPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            byte op = 3;
            sockWriter.write(op);
            sockWriter.flush();
            sockReader.readLine();
        } catch (Exception ignored) {
            currentPlayer = "none";
        }
    }

    static synchronized String getCurrentPlayer() {
        return currentPlayer;
    }
}
