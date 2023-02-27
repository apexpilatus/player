package beans;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;

public class Storage {
    String[] musicDirPaths = {"/home/store/music/qbzcd", "/home/store/music/dzr", "/home/store/music/hack/1", "/home/store/music/hack/2", "/home/store/music/hack/3", "/home/store/music/hack/4"};

    public Map<String, List<String>> getAlbums() {
        Map<String, List<String>> albums = new TreeMap<>();
        for (String musicDirPath : musicDirPaths) {
            File musicDir = new File(musicDirPath);
            if (musicDir.exists()) {
                for (String album : Objects.requireNonNull(musicDir.list())) {
                    albums.computeIfAbsent(album, (k) -> new ArrayList<>()).add(musicDirPath);
                }
            }
        }

        try {
            BufferedWriter htmlFileWriter = new BufferedWriter(new FileWriter("fuck"));
            albums.forEach((k, v) -> {
                    v.forEach((s) -> {
                        try {
                            htmlFileWriter.write(s);
                        } catch (IOException e) {
                            throw new RuntimeException(e);
                        }
                    });
            });
            htmlFileWriter.flush();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }

        return albums;
    }
}