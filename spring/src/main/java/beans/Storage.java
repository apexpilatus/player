package beans;

import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;

import java.io.File;
import java.io.FileInputStream;
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
        return albums;
    }

    public Map<String, String> getMetas(String file) throws IOException {
        Map<String, String> metasMap = new HashMap<>();
        try (FileInputStream flacIs = new FileInputStream(file)) {
            FLACDecoder flacDec = new FLACDecoder(flacIs);
            Metadata[] metas = flacDec.readMetadata();
            for (Metadata meta : metas) {
                if (meta.toString().contains("VorbisComment")) {
                    meta.toString().lines().forEach((line) -> {
                        if (line.contains("TRACKNUMBER")) {
                            metasMap.put("TRACKNUMBER", line.split("=")[1] + ". ");
                        }
                        if (line.contains("TITLE")) {
                            metasMap.put("TITLE", line.substring(7));
                        }
                        if (file.contains("01.flac") && line.contains("ALBUM")) {
                            metasMap.put("ALBUM", line.substring(7));
                        }
                        if (file.contains("01.flac") && line.contains("ARTIST")) {
                            metasMap.put("ARTIST", line.substring(8));
                        }
                    });
                }
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
        return metasMap;
    }
}