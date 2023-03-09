package beans;

import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;
import org.jflac.metadata.Picture;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Field;
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
        metasMap.put("TRACKNUMBER", "");
        metasMap.put("TITLE", "");
        metasMap.put("ALBUM", "");
        metasMap.put("ARTIST", "");
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
            throw new IOException(e);
        }
        return metasMap;
    }

    public byte[] getPictureBytes(String album) throws IOException, NoSuchFieldException, IllegalAccessException {
        byte[] pictureBytes = {1, 2, 3};
        try (FileInputStream flacIs = new FileInputStream(album + "/01.flac")) {
            FLACDecoder flacDec = new FLACDecoder(flacIs);
            Metadata[] metas = flacDec.readMetadata();
            for (Metadata meta : metas) {
                if (meta.toString().contains("Picture")) {
                    Picture picMeta = (Picture) meta;
                    Class<? extends Picture> c = picMeta.getClass();
                    Field f = c.getDeclaredField("image");
                    f.setAccessible(true);
                    pictureBytes = (byte[]) f.get(picMeta);
                }
            }
        } catch (IOException e) {
            throw new IOException(e);
        } catch (NoSuchFieldException e) {
            throw new NoSuchFieldException();
        } catch (IllegalAccessException e) {
            throw new IllegalAccessException();
        }
        return pictureBytes;
    }
}