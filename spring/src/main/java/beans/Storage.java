package beans;

import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;
import org.jflac.metadata.Picture;

import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Field;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;
import java.util.stream.Stream;

public class Storage {
    String musicDir = "/home/store/music";

    public Map<String, List<String>> getAlbums() throws IOException {
        Map<String, List<String>> albums = new TreeMap<>();
        try (Stream<Path> walk = Files.walk(Paths.get(musicDir))) {
            walk.filter(Files::isDirectory).filter((path) -> {
                long count;
                try (Stream<Path> files = Files.list(path)) {
                    count = files.filter(Files::isRegularFile).count();
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
                return count > 0;
            }).forEach((path) -> albums.computeIfAbsent(path.getFileName().toString(), (k) -> new ArrayList<>()).add(path.getParent().toString()));
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