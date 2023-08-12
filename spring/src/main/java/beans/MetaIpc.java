package beans;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.*;

public class MetaIpc {
    final String metaHost = "meta";
    final int metaPort = 9696;
    final int timeOut = 5000;

    public Map<String, List<String>> meta0GetAlbums() {
        Map<String, List<String>> albums = new TreeMap<>();
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(metaHost, metaPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '0';
            sockWriter.write(op);
            sockWriter.flush();
            for (String path = sockReader.readLine(); !path.equals("&the_end"); path = sockReader.readLine()) {
                for (String album = sockReader.readLine(); !album.equals("&end_folder"); album = sockReader.readLine()) {
                    albums.computeIfAbsent(album, (k) -> new ArrayList<>()).add(path);
                }
            }
        } catch (IOException ignored) {
        }
        return albums;
    }

    public byte[] meta1GetPicture(String file) {
        byte[] pictureBytes = {1, 2, 3};
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(metaHost, metaPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '1';
            sockWriter.write(op);
            sockWriter.flush();
            sockWriter.write(file);
            sockWriter.flush();
            int pictureSize = Integer.parseInt(sockReader.readLine());
            pictureBytes = new byte[pictureSize];
            sockWriter.write("ok");
            sockWriter.flush();
            for (int readSize, off = 0; pictureSize > 0; pictureSize -= readSize, off += readSize) {
                readSize = sock.getInputStream().read(pictureBytes, off, pictureSize);
            }
            sockWriter.write("ok");
            sockWriter.flush();
        } catch (IOException ignored) {
        }
        return pictureBytes;
    }

    public Map<String, String> meta2GetTags(String file) {
        Map<String, String> metasMap = new HashMap<>();
        metasMap.put("ARTIST", "");
        metasMap.put("ALBUM", "");
        metasMap.put("TRACKNUMBER", "");
        metasMap.put("TITLE", "");
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(metaHost, metaPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '2';
            sockWriter.write(op);
            sockWriter.flush();
            sockWriter.write(file);
            sockWriter.flush();
            for (String comment = sockReader.readLine(); !comment.equals("&end_tags"); comment = sockReader.readLine()) {
                metasMap.put(comment.split("=")[0], comment.substring(comment.split("=")[0].length() + 1));
            }
            metasMap.put("RATE", sockReader.readLine());
        } catch (IOException ignored) {
        }
        return metasMap;
    }
}
