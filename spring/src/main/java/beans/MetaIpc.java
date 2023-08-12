package beans;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

public class MetaIpc {
    final String metaHost = "meta";
    final int metaPort = 9696;
    final int timeOut = 5000;

    public Map<String, List<String>> meta0GetAlbums() {
        Map<String, List<String>> albums = new TreeMap<>();
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(metaHost, metaPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '0';
            writer.write(op);
            writer.flush();
            for (String path = reader.readLine(); !path.equals("&the_end"); path = reader.readLine()) {
                for (String album = reader.readLine(); !album.equals("&end_folder"); album = reader.readLine()) {
                    albums.computeIfAbsent(album, (k) -> new ArrayList<>()).add(path);
                }
            }
        } catch (IOException ignored) {
        }
        return albums;
    }
}
