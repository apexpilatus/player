package beans;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.*;

public class MetaIpc {
    final String metaHost = "meta";
    final int metaPort = 9696;
    final int timeOut = 8000;

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

    public byte[] meta1GetPicture(String album) {
        byte[] pictureBytes = new byte[4];
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(metaHost, metaPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedInputStream sockReader = new BufferedInputStream(sock.getInputStream());
            char op = '1';
            sockWriter.write(op);
            sockWriter.write(album);
            sockWriter.flush();
            int readSize = sockReader.read(pictureBytes, 0, 4);
            if (readSize == 4) {
                sockWriter.write("ok");
                sockWriter.flush();
                int pictureSize = ByteBuffer.wrap(pictureBytes).order(ByteOrder.LITTLE_ENDIAN).getInt();
                pictureBytes = new byte[pictureSize];
                for (int off = 0; pictureSize > 0; pictureSize -= readSize, off += readSize) {
                    readSize = new BufferedInputStream(sock.getInputStream()).read(pictureBytes, off, pictureSize);
                }
                sockWriter.write("ok");
            } else {
                sockWriter.write("not");
            }
            sockWriter.flush();
        } catch (IOException ignored) {
        }
        return pictureBytes;
    }

    public Map<String, Map<String, String>> meta2GetTags(String album) {
        Map<String, Map<String, String>> metasMap = new TreeMap<>();
        try (Socket sock = new Socket()) {
            sock.connect(new InetSocketAddress(metaHost, metaPort), timeOut);
            sock.setSoTimeout(timeOut);
            BufferedWriter sockWriter = new BufferedWriter(new OutputStreamWriter(sock.getOutputStream()));
            BufferedReader sockReader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
            char op = '2';
            sockWriter.write(op);
            sockWriter.flush();
            sockWriter.write(album);
            sockWriter.flush();
            for (String file = sockReader.readLine(); !file.equals("&the_end"); file = sockReader.readLine()) {
                metasMap.put(file, new HashMap<>());
                metasMap.get(file).put("ARTIST", "");
                metasMap.get(file).put("ALBUM", "");
                metasMap.get(file).put("TRACKNUMBER", "");
                metasMap.get(file).put("TITLE", "");
                metasMap.get(file).put("RATE", "");
                for (String comment = sockReader.readLine(); !comment.equals("&end_tags"); comment = sockReader.readLine()) {
                    metasMap.get(file).put(comment.split("=")[0], comment.substring(comment.split("=")[0].length() + 1));
                }
            }
        } catch (IOException ignored) {
        }
        return metasMap;
    }
}
