package beans;

import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;
import org.jflac.metadata.Picture;
import org.jflac.metadata.StreamInfo;
import org.jflac.metadata.VorbisComment;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.lang.reflect.Field;
import java.text.DecimalFormat;
import java.util.*;

public class Storage {
    String[] musicDirPaths = { "/home/store/music/qbzcd", "/home/store/music/dzr", "/home/store/music/hack/1",
            "/home/store/music/hack/2", "/home/store/music/hack/3", "/home/store/music/hack/4" };

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
                    VorbisComment vorbis = (VorbisComment) meta;
                    metasMap.put("ARTIST",
                            vorbis.getCommentByName("ARTIST").length == 0 ? "" : vorbis.getCommentByName("ARTIST")[0]);
                    metasMap.put("ALBUM",
                            vorbis.getCommentByName("ALBUM").length == 0 ? "" : vorbis.getCommentByName("ALBUM")[0]);
                    metasMap.put("TRACKNUMBER", vorbis.getCommentByName("TRACKNUMBER").length == 0 ? "-"
                            : vorbis.getCommentByName("TRACKNUMBER")[0] + ") ");
                    metasMap.put("TITLE",
                            vorbis.getCommentByName("TITLE").length == 0 ? "" : vorbis.getCommentByName("TITLE")[0]);
                }
                if (meta.toString().contains("StreamInfo")) {
                    StreamInfo info = (StreamInfo) meta;
                    metasMap.put("RATE",
                            String.valueOf(info.getBitsPerSample()) + "/"
                                    + String.valueOf(new DecimalFormat("#.#").format(info.getSampleRate() / 1000f)));
                }
            }
        } catch (Exception e) {
            throw new IOException(e);
        }
        return metasMap;
    }

    public byte[] getPictureBytes(String album) throws IOException, NoSuchFieldException, IllegalAccessException {
        byte[] pictureBytes = { 1, 2, 3 };
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
