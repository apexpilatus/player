package beans;

import org.jflac.FLACDecoder;
import org.jflac.metadata.Metadata;
import org.jflac.metadata.StreamInfo;
import org.jflac.metadata.VorbisComment;

import java.io.FileInputStream;
import java.io.IOException;
import java.text.DecimalFormat;
import java.util.HashMap;
import java.util.Map;

public class Storage {

    public Map<String, String> getMetas(String file) throws IOException {
        Map<String, String> metasMap = new HashMap<>();
        try (FileInputStream flacIs = new FileInputStream(file)) {
            FLACDecoder flacDec = new FLACDecoder(flacIs);
            Metadata[] metas = flacDec.readMetadata();
            for (Metadata meta : metas) {
                if (meta.toString().contains("VorbisComment")) {
                    VorbisComment vorbis = (VorbisComment) meta;
                    metasMap.put("ARTIST", vorbis.getCommentByName("ARTIST").length == 0 ? "" : vorbis.getCommentByName("ARTIST")[0]);
                    metasMap.put("ALBUM", vorbis.getCommentByName("ALBUM").length == 0 ? "" : vorbis.getCommentByName("ALBUM")[0]);
                    String trackNumber = vorbis.getCommentByName("TRACKNUMBER").length == 0 ? "-" : vorbis.getCommentByName("TRACKNUMBER")[0];
                    if (trackNumber.length() == 1) {
                        trackNumber = "&nbsp;&nbsp;" + trackNumber;
                    }
                    metasMap.put("TRACKNUMBER", trackNumber + ")");
                    metasMap.put("TITLE", vorbis.getCommentByName("TITLE").length == 0 ? "" : vorbis.getCommentByName("TITLE")[0]);
                }
                if (meta.toString().contains("StreamInfo")) {
                    assert meta instanceof StreamInfo;
                    StreamInfo info = (StreamInfo) meta;
                    metasMap.put("RATE", info.getBitsPerSample() + "/" + new DecimalFormat("#.#").format(info.getSampleRate() / 1000f));
                }
            }
        } catch (Exception e) {
            throw new IOException(e);
        }
        return metasMap;
    }

}
